#ifndef TERRAINRENDERER_H
#define TERRAINRENDERER_H

#include "BaseRenderer.h"

// Forward declaration
struct TerrainRenderData;
class ResourceManager;

/**
 * TerrainRenderer - Handles all terrain rendering including heightmaps,
 * floating elements, walls, floors, and special terrain effects.
 *
 * This class encapsulates the complex terrain rendering logic previously
 * found in LevelHandler::DrawTerrain(), providing a clean separation
 * between game logic and rendering.
 */
class TerrainRenderer : public BaseRenderer
{
public:
    explicit TerrainRenderer(ResourceManager* resources = nullptr);
    virtual ~TerrainRenderer() = default;

    // IRenderer interface implementation
    virtual bool Initialize() override;
    virtual void Cleanup() override;

    // Terrain-specific rendering method
    void RenderTerrain(const TerrainRenderData &terrainData);
    // Modern additive water pass; compatibility retains its reference ordering.
    void RenderWater(const TerrainRenderData& terrain);

private:
    ResourceManager* resources;
    void RenderModernTerrain(const TerrainRenderData& terrain);
};

#endif // TERRAINRENDERER_H
