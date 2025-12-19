#pragma once

#include "RenderData.h"
#include "TankDataExtractor.h"
#include "BulletDataExtractor.h"
#include "EffectDataExtractor.h"
#include "ItemDataExtractor.h"
#include "HUDDataExtractor.h"
#include "../TankHandler.h"
#include "../LevelHandler.h"

// Forward declaration
class GameWorld;

/**
 * Central coordinator for extracting all rendering data from game objects.
 * 
 * This class serves as the bridge between the game logic layer and the 
 * rendering layer, coordinating all data extraction operations to build
 * a complete SceneData structure for rendering.
 * 
 * Design Principles:
 * - Single responsibility: Only extracts data, doesn't modify game state
 * - Consistent interface: Always returns SceneData in same format
 * - Performance oriented: Minimizes data copying and allocations
 * - Dependency injection: Receives game objects as constructor parameters
 */
class SceneDataBuilder {
public:
    /**
     * Constructor with injected game object dependencies.
     * 
     * @param tanks TankHandler containing enemy tank game logic
     * @param level LevelHandler containing terrain and level data
     * @param world GameWorld for entity access (bullets, effects, etc.)
     * @param playerMgr PlayerManager for player tank and stats access
     */
    SceneDataBuilder(const TankHandler& tanks, const LevelHandler& level, 
                    const class GameWorld* world, const class PlayerManager* playerMgr);
    
    /**
     * Builds complete scene data for rendering.
     * Extracts all necessary rendering data from game objects.
     * 
     * @return SceneData structure containing all data needed for rendering
     */
    SceneData BuildScene() const;
    
    /**
     * Builds scene data optimized for a specific player view.
     * Can include view-specific optimizations like frustum culling.
     * 
     * @param playerIndex Index of the player whose view to optimize for
     * @return SceneData structure optimized for the specified player
     */
    SceneData BuildSceneForPlayer(int playerIndex) const;
    
    /**
     * Check if the builder is ready to extract data.
     * 
     * @return true if all required game objects are available
     */
    bool IsReady() const;
    
    /**
     * Extract camera data for all players (public method for GraphicsTask integration)
     * 
     * @return Vector of CameraData for all active players
     */
    std::vector<CameraData> ExtractCameraData() const;

private:
    // Game object references (injected dependencies)
    const TankHandler& tankHandler;
    const LevelHandler& levelHandler;
    const class GameWorld* gameWorld;
    const class PlayerManager* playerManager;
    
    // Individual data extraction methods
    std::vector<TankRenderData> ExtractTankData() const;
    std::vector<BulletRenderData> ExtractBulletData() const;
    std::vector<EffectRenderData> ExtractEffectData() const;
    std::vector<ItemRenderData> ExtractItemData() const;
    TerrainRenderData ExtractTerrainData() const;
    std::unique_ptr<UIRenderData> ExtractUIData() const;
    
    // Game state extraction methods
    void ExtractGameState(SceneData& scene) const;
    void ExtractRenderingFlags(SceneData& scene) const;
    
    // Optimization methods
    void OptimizeForPlayer(SceneData& scene, int playerIndex) const;
    void PerformFrustumCulling(SceneData& scene, int playerIndex) const;
    void SortObjectsByDistance(SceneData& scene, int playerIndex) const;
};
