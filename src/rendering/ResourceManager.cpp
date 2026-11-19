#include "ResourceManager.h"
#include "RenderContext.h"
#include "GpuTexture.h"
#include "../Logger.h"
ResourceManager::ResourceManager() 
    : isInitialized(false)
    , geometryReady(false)
    , texturesLoaded(false)
    , meshesLoaded(false) {
}

bool ResourceManager::Initialize() {
    if (isInitialized) {
        return true;
    }
    
    // Compile/link once per catalogue/context, before any modern draw resource.
    modernRenderer.reset(new ModernRenderer);
    RenderContext::Current().SetModernRenderer(modernRenderer.get());

    // Initialize all resource subsystems
    bool success = true;
    
    success &= InitializeTextures();
    success &= InitializeMeshes();
    success &= InitializeGeometry();
    try {
        fontAtlas = FontAtlas::Load("./fonts/DroidSansMono.ttf");
        fontTexture = std::make_shared<GpuTexture>(fontAtlas.image, TextureUploadOptions{false, false});
        for (int i = 0; i < FontAtlas::Count; ++i) fontGlyphs[i].SetGeometry(fontAtlas.glyphs[i].geometry);
    } catch (const std::exception& error) {
        Logger::Get().Write("Font atlas initialization failed: %s\n", error.what());
        success = false;
    }
    
    if (success) {
        isInitialized = true;
    } else {
        // If any initialization failed, clean up what was created
        Cleanup();
    }
    
    return success;
}

