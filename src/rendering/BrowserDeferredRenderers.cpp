#ifdef __EMSCRIPTEN__

#include "EffectRenderer.h"
#include "HUDRenderer.h"
#include "MenuRenderer.h"
#include "TankRenderer.h"
#include "TerrainRenderer.h"

#include "../Logger.h"

namespace
{
void LogDeferred(const char* subsystem)
{
    Logger::Get().Write(
        "Browser renderer unavailable: %s is intentionally deferred; visual submission will be skipped.\n",
        subsystem);
}
}

EffectRenderer::EffectRenderer()
    : blendEnabled(false), texturesEnabled(false), currentTexture(0)
{
}

bool EffectRenderer::Initialize()
{
    LogDeferred("effects");
    return BaseRenderer::Initialize();
}

void EffectRenderer::Cleanup() { BaseRenderer::Cleanup(); }
void EffectRenderer::RenderEffects(const std::vector<EffectRenderData>&) {}

HUDRenderer::HUDRenderer() : texturesLoaded(false) {}

bool HUDRenderer::Initialize()
{
    LogDeferred("HUD");
    return BaseRenderer::Initialize();
}

void HUDRenderer::Cleanup() { BaseRenderer::Cleanup(); }
void HUDRenderer::RenderPlayerHUD(const HUDRenderData&) {}
void HUDRenderer::RenderAllPlayerHUDs(const std::vector<HUDRenderData>&) {}
void HUDRenderer::RenderCompleteUI(const UIRenderData&) {}
void HUDRenderer::RenderMenu(const MenuRenderData&) {}
void HUDRenderer::RenderDebugInfo(const DebugRenderData&) {}

MenuRenderer::MenuRenderer() = default;

bool MenuRenderer::Initialize()
{
    LogDeferred("menus");
    return BaseRenderer::Initialize();
}

void MenuRenderer::Cleanup() { BaseRenderer::Cleanup(); }
void MenuRenderer::RenderMenu(const MenuRenderData&) {}

TankRenderer::TankRenderer() : animationDrift(0.0f) {}

bool TankRenderer::Initialize()
{
    LogDeferred("enemy tank rendering");
    return BaseRenderer::Initialize();
}

void TankRenderer::Cleanup() { BaseRenderer::Cleanup(); }
void TankRenderer::Render(const TankRenderData&) {}
void TankRenderer::RenderMultiple(const std::vector<TankRenderData>&) {}
void TankRenderer::SetupRenderState() {}
void TankRenderer::CleanupRenderState() {}

TerrainRenderer::TerrainRenderer()
    : currentTerrainData(nullptr), displayListInitialized(false), lastHeightX(-1), lastHeightZ(-1)
{
    InitializeColorPalette();
}

bool TerrainRenderer::Initialize()
{
    LogDeferred("terrain");
    return BaseRenderer::Initialize();
}

void TerrainRenderer::Cleanup() { BaseRenderer::Cleanup(); }
void TerrainRenderer::RenderTerrain(const TerrainRenderData&) {}
void TerrainRenderer::Setup3DRenderState() {}

void TerrainRenderer::InitializeColorPalette()
{
    for (int i = 0; i < COLOR_PALETTE_SIZE; ++i)
        for (int channel = 0; channel < 4; ++channel)
            colorPalette[i][channel] = channel == 3 ? 1.0f : 0.0f;
}

#endif
