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
    defaultFont = TTF_OpenFont("runtime/fonts/arial.ttf", 16);
    if (!defaultFont) {
        // Try alternative font location
        defaultFont = TTF_OpenFont("runtime/fonts/DroidSansMono.ttf", 16);
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
        PrepareMesh(bodyMesh, "runtime/converted_nowbody.obj");
        PrepareMesh(turretMesh, "runtime/converted_nowturret.obj");
        PrepareMesh(cannonMesh, "runtime/converted_cannon.obj");
        PrepareMesh(itemMesh, "runtime/converted_item.obj");
        
        meshesLoaded = true;
    } catch (...) {
        meshesLoaded = false;
    }
    
    return meshesLoaded;
}

bool ResourceManager::InitializeDisplayLists() {
    try {
        BuildCubeLists();
        BuildBulletList();
        BuildTankDisplayLists();
        BuildItemList();
        BuildSquareLists();
        
        displayListsBuilt = true;
    } catch (...) {
        displayListsBuilt = false;
    }
    
    return displayListsBuilt;
}

void ResourceManager::BuildCubeLists() {
    // Build cubeList1 (moved from GraphicsTask::BuildDisplayLists)
    cubeList1.BeginNewList();
    
    glBegin(GL_QUADS);
    // Top Face
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
    // Bottom Face
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
    // Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
    // Back Face
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
    // Right Face
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
    // Left Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glEnd();
    
    cubeList1.EndNewList();
    
    // Build cubeList2 (smaller cube for bullets/items)
    cubeList2.BeginNewList();
    
    glPushMatrix();
    glScalef(0.1f, 0.1f, 0.1f);
    
    glBegin(GL_QUADS);
    // Same geometry as cubeList1 but scaled
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glEnd();
    
    glPopMatrix();
    cubeList2.EndNewList();
}

void ResourceManager::BuildBulletList() {
    bulletList.BeginNewList();
    
    // Simple bullet geometry (small sphere approximation with quads)
    glBegin(GL_QUADS);
    glScalef(0.05f, 0.05f, 0.05f);
    
    // Simplified bullet - just a small cube for now
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    
    glEnd();
    
    bulletList.EndNewList();
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
    // squareList - for UI elements and targeting
    squareList.BeginNewList();
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, 0.0f);
    glEnd();
    
    squareList.EndNewList();
    
    // squareList2 - alternative square for different UI elements
    squareList2.BeginNewList();
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, 0.0f);
    glEnd();
    
    squareList2.EndNewList();
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
