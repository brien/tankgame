#include "PlatformGL.h"
#include "RenderContext.h"
#include "ResourceManager.h"

#include <cmath>
#include "TerrainRenderer.h"
#include "RenderData.h"
#include "../App.h"
#include "../Logger.h"

TerrainRenderer::TerrainRenderer(ResourceManager* catalogue) : resources(catalogue) {}

bool TerrainRenderer::Initialize()
{
    if (!BaseRenderer::Initialize())
    {
        return false;
    }

    return true;
}

void TerrainRenderer::Cleanup()
{
    BaseRenderer::Cleanup();
    Logger::Get().Write("TerrainRenderer cleaned up \n");
}

void TerrainRenderer::RenderTerrain(const TerrainRenderData &terrainData)
{
    if (!IsReady())
    {
        return;
    }

    RenderModernTerrain(terrainData);
}

void TerrainRenderer::RenderModernTerrain(const TerrainRenderData& terrain)
{
    if (!resources) throw std::logic_error("modern terrain requires a resource catalogue");
    const auto& batches = resources->PrepareTerrain(terrain);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    for (std::size_t i = 0; i < batches.size(); ++i) {
        if (i == static_cast<std::size_t>(TerrainBatch::Water)) continue;
        const auto& batch = batches[i];
        if (batch.empty) continue;
        glFrontFace(batch.counterClockwise ? GL_CCW : GL_CW);
        RenderContext::Current().Draw(batch.geometry, Matrix4::Identity(), batch.material);
    }
    glFrontFace(GL_CW);
}

void TerrainRenderer::RenderWater(const TerrainRenderData& terrain)
{
    if (!IsReady()) return;
    if (!resources) throw std::logic_error("modern water requires a resource catalogue");
    const auto& batch = resources->PrepareTerrain(terrain)[static_cast<std::size_t>(TerrainBatch::Water)];
    if (batch.empty) return;

    // Additive ribbons are order-independent with each other, but must test
    // against the complete opaque depth buffer without writing to it.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // Ribbons can be coplanar with terrain walls. Bias their depth test toward
    // the camera so rounding differences do not produce speckled missing pixels.
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    RenderContext::Current().Draw(batch.geometry, Matrix4::Identity(), batch.material);

    // Establish the shared opaque baseline for the next pass or viewport.
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
}
