#include "ResourceManager.h"
ResourceManager::ResourceManager() 
    : isInitialized(false)
    , displayListsBuilt(false)
    , texturesLoaded(false)
    , meshesLoaded(false) {
}

bool ResourceManager::Initialize() {
    if (isInitialized) {
        return true;
    }
    
    // Initialize all resource subsystems
    bool success = true;
    
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
    
    isInitialized = false;
    displayListsBuilt = false;
    texturesLoaded = false;
    meshesLoaded = false;
}

void ResourceManager::SetupRenderState() {
    // ResourceManager doesn't change OpenGL state during rendering
    // Individual renderers handle their own state management
}

void ResourceManager::CleanupRenderState() {
    // ResourceManager doesn't change OpenGL state during rendering
}

bool ResourceManager::InitializeTextures() {
    // Delegate texture loading to TextureHandler
    textureHandler.LoadTextures();
    ringMaterial.texture = textureHandler.GetTexture(TEXTURE_RING);
    starMaterial.texture = textureHandler.GetTexture(TEXTURE_DIAMOND);
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
        bodyListEx.SetGeometry(bodyMesh.CreateTriangleExtrudedGeometry(.01f));
        
        // turretListEx - enhanced turret
        turretListEx.SetGeometry(turretMesh.CreateTriangleExtrudedGeometry(.01f));
        
        // cannonListEx - enhanced cannon
        cannonListEx.SetGeometry(cannonMesh.CreateTriangleExtrudedGeometry(.01f));
        
        // Standard body, turret, cannon lists (simplified versions)
        bodyList.SetGeometry(bodyMesh.CreateTriangleGeometry());
        
        turretList.SetGeometry(turretMesh.CreateTriangleGeometry());
        
        cannonList.SetGeometry(cannonMesh.CreateTriangleGeometry());
        
        // Extended variants (bodyListEx2, turretListEx2, cannonListEx2)
        bodyListEx2.SetGeometry(bodyMesh.CreateEdgeExtrudedGeometry(.01f));
        
        turretListEx2.SetGeometry(turretMesh.CreateEdgeExtrudedGeometry(.01f));
        
        cannonListEx2.SetGeometry(cannonMesh.CreateEdgeExtrudedGeometry(.01f));
    }
}

void ResourceManager::BuildItemList() {
    if (meshesLoaded) {
        itemList.SetGeometry(itemMesh.CreateTriangleExtrudedGeometry(.01f));
    } else {
        itemList.SetGeometry(SimpleGeometry::CreateItemFallback());
    }
}

void ResourceManager::BuildSquareLists() {
    squareList = DisplayList(1);
    squareList.SetGeometry(SimpleGeometry::CreateHorizontalSquare());

    squareList2 = DisplayList(1);
    squareList2.SetGeometry(SimpleGeometry::CreateHorizontalSquare(
        0.5f, PrimitiveTopology::LINE_LOOP));
}

void ResourceManager::PrepareMesh(igtl_QGLMesh& mesh, const char* fileName) {
    FILE* input = fopen(fileName, "rb");
    if (!input)
        throw std::runtime_error(std::string("cannot open mesh: ") + fileName);
    const bool loaded = mesh.LoadGSM(input);
    fclose(input);
    if (!loaded)
        throw std::runtime_error(std::string("cannot decode mesh: ") + fileName);
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

DisplayList& ResourceManager::GetGeometry(GeometryResource resource) {
    switch (resource) {
    case GeometryResource::TerrainCube: return cubeList1;
    case GeometryResource::Bullet: return bulletList;
    case GeometryResource::TankBody: return bodyList;
    case GeometryResource::TankTurret: return turretList;
    case GeometryResource::TankCannon: return cannonList;
    case GeometryResource::Item: return itemList;
    case GeometryResource::HorizontalQuad: return squareList;
    case GeometryResource::HorizontalOutline: return squareList2;
    case GeometryResource::TankBodyExtruded: return bodyListEx;
    case GeometryResource::TankTurretExtruded: return turretListEx;
    case GeometryResource::TankCannonExtruded: return cannonListEx;
    case GeometryResource::TankBodyEdges: return bodyListEx2;
    case GeometryResource::TankTurretEdges: return turretListEx2;
    case GeometryResource::TankCannonEdges: return cannonListEx2;
    }
    throw std::out_of_range("unknown geometry resource");
}

const DisplayList& ResourceManager::GetGeometry(GeometryResource resource) const {
    return const_cast<ResourceManager*>(this)->GetGeometry(resource);
}

const BasicMaterial& ResourceManager::GetMaterial(MaterialResource resource) const {
    switch (resource) {
    case MaterialResource::RingOverlay: return ringMaterial;
    case MaterialResource::StarOverlay: return starMaterial;
    }
    throw std::out_of_range("unknown material resource");
}