void ResourceManager::Cleanup() {
    for (auto& glyph : fontGlyphs) glyph = GeometryAsset{};
    fontTexture.reset();
    fontAtlas = FontAtlas{};
    CleanupGeometry();
    RenderContext::Current().SetModernRenderer(nullptr);
    modernRenderer.reset();
    CleanupTextures();
    CleanupMeshes();
    
    isInitialized = false;
    geometryReady = false;
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

bool ResourceManager::InitializeGeometry() {
    try {
        Geometry ui;
        ui.hasTextureCoordinates = true;
        ui.vertices = {{0, 0, 0, 0, 0}, {1, 0, 0, 1, 0}, {1, 1, 0, 1, 1}, {0, 1, 0, 0, 1}};
        uiQuad.SetGeometry(ui);
        ui.topology = PrimitiveTopology::LINE_LOOP;
        uiOutline.SetGeometry(ui);
        Geometry line;
        line.topology = PrimitiveTopology::LINES;
        line.vertices = {{0, 0, 0}, {1, 1, 1}};
        debugLine.SetGeometry(line);
        BuildCubeGeometry();
        BuildQuadGeometry();
        BuildBulletGeometry();
        BuildTankGeometry();
        BuildItemGeometry();
        
        geometryReady = true;
    } catch (...) {
        geometryReady = false;
    }
    
    return geometryReady;
}

void ResourceManager::BuildCubeGeometry() {
    terrainCube = GeometryAsset{};
    terrainCube.SetGeometry(SimpleGeometry::CreateCube());
}

void ResourceManager::BuildBulletGeometry() {
    bulletGeometry = GeometryAsset{};
    bulletGeometry.SetGeometry(SimpleGeometry::CreateBullet());
}

void ResourceManager::BuildTankGeometry() {
    // Enemy geometry is the compatibility renderer's fixed mesh for all types.
    enemyBody.SetGeometry(SimpleGeometry::CreateEnemyBody());
    enemyBarrel.SetGeometry(SimpleGeometry::CreateEnemyBarrel());
    enemyTurret.SetGeometry(SimpleGeometry::CreateEnemyTurret());

    // Build tank body geometry resources using loaded meshes
    if (meshesLoaded) {
        // bodyExtruded - enhanced body
        bodyExtruded.SetGeometry(bodyMesh.CreateTriangleExtrudedGeometry(.01f));
        
        // turretExtruded - enhanced turret
        turretExtruded.SetGeometry(turretMesh.CreateTriangleExtrudedGeometry(.01f));
        
        // cannonExtruded - enhanced cannon
        cannonExtruded.SetGeometry(cannonMesh.CreateTriangleExtrudedGeometry(.01f));
        
        // Standard body, turret, cannon geometry (simplified versions)
        bodyGeometry.SetGeometry(bodyMesh.CreateTriangleGeometry());
        
        turretGeometry.SetGeometry(turretMesh.CreateTriangleGeometry());
        
        cannonGeometry.SetGeometry(cannonMesh.CreateTriangleGeometry());
        
        // Extended variants (bodyEdges, turretEdges, cannonEdges)
        bodyEdges.SetGeometry(bodyMesh.CreateEdgeExtrudedGeometry(.01f));
        
        turretEdges.SetGeometry(turretMesh.CreateEdgeExtrudedGeometry(.01f));
        
        cannonEdges.SetGeometry(cannonMesh.CreateEdgeExtrudedGeometry(.01f));
    }
}

void ResourceManager::BuildItemGeometry() {
    if (meshesLoaded) {
        itemGeometry.SetGeometry(itemMesh.CreateTriangleExtrudedGeometry(.01f));
    } else {
        itemGeometry.SetGeometry(SimpleGeometry::CreateItemFallback());
    }
}

void ResourceManager::BuildQuadGeometry() {
    horizontalQuad = GeometryAsset{};
    horizontalQuad.SetGeometry(SimpleGeometry::CreateHorizontalSquare());

    horizontalOutline = GeometryAsset{};
    horizontalOutline.SetGeometry(SimpleGeometry::CreateHorizontalSquare(
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

void ResourceManager::CleanupGeometry() {
    // Release VBOs before the shared program while the context is current.
    // Replace entries so subsequent initialization starts with empty resources.
    terrainDraws = TerrainDrawBatches{};
    terrainGeometry.Clear();
    terrainCube = GeometryAsset{}; bulletGeometry = GeometryAsset{};
    bodyGeometry = GeometryAsset{}; turretGeometry = GeometryAsset{}; cannonGeometry = GeometryAsset{}; itemGeometry = GeometryAsset{};
    horizontalQuad = GeometryAsset{}; horizontalOutline = GeometryAsset{};
    bodyExtruded = GeometryAsset{}; turretExtruded = GeometryAsset{}; cannonExtruded = GeometryAsset{};
    bodyEdges = GeometryAsset{}; turretEdges = GeometryAsset{}; cannonEdges = GeometryAsset{};
    uiQuad = GeometryAsset{}; uiOutline = GeometryAsset{};
    debugLine = GeometryAsset{};
    enemyBody = GeometryAsset{}; enemyBarrel = GeometryAsset{}; enemyTurret = GeometryAsset{};
    geometryReady = false;
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

GeometryAsset& ResourceManager::GetGeometry(GeometryResource resource) {
    switch (resource) {
    case GeometryResource::UIQuad: return uiQuad;
    case GeometryResource::UIOutline: return uiOutline;
    case GeometryResource::DebugLine: return debugLine;
    case GeometryResource::EnemyBody: return enemyBody;
    case GeometryResource::EnemyBarrel: return enemyBarrel;
    case GeometryResource::EnemyTurret: return enemyTurret;
    case GeometryResource::TerrainCube: return terrainCube;
    case GeometryResource::Bullet: return bulletGeometry;
    case GeometryResource::TankBody: return bodyGeometry;
    case GeometryResource::TankTurret: return turretGeometry;
    case GeometryResource::TankCannon: return cannonGeometry;
    case GeometryResource::Item: return itemGeometry;
    case GeometryResource::HorizontalQuad: return horizontalQuad;
    case GeometryResource::HorizontalOutline: return horizontalOutline;
    case GeometryResource::TankBodyExtruded: return bodyExtruded;
    case GeometryResource::TankTurretExtruded: return turretExtruded;
    case GeometryResource::TankCannonExtruded: return cannonExtruded;
    case GeometryResource::TankBodyEdges: return bodyEdges;
    case GeometryResource::TankTurretEdges: return turretEdges;
    case GeometryResource::TankCannonEdges: return cannonEdges;
    }
    throw std::out_of_range("unknown geometry resource");
}

const GeometryAsset& ResourceManager::GetGeometry(GeometryResource resource) const {
    return const_cast<ResourceManager*>(this)->GetGeometry(resource);
}

const BasicMaterial& ResourceManager::GetMaterial(MaterialResource resource) const {
    switch (resource) {
    case MaterialResource::RingOverlay: return ringMaterial;
    case MaterialResource::StarOverlay: return starMaterial;
    }
    throw std::out_of_range("unknown material resource");
}

const TerrainDrawBatches& ResourceManager::PrepareTerrain(const TerrainRenderData& terrain) {
    if (!isInitialized)
        throw std::logic_error("terrain uploads require the initialized modern catalogue");
    if (!terrainGeometry.Update(terrain)) return terrainDraws;
    try {
        TerrainDrawBatches next;
        for (std::size_t i = 0; i < next.size(); ++i) {
            const auto batch = static_cast<TerrainBatch>(i);
            auto& draw = next[i];
            const auto& geometry = terrainGeometry.Batches()[i];
            draw.empty = geometry.vertices.empty();
            if (draw.empty) continue;
            TextureNames texture = TEXTURE_BLACK_CUBE;
            if (batch == TerrainBatch::WhiteSurface) texture = TEXTURE_WHITE_CUBE;
            if (batch == TerrainBatch::CheckerSurface) texture = TEXTURE_CHECKER;
            // Legacy TEXTURE_BLEND is slot 12, named TEXTURE_EXIT in the catalogue.
            if (batch == TerrainBatch::Water) {
                texture = TEXTURE_EXIT;
                draw.material.alpha = 0.5f;
            }
            draw.material.lit = true;
            draw.material.texture = textureHandler.GetTexture(texture);
            draw.counterClockwise = batch == TerrainBatch::Floating ||
                (batch == TerrainBatch::Boundary && terrain.levelNumber == 48);
            draw.geometry.SetGeometry(geometry);
        }
        terrainDraws = std::move(next);
    } catch (...) {
        // A failed upload must not mark the new content as GPU-resident.
        terrainGeometry.Clear();
        throw;
    }
    return terrainDraws;
}
