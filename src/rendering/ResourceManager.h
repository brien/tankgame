#pragma once

#include "IRenderer.h"
#include "../DisplayList.h"
#include "../TextureHandler.h"
#include "../igtl_qmesh.h"
#include <SDL2/SDL_ttf.h>

/**
 * Centralized resource manager for all rendering resources.
 * 
 * This class manages the lifecycle and access to all rendering resources
 * that were previously scattered throughout GraphicsTask, including:
 * - Display lists for geometry rendering
 * - Texture management through TextureHandler  
 * - Mesh data for complex geometry
 * - Font resources for text rendering
 * 
 * Design Principles:
 * - Single point of access for all rendering resources
 * - Proper resource lifecycle management (initialization/cleanup)
 * - Separation from game logic - purely rendering resources
 * - Thread-safe access to shared resources
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
    
    // Display list access
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
    
    // Font management
    TTF_Font* GetDefaultFont() const { return defaultFont; }
    
    // Resource state queries
    bool IsInitialized() const { return isInitialized; }
    bool AreDisplayListsReady() const { return displayListsBuilt; }
    bool AreTexturesLoaded() const { return texturesLoaded; }
    bool AreMeshesLoaded() const { return meshesLoaded; }
    
private:
    // Display lists (moved from GraphicsTask)
    DisplayList cubeList1{1};
    DisplayList cubeList2;
    DisplayList bulletList;
    DisplayList bodyList;
    DisplayList turretList;
    DisplayList cannonList;
    DisplayList itemList;
    DisplayList squareList;
    DisplayList squareList2;
    
    // Extended display lists for different tank types
    DisplayList bodyListEx;
    DisplayList turretListEx;
    DisplayList cannonListEx;
    DisplayList bodyListEx2;
    DisplayList turretListEx2;
    DisplayList cannonListEx2;
    
    // Resource managers
    TextureHandler textureHandler;
    
    // Mesh data (moved from GraphicsTask private members)
    igtl_QGLMesh bodyMesh;
    igtl_QGLMesh turretMesh;
    igtl_QGLMesh cannonMesh;
    igtl_QGLMesh itemMesh;
    
    // Font resources
    TTF_Font* defaultFont;
    
    // Initialization state tracking
    bool isInitialized;
    bool displayListsBuilt;
    bool texturesLoaded;
    bool meshesLoaded;
    bool fontsLoaded;
    
    // Private initialization methods
    bool InitializeDisplayLists();
    bool InitializeTextures();
    bool InitializeMeshes();
    bool InitializeFonts();
    
    // Display list building methods (moved from GraphicsTask)
    void BuildCubeLists();
    void BuildBulletList();
    void BuildTankDisplayLists();
    void BuildItemList();
    void BuildSquareLists();
    
    // Mesh processing methods (moved from GraphicsTask)
    void FixMesh(igtl_QGLMesh& mesh);
    void PrepareMesh(igtl_QGLMesh& mesh, const char* fileName);
    
    // Cleanup methods
    void CleanupDisplayLists();
    void CleanupTextures();
    void CleanupMeshes();
    void CleanupFonts();
};
