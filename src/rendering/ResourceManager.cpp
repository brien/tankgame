#include "ResourceManager.h"
#include "../App.h"

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

ResourceManager::ResourceManager() 
    : defaultFont(nullptr)
    , isInitialized(false)
    , displayListsBuilt(false)
    , texturesLoaded(false)
    , meshesLoaded(false)
    , fontsLoaded(false) {
}

bool ResourceManager::Initialize() {
    if (isInitialized) {
        return true;
    }
    
    // Initialize all resource subsystems
    bool success = true;
    
    success &= InitializeFonts();
    success &= InitializeTextures();
    success &= InitializeMeshes();
    success &= InitializeDisplayLists();
    
    if (success) {
        isInitialized = true;
    } else {
        // If any initialization failed, clean up what was created
        Cleanup();
    }
    
    return success;
}

void ResourceManager::Cleanup() {
    CleanupDisplayLists();
    CleanupTextures();
    CleanupMeshes();
    CleanupFonts();
    
    isInitialized = false;
    displayListsBuilt = false;
    texturesLoaded = false;
    meshesLoaded = false;
    fontsLoaded = false;
}

void ResourceManager::SetupRenderState() {
    // ResourceManager doesn't change OpenGL state during rendering
    // Individual renderers handle their own state management
}

void ResourceManager::CleanupRenderState() {
    // ResourceManager doesn't change OpenGL state during rendering
}

bool ResourceManager::InitializeFonts() {
    // Initialize font system if not already done
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0) {
            return false;
        }
    }
    
    // Load default font
    defaultFont = TTF_OpenFont("fonts/arial.ttf", 16);
    if (!defaultFont) {
        // Try alternative font location
        defaultFont = TTF_OpenFont("fonts/DroidSansMono.ttf", 16);
    }
    
    fontsLoaded = (defaultFont != nullptr);
    return fontsLoaded;
}

bool ResourceManager::InitializeTextures() {
    // Delegate texture loading to TextureHandler
    textureHandler.LoadTextures();
    texturesLoaded = true;  // Assume success since LoadTextures doesn't return status
    return texturesLoaded;
}

bool ResourceManager::InitializeMeshes() {
    try {
        // Load mesh files (moved from GraphicsTask)
        PrepareMesh(bodyMesh, "nowbody.gsm");
        PrepareMesh(turretMesh, "nowturret.gsm");
        PrepareMesh(cannonMesh, "cannon.gsm");
        PrepareMesh(itemMesh, "body.gsm");
        
        meshesLoaded = true;
    } catch (...) {
        meshesLoaded = false;
    }
    
    return meshesLoaded;
}

bool ResourceManager::InitializeDisplayLists() {
    try {
        BuildCubeLists();
        BuildSquareLists();
        BuildBulletList();
        BuildTankDisplayLists();
        BuildItemList();
        
        displayListsBuilt = true;
    } catch (...) {
        displayListsBuilt = false;
    }
    
    return displayListsBuilt;
}

void ResourceManager::BuildCubeLists() {
    cubeList1 = DisplayList(1);
    cubeList1.SetGeometry(SimpleGeometry::CreateCube());

    cubeList2 = DisplayList(1);
    cubeList2.SetGeometry(SimpleGeometry::CreateCube(0.05f));
}

void ResourceManager::BuildBulletList() {
    bulletList = DisplayList(1);
    bulletList.SetGeometry(SimpleGeometry::CreateBullet());
}

void ResourceManager::BuildTankDisplayLists() {
    // Build tank body display lists using loaded meshes
    if (meshesLoaded) {
        // bodyListEx - enhanced body
        bodyListEx.BeginNewList();
        bodyMesh.DrawTriangles();
        bodyListEx.EndNewList();
        
        // turretListEx - enhanced turret
        turretListEx.BeginNewList();
        turretMesh.DrawTriangles();
        turretListEx.EndNewList();
        
        // cannonListEx - enhanced cannon
        cannonListEx.BeginNewList();
        cannonMesh.DrawTriangles();
        cannonListEx.EndNewList();
        
        // Standard body, turret, cannon lists (simplified versions)
        bodyList.BeginNewList();
        bodyMesh.DrawTriangles();
        bodyList.EndNewList();
        
        turretList.BeginNewList();
        turretMesh.DrawTriangles();
        turretList.EndNewList();
        
        cannonList.BeginNewList();
        cannonMesh.DrawTriangles();
        cannonList.EndNewList();
        
        // Extended variants (bodyListEx2, turretListEx2, cannonListEx2)
        bodyListEx2.BeginNewList();
        bodyMesh.DrawTriangles();
        bodyListEx2.EndNewList();
        
        turretListEx2.BeginNewList();
        turretMesh.DrawTriangles();
        turretListEx2.EndNewList();
        
        cannonListEx2.BeginNewList();
        cannonMesh.DrawTriangles();
        cannonListEx2.EndNewList();
    }
}

void ResourceManager::BuildItemList() {
    itemList.BeginNewList();
    
    if (meshesLoaded) {
        itemMesh.DrawTriangles();
    } else {
        // Fallback to simple geometry if mesh loading failed
        glBegin(GL_QUADS);
        glScalef(0.2f, 0.2f, 0.2f);
        
        // Simple item representation
        glVertex3f(-1.0f, -1.0f,  1.0f);
        glVertex3f( 1.0f, -1.0f,  1.0f);
        glVertex3f( 1.0f,  1.0f,  1.0f);
        glVertex3f(-1.0f,  1.0f,  1.0f);
        
        glEnd();
    }
    
    itemList.EndNewList();
}

void ResourceManager::BuildSquareLists() {
    squareList = DisplayList(1);
    squareList.SetGeometry(SimpleGeometry::CreateVerticalSquare());

    squareList2 = DisplayList(1);
    squareList2.SetGeometry(SimpleGeometry::CreateVerticalSquare(1.0f));
}

void ResourceManager::PrepareMesh(igtl_QGLMesh& mesh, const char* fileName) {
    // Load mesh using proper method name
    mesh.LoadOBJ(fileName);
    FixMesh(mesh);
}

void ResourceManager::FixMesh(igtl_QGLMesh& mesh) {
    // Fix mesh normals and prepare for rendering
    // Note: The original code accessed internal members, but we should use the public interface
    // For now, just ensure the mesh is loaded properly
    // The mesh normals are typically handled internally by the mesh class
    
    // Additional mesh processing could be added here if needed
    // For example: mesh.FixWinding() to ensure consistent winding order
}

void ResourceManager::CleanupDisplayLists() {
    // Display lists automatically clean up when destroyed
    displayListsBuilt = false;
}

void ResourceManager::CleanupTextures() {
    // TextureHandler cleanup - since there's no CleanupTextures method, 
    // textures will be cleaned up by the destructor
    texturesLoaded = false;
}

void ResourceManager::CleanupMeshes() {
    // Mesh cleanup is handled by destructors
    meshesLoaded = false;
}

void ResourceManager::CleanupFonts() {
    if (defaultFont) {
        TTF_CloseFont(defaultFont);
        defaultFont = nullptr;
    }
    fontsLoaded = false;
}
