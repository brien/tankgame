#pragma once

#include "IRenderer.h"
#include "../DisplayList.h"
#include "../TextureHandler.h"
#include "../igtl_qmesh.h"
#include "ModernRenderer.h"
#include <stdexcept>

enum class GeometryResource {
    TerrainCube, Bullet, TankBody, TankTurret, TankCannon, Item,
    HorizontalQuad, HorizontalOutline, TankBodyExtruded,
    TankTurretExtruded, TankCannonExtruded, TankBodyEdges,
    TankTurretEdges, TankCannonEdges, EnemyBody, EnemyBarrel, EnemyTurret
};

enum class MaterialResource { RingOverlay, StarOverlay };

/**
 * Centralized resource manager for all rendering resources.
 * 
 * This class manages the lifecycle and access to all rendering resources
 * that were previously scattered throughout GraphicsTask, including:
 * - Display lists for geometry rendering
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
    
    // Typed modern/compatibility catalogue access. A lookup always returns the
    // same catalogue-owned object for the lifetime of this manager.
    DisplayList& GetGeometry(GeometryResource resource);
    const DisplayList& GetGeometry(GeometryResource resource) const;
    const BasicMaterial& GetMaterial(MaterialResource resource) const;
    const ModernRenderer* GetModernRenderer() const { return modernRenderer.get(); }

    // Compatibility aliases retained while non-migrated renderers are removed.
    const DisplayList& GetCubeList1() const { return cubeList1; }
    const DisplayList& GetCubeList2() const { return cubeList2; }
    const DisplayList& GetBulletList() const { return bulletList; }
    const DisplayList& GetBodyList() const { return bodyList; }
    const DisplayList& GetTurretList() const { return turretList; }
    const DisplayList& GetCannonList() const { return cannonList; }
    const DisplayList& GetItemList() const { return itemList; }
    const DisplayList& GetSquareList() const { return squareList; }
    const DisplayList& GetSquareList2() const { return squareList2; }
    
    // Extended display lists for different tank types
    const DisplayList& GetBodyListEx() const { return bodyListEx; }
    const DisplayList& GetTurretListEx() const { return turretListEx; }
    const DisplayList& GetCannonListEx() const { return cannonListEx; }
    const DisplayList& GetBodyListEx2() const { return bodyListEx2; }
    const DisplayList& GetTurretListEx2() const { return turretListEx2; }
    const DisplayList& GetCannonListEx2() const { return cannonListEx2; }
    
    // Texture management (delegate to existing TextureHandler)
    TextureHandler& GetTextureHandler() { return textureHandler; }
    const TextureHandler& GetTextureHandler() const { return textureHandler; }
    
    // Mesh data access
    const igtl_QGLMesh& GetBodyMesh() const { return bodyMesh; }
    const igtl_QGLMesh& GetTurretMesh() const { return turretMesh; }
    const igtl_QGLMesh& GetCannonMesh() const { return cannonMesh; }
    const igtl_QGLMesh& GetItemMesh() const { return itemMesh; }
    
    // Resource state queries
    bool IsInitialized() const { return isInitialized; }
    bool AreDisplayListsReady() const { return displayListsBuilt; }
    bool AreTexturesLoaded() const { return texturesLoaded; }
    bool AreMeshesLoaded() const { return meshesLoaded; }
    
private:
    // Created before geometry upload and explicitly destroyed after all VBOs.
    std::unique_ptr<ModernRenderer> modernRenderer;
    // Display lists (moved from GraphicsTask)
    DisplayList cubeList1{1};
    DisplayList cubeList2{1};
    DisplayList bulletList{1};
    DisplayList bodyList{1};
    DisplayList turretList{1};
    DisplayList cannonList{1};
    DisplayList itemList{1};
    DisplayList squareList{1};
    DisplayList squareList2{1};
    
    // Extended display lists for different tank types
    DisplayList bodyListEx{1};
    DisplayList turretListEx{1};
    DisplayList cannonListEx{1};
    DisplayList bodyListEx2{1};
    DisplayList turretListEx2{1};
    DisplayList cannonListEx2{1};
    
    DisplayList enemyBody{1};
    DisplayList enemyBarrel{1};
    DisplayList enemyTurret{1};

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
    bool displayListsBuilt;
    bool texturesLoaded;
    bool meshesLoaded;
    
    // Private initialization methods
    bool InitializeDisplayLists();
    bool InitializeTextures();
    bool InitializeMeshes();
    
    // Display list building methods (moved from GraphicsTask)
    void BuildCubeLists();
    void BuildBulletList();
    void BuildTankDisplayLists();
    void BuildItemList();
    void BuildSquareLists();
    
    // Mesh processing methods (moved from GraphicsTask)
    void PrepareMesh(igtl_QGLMesh& mesh, const char* fileName);
    
    // Cleanup methods
    void CleanupDisplayLists();
    void CleanupTextures();
    void CleanupMeshes();
};
