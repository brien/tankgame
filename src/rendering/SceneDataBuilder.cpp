#include "SceneDataBuilder.h"
#include "../App.h"
#include "../GameWorld.h"

SceneDataBuilder::SceneDataBuilder(const PlayerManager& players, const LevelHandler& level, 
                                 const BulletHandler& bullets, const FXHandler& fx)
    : playerManager(players)
    , levelHandler(level)
    , bulletHandler(bullets)
    , fxHandler(fx) {
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
    
    // Extract player tank data using PlayerManager
    auto playerTanks = TankDataExtractor::ExtractPlayerData(
        playerManager.GetPlayers(), 
        playerManager.GetSpecial(), 
        playerManager.GetNumPlayers()
    );
    
    // Extract enemy tank data using unified view (combines old + GameWorld enemy tanks)
    // Note: This will need to be updated when TankHandler is fully removed
    auto enemyTanks = TankDataExtractor::ExtractEnemyData(
        TankHandler::GetSingleton().GetAllEnemyTanks()
    );
    
    // Combine all tank data
    allTanks.reserve(playerTanks.size() + enemyTanks.size());
    allTanks.insert(allTanks.end(), playerTanks.begin(), playerTanks.end());
    allTanks.insert(allTanks.end(), enemyTanks.begin(), enemyTanks.end());
    
    return allTanks;
}

std::vector<BulletRenderData> SceneDataBuilder::ExtractBulletData() const {
    // Check if BulletHandler has GameWorld reference, use it directly for better performance
    if (auto* gameWorld = bulletHandler.GetGameWorld()) {
        const auto& worldBullets = gameWorld->GetBullets();
        std::vector<Bullet> bulletRefs;
        bulletRefs.reserve(worldBullets.size());
        
        for (const auto& bulletPtr : worldBullets) {
            if (bulletPtr && bulletPtr->IsAlive()) {
                bulletRefs.push_back(*bulletPtr);
            }
        }
        
        return BulletDataExtractor::ExtractBulletRenderData(bulletRefs);
    } else {
        // Fallback to BulletHandler (should be empty now)
        return BulletDataExtractor::ExtractBulletRenderData(bulletHandler.GetAllBullets());
    }
}

std::vector<EffectRenderData> SceneDataBuilder::ExtractEffectData() const {
    // Check if FXHandler has GameWorld reference, use it directly for better performance
    if (auto* gameWorld = fxHandler.GetGameWorld()) {
        const auto& worldEffects = gameWorld->GetFX();
        std::vector<FX> effectRefs;
        effectRefs.reserve(worldEffects.size());
        
        for (const auto& effectPtr : worldEffects) {
            if (effectPtr && effectPtr->IsAlive()) {
                effectRefs.push_back(*effectPtr);
            }
        }
        
        return EffectDataExtractor::ExtractEffectRenderData(effectRefs);
    } else {
        // Fallback to FXHandler (should be empty now)
        return EffectDataExtractor::ExtractEffectRenderData(fxHandler.GetAllFX());
    }
}

std::vector<ItemRenderData> SceneDataBuilder::ExtractItemData() const {
    // Use unified item view (combines old LevelHandler items + new GameWorld items)
    auto allItems = levelHandler.GetAllItems();
    
    std::vector<ItemRenderData> itemData;
    itemData.reserve(allItems.size());
    
    for (const Item* item : allItems) {
        if (item && item->IsAlive()) {
            ItemRenderData data;
            data.position = Vector3{item->x, item->y, item->z};
            data.rotationY = item->ry; // Use only Y rotation for spinning
            data.itemType = item->type;
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
    
    int numPlayers = playerManager.GetNumPlayers();
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
    scene.numPlayers = playerManager.GetNumPlayers();
    scene.gameStarted = App::GetSingleton().gameTask->IsGameStarted();
    scene.paused = App::GetSingleton().gameTask->IsPaused();
    // Note: TankHandler doesn't seem to have versusMode as a method, use a reasonable fallback
    scene.versusMode = (playerManager.GetNumPlayers() > 1); // Simple heuristic
    
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
    
    // TODO: Update HUDDataExtractor to use PlayerManager instead of TankHandler
    UIRenderData uiData = HUDDataExtractor::ExtractCompleteUIData(
        TankHandler::GetSingleton(), gameStarted, isPaused, showMenu, menuState, showDebug);
    
    return std::make_unique<UIRenderData>(std::move(uiData));
}
