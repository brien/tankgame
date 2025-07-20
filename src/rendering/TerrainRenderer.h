#ifndef TERRAINRENDERER_H
#define TERRAINRENDERER_H

#include "BaseRenderer.h"

// Forward declaration
struct TerrainRenderData;

/**
 * TerrainRenderer - Handles all terrain rendering including heightmaps, 
 * floating elements, walls, floors, and special terrain effects.
 * 
 * This class encapsulates the complex terrain rendering logic previously
 * found in LevelHandler::DrawTerrain(), providing a clean separation
 * between game logic and rendering.
 */
class TerrainRenderer : public BaseRenderer {
public:
    TerrainRenderer();
    virtual ~TerrainRenderer() = default;

    // IRenderer interface implementation
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    
    // Terrain-specific rendering method
    void RenderTerrain(const TerrainRenderData& terrainData);

private:
    // Main rendering functions
    void RenderFloatingElements(const TerrainRenderData& terrain);
    void RenderTerrainSurface(const TerrainRenderData& terrain);
    void RenderTerrainWalls(const TerrainRenderData& terrain);
    void RenderBoundaryWalls(const TerrainRenderData& terrain);
    void RenderWaterEffects(const TerrainRenderData& terrain);
    
    // Utility functions
    void SetupTerrainColors(const TerrainRenderData& terrain);
    void BindTerrainTexture(int levelNumber);
    void RenderTerrainQuad(int x, int z, int height, int strips, bool isLast = false);
    void RenderWallQuad(int ix, int iz, int lastY, int currentY, int direction);
    void RenderBlendQuad(int ix, int iz, int height, int direction);
    
    // Color palette for terrain rendering
    static constexpr int COLOR_PALETTE_SIZE = 32;
    float colorPalette[COLOR_PALETTE_SIZE][4];
    
    // Texture IDs for different terrain types
    enum TerrainTextures {
        TEXTURE_STANDARD = 11,
        TEXTURE_WATER = 10,
        TEXTURE_SPECIAL = 15,
        TEXTURE_BLEND = 12
    };
    
    // Cached display list for cube rendering (floating elements)
    bool displayListInitialized;
    
    // Helper variables for wall rendering optimization
    mutable int lastHeightX;
    mutable int lastHeightZ;
    
    // Initialize the color palette
    void InitializeColorPalette();
};

#endif // TERRAINRENDERER_H
