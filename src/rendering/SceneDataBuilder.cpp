#include "SceneDataBuilder.h"
#include "../App.h"
#include "../GameWorld.h"

SceneDataBuilder::SceneDataBuilder(const TankHandler& tanks, const LevelHandler& level, 
                                 const GameWorld* world)
    : tankHandler(tanks)
    , levelHandler(level)
    , gameWorld(world) {
}

SceneData SceneDataBuilder::BuildScene() const {
    SceneData scene;
    
    // Extract all rendering data from game objects
    scene.tanks = ExtractTankData();
    scene.bullets = ExtractBulletData();
    scene.effects = ExtractEffectData();
    scene.items = ExtractItemData();
    scene.terrain = ExtractTerrainData();
    scene.cameras = ExtractCameraData();
    
    // Extract UI data for HUD/Menu/Debug rendering
    auto uiDataPtr = ExtractUIData();
    scene.uiData = uiDataPtr.release();
    
    // Extract game state information
    ExtractGameState(scene);
    ExtractRenderingFlags(scene);
    
    return scene;
}

SceneData SceneDataBuilder::BuildSceneForPlayer(int playerIndex) const {
    SceneData scene = BuildScene();
    
    // Apply player-specific optimizations
    OptimizeForPlayer(scene, playerIndex);
    
    return scene;
}

bool SceneDataBuilder::IsReady() const {
    // Check if all required game objects are in a valid state
    // This is a basic check - could be expanded with more validation
    return true; // For now, assume dependencies are always valid
}

std::vector<TankRenderData> SceneDataBuilder::ExtractTankData() const {
    std::vector<TankRenderData> allTanks;
    
    // Extract player tank data from TankHandler (players not yet migrated to GameWorld)
    auto playerTanks = TankDataExtractor::ExtractPlayerData(
        tankHandler.players, 
        tankHandler.special, 
        tankHandler.numPlayers
    );
    
    allTanks.reserve(playerTanks.size() + 20); // Reserve space for players + expected enemies
    allTanks.insert(allTanks.end(), playerTanks.begin(), playerTanks.end());
    
    // Extract enemy tanks from GameWorld (if available)
    if (gameWorld) {
        const auto& worldTanks = gameWorld->GetTanks();
        
        for (const auto& tankPtr : worldTanks) {
            if (tankPtr && tankPtr->IsAlive()) {
                allTanks.push_back(TankDataExtractor::ExtractRenderData(*tankPtr));
            }
        }
    } else {
        // Fallback: Extract enemy tanks from TankHandler legacy system
        auto enemyTanks = TankDataExtractor::ExtractEnemyData(
            tankHandler.GetAllEnemyTanks()
        );
        allTanks.insert(allTanks.end(), enemyTanks.begin(), enemyTanks.end());
    }
    
    return allTanks;
}

std::vector<BulletRenderData> SceneDataBuilder::ExtractBulletData() const {
    // Extract bullets directly from GameWorld
    if (!gameWorld) {
        return {}; // Return empty if no GameWorld
    }
    
    const auto& worldBullets = gameWorld->GetBullets();
    std::vector<Bullet> bulletRefs;
    bulletRefs.reserve(worldBullets.size());
    
    for (const auto& bulletPtr : worldBullets) {
        if (bulletPtr && bulletPtr->IsAlive()) {
            bulletRefs.push_back(*bulletPtr);
        }
    }
    
    return BulletDataExtractor::ExtractBulletRenderData(bulletRefs);
}

std::vector<EffectRenderData> SceneDataBuilder::ExtractEffectData() const {
    // Extract FX directly from GameWorld
    if (!gameWorld) {
        return {};
    }
    
    const auto& worldFX = gameWorld->GetFX();
    std::vector<FX> fxRefs;
    fxRefs.reserve(worldFX.size());
    
    for (const auto& fxPtr : worldFX) {
        if (fxPtr && fxPtr->IsAlive()) {
            fxRefs.push_back(*fxPtr);
        }
    }
    
    return EffectDataExtractor::ExtractEffectRenderData(fxRefs);
}

std::vector<ItemRenderData> SceneDataBuilder::ExtractItemData() const {
    // Extract items directly from GameWorld
    if (!gameWorld) {
        return {};
    }
    
    const auto& worldItems = gameWorld->GetItems();
    std::vector<ItemRenderData> itemData;
    itemData.reserve(worldItems.size());
    
    for (const auto& itemPtr : worldItems) {
        if (itemPtr && itemPtr->IsAlive()) {
            ItemRenderData data;
            data.position = Vector3{itemPtr->x, itemPtr->y, itemPtr->z};
            data.rotationY = itemPtr->ry; // Use only Y rotation for spinning
            data.itemType = itemPtr->type;
            data.visible = true;
            itemData.push_back(data);
        }
    }
    
    return itemData;
}

TerrainRenderData SceneDataBuilder::ExtractTerrainData() const {
    TerrainRenderData terrain;
    
    // Extract basic terrain information from LevelHandler using existing members
    terrain.levelNumber = levelHandler.levelNumber;
    terrain.sizeX = levelHandler.sizeX;
    terrain.sizeZ = levelHandler.sizeZ;
    
    // Extract rendering flags
    terrain.drawFloor = levelHandler.drawFloor;
    terrain.drawWalls = levelHandler.drawWalls; 
    terrain.drawTop = levelHandler.drawTop;
    
    // Let the LevelHandler populate the detailed render data
    levelHandler.populateTerrainRenderData(terrain);
    
    return terrain;
}

