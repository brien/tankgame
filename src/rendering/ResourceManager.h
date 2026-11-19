#pragma once

#include "IRenderer.h"
#include "GeometryAsset.h"
#include "../TextureHandler.h"
#include "../igtl_qmesh.h"
#include "ModernRenderer.h"
#include "TerrainGeometry.h"
#include "FontAtlas.h"
#include <stdexcept>

enum class GeometryResource {
    TerrainCube, Bullet, TankBody, TankTurret, TankCannon, Item,
    HorizontalQuad, HorizontalOutline, TankBodyExtruded,
    TankTurretExtruded, TankCannonExtruded, TankBodyEdges,
    TankTurretEdges, TankCannonEdges, EnemyBody, EnemyBarrel, EnemyTurret, DebugLine, UIQuad, UIOutline
};

enum class MaterialResource { RingOverlay, StarOverlay };

struct TerrainDrawBatch {
    GeometryAsset geometry;
    BasicMaterial material;
    bool counterClockwise = false;
    bool empty = true;
};
using TerrainDrawBatches = std::array<TerrainDrawBatch, static_cast<std::size_t>(TerrainBatch::Count)>;

/**
 * Centralized resource manager for all rendering resources.
 * 
 * This class manages the lifecycle and access to all rendering resources
 * that were previously scattered throughout GraphicsTask, including:
 * - GPU geometry for geometry rendering
 * - Texture management through TextureHandler  
 * - Mesh data for complex geometry
 * 
 * Design Principles:
 * - Single point of access for all rendering resources
 * - Proper resource lifecycle management (initialization/cleanup)
 * - Separation from game logic - purely rendering resources
 * - Stable typed lookup for the migrated renderer slice
 */
class ResourceManager : public IRenderer {
public:
    ResourceManager();
    virtual ~ResourceManager() = default;
    
    // IRenderer interface implementation
    bool Initialize() override;
    void Cleanup() override;
    void SetupRenderState() override;
    void CleanupRenderState() override;
    
    // Typed catalogue access. A lookup always returns the
    // same catalogue-owned object for the lifetime of this manager.
    GeometryAsset& GetGeometry(GeometryResource resource);
    const GeometryAsset& GetGeometry(GeometryResource resource) const;
    const BasicMaterial& GetMaterial(MaterialResource resource) const;
    const ModernRenderer* GetModernRenderer() const { return modernRenderer.get(); }
    const TerrainDrawBatches& PrepareTerrain(const TerrainRenderData& terrain);

    const FontAtlas& GetFontAtlas() const { return fontAtlas; }
    const GeometryAsset& GetFontGlyph(unsigned char c) const { return fontGlyphs[FontAtlas::GlyphIndex(c)]; }
    const std::shared_ptr<const GpuTexture>& GetFontTexture() const { return fontTexture; }

    // Texture management (delegate to existing TextureHandler)
    TextureHandler& GetTextureHandler() { return textureHandler; }
    const TextureHandler& GetTextureHandler() const { return textureHandler; }
    
    // Resource state queries
    bool IsInitialized() const { return isInitialized; }
    bool IsGeometryReady() const { return geometryReady; }
    bool AreTexturesLoaded() const { return texturesLoaded; }
    bool AreMeshesLoaded() const { return meshesLoaded; }
    
private:
    // Created before geometry upload and explicitly destroyed after all VBOs.
    std::unique_ptr<ModernRenderer> modernRenderer;
    FontAtlas fontAtlas;
    std::array<GeometryAsset, FontAtlas::Count> fontGlyphs;
    std::shared_ptr<const GpuTexture> fontTexture;
    TerrainGeometry terrainGeometry;
    TerrainDrawBatches terrainDraws;
    // GPU geometry (moved from GraphicsTask)
    GeometryAsset terrainCube;
    GeometryAsset bulletGeometry;
    GeometryAsset bodyGeometry;
    GeometryAsset turretGeometry;
    GeometryAsset cannonGeometry;
    GeometryAsset itemGeometry;
    GeometryAsset horizontalQuad;
    GeometryAsset horizontalOutline;
    
    // Extended geometry resources for different tank types
    GeometryAsset bodyExtruded;
    GeometryAsset turretExtruded;
    GeometryAsset cannonExtruded;
    GeometryAsset bodyEdges;
    GeometryAsset turretEdges;
    GeometryAsset cannonEdges;
    
    GeometryAsset uiQuad;
    GeometryAsset uiOutline;
    GeometryAsset debugLine;
    GeometryAsset enemyBody;
    GeometryAsset enemyBarrel;
    GeometryAsset enemyTurret;

    // Resource managers
    TextureHandler textureHandler;
    BasicMaterial ringMaterial;
    BasicMaterial starMaterial;
    
    // Mesh data (moved from GraphicsTask private members)
    igtl_QGLMesh bodyMesh;
    igtl_QGLMesh turretMesh;
    igtl_QGLMesh cannonMesh;
    igtl_QGLMesh itemMesh;
    
    // Initialization state tracking
    bool isInitialized;
    bool geometryReady;
    bool texturesLoaded;
    bool meshesLoaded;
    
    // Private initialization methods
    bool InitializeGeometry();
    bool InitializeTextures();
    bool InitializeMeshes();
    
    // Geometry building methods (moved from GraphicsTask)
    void BuildCubeGeometry();
    void BuildBulletGeometry();
    void BuildTankGeometry();
    void BuildItemGeometry();
    void BuildQuadGeometry();
    
    // Mesh processing methods (moved from GraphicsTask)
    void PrepareMesh(igtl_QGLMesh& mesh, const char* fileName);
    
    // Cleanup methods
    void CleanupGeometry();
    void CleanupTextures();
    void CleanupMeshes();
};