std::vector<CameraData> SceneDataBuilder::ExtractCameraData() const {
    std::vector<CameraData> cameras;
    
    int numPlayers = tankHandler.numPlayers;
    cameras.reserve(numPlayers);
    
    // Extract camera data for each player
    for (int i = 0; i < numPlayers; ++i) {
        CameraData camData;
        
        // Get camera position and orientation from App's camera system
        // Note: This assumes App::GetSingleton().graphicsTask->cams[i] exists
        if (i < 4) { // Safety check for camera array bounds
            const Camera& cam = App::GetSingleton().graphicsTask->cams[i];
            
            camData.position = Vector3(cam.xpos(), cam.ypos(), cam.zpos());
            camData.focus = Vector3(cam.xfocus(), cam.yfocus(), cam.zfocus());
            camData.playerId = i;
        }
        
        cameras.push_back(camData);
    }
    
    return cameras;
}

void SceneDataBuilder::ExtractGameState(SceneData& scene) const {
    // Extract game state from various handlers using actual available members
    scene.numPlayers = tankHandler.numPlayers;
    scene.gameStarted = App::GetSingleton().gameTask->IsGameStarted();
    scene.paused = App::GetSingleton().gameTask->IsPaused();
    // Note: TankHandler doesn't seem to have versusMode as a method, use a reasonable fallback
    scene.versusMode = (tankHandler.numPlayers > 1); // Simple heuristic
    
    // Debug mode could be extracted from a global setting or App state
    scene.debugMode = false; // TODO: Implement debug mode detection
}

void SceneDataBuilder::ExtractRenderingFlags(SceneData& scene) const {
    // Extract special rendering flags using available data
    scene.drawSky = (levelHandler.levelNumber == 48); // Special sky level
    
    // Other rendering flags could be added here
    // scene.drawMinimap = levelHandler.HasMinimap();
    // scene.drawFog = levelHandler.HasFog();
}

void SceneDataBuilder::OptimizeForPlayer(SceneData& scene, int playerIndex) const {
    // Apply player-specific optimizations
    if (playerIndex >= 0 && playerIndex < scene.numPlayers) {
        // Perform view frustum culling
        PerformFrustumCulling(scene, playerIndex);
        
        // Sort objects by distance for rendering optimization
        SortObjectsByDistance(scene, playerIndex);
    }
}

void SceneDataBuilder::PerformFrustumCulling(SceneData& scene, int playerIndex) const {
    // TODO: Implement frustum culling based on player camera
    // This would remove objects outside the player's view to improve performance
    
    // For now, this is a placeholder - all objects are kept
    // In a full implementation, this would:
    // 1. Get the player's camera frustum
    // 2. Test each object against the frustum
    // 3. Remove objects that are completely outside the view
    
    // Example structure:
    // if (playerIndex < scene.cameras.size()) {
    //     const CameraData& camera = scene.cameras[playerIndex];
    //     Frustum frustum = CalculateFrustum(camera);
    //     
    //     // Remove tanks outside frustum
    //     scene.tanks.erase(
    //         std::remove_if(scene.tanks.begin(), scene.tanks.end(),
    //             [&frustum](const TankRenderData& tank) {
    //                 return !frustum.Contains(tank.position);
    //             }),
    //         scene.tanks.end()
    //     );
    //     
    //     // Same for bullets, effects, items...
    // }
}

void SceneDataBuilder::SortObjectsByDistance(SceneData& scene, int playerIndex) const {
    // TODO: Implement distance-based sorting for rendering optimization
    // This could help with transparency rendering and performance
    
    // For now, this is a placeholder
    // In a full implementation, this would:
    // 1. Get the player's camera position
    // 2. Calculate distance from camera to each object
    // 3. Sort objects by distance (front-to-back for opaque, back-to-front for transparent)
    
    // Example structure:
    // if (playerIndex < scene.cameras.size()) {
    //     const Vector3& cameraPos = scene.cameras[playerIndex].position;
    //     
    //     // Sort tanks by distance from camera
    //     std::sort(scene.tanks.begin(), scene.tanks.end(),
    //         [&cameraPos](const TankRenderData& a, const TankRenderData& b) {
    //             float distA = Distance(a.position, cameraPos);
    //             float distB = Distance(b.position, cameraPos);
    //             return distA < distB; // Front to back
    //         });
    //     
    //     // Sort other objects similarly...
    // }
}

std::unique_ptr<UIRenderData> SceneDataBuilder::ExtractUIData() const {
    // Use the comprehensive UI data extraction method
    bool gameStarted = App::GetSingleton().gameTask->IsGameStarted();
    bool isPaused = App::GetSingleton().gameTask->IsPaused();
    bool showMenu = !gameStarted; // Show menu when game hasn't started
    int menuState = App::GetSingleton().gameTask->GetMenuState();
    bool showDebug = App::GetSingleton().gameTask->IsDebugMode();
    
    UIRenderData uiData = HUDDataExtractor::ExtractCompleteUIData(
        tankHandler, gameStarted, isPaused, showMenu, menuState, showDebug);
    
    return std::make_unique<UIRenderData>(std::move(uiData));
}
