#ifndef HUDDATAEXTRACTOR_H
#define HUDDATAEXTRACTOR_H

#include "HUDData.h"
#include "RenderData.h"
#include <array>

// Forward declarations
class Tank;
class TankHandler;

/**
 * HUDDataExtractor - Extracts HUD rendering data from game objects
 * 
 * This class is responsible for converting Tank objects and game state
 * into HUDRenderData structures that can be used by the HUD renderer
 * without any dependencies on game logic.
 */
class HUDDataExtractor {
public:
    /**
     * Extract HUD data for a single player
     * @param player The tank object to extract HUD data from
     * @param playerId The player ID (0, 1, etc.)
     * @return Complete HUD render data for this player
     */
    static HUDRenderData ExtractPlayerHUD(const Tank& player, int playerId);
    
    /**
     * Extract HUD data for all active players
     * @param playerTanks Array of player tank pointers from PlayerManager
     * @param numPlayers Number of active players
     * @return Vector of HUD render data for all players
     */
    static std::vector<HUDRenderData> ExtractAllPlayerHUDs(
        const std::array<Tank*, 2>& playerTanks, int numPlayers);
    
    /**
     * Extract complete UI data including HUDs, menus, and debug info
     * @param playerMgr Reference to player manager for player data
     * @param gameStarted Whether the game has started
     * @param isPaused Whether the game is paused
     * @param showMenu Whether menu should be displayed
     * @param menuState Current menu state
     * @param showDebug Whether debug info should be displayed
     * @return Complete UI render data
     */
    static UIRenderData ExtractCompleteUIData(
        const class PlayerManager& playerMgr,
        bool gameStarted = true,
        bool isPaused = false,
        bool showMenu = false,
        int menuState = 0,
        bool showDebug = false);
    
    /**
     * Extract menu data
     * @param menuState Current menu state
     * @param isVisible Whether menu is visible
     * @return Menu render data
     */
    static MenuRenderData ExtractMenuData(int menuState, bool isVisible);
    
    /**
     * Extract debug data
     * @param showDebug Whether debug info should be shown
     * @return Debug render data
     */
    static DebugRenderData ExtractDebugData(bool showDebug);

private:
    /**
     * Calculate targeting direction for a player
     * @param player The tank to calculate targeting for
     * @return Targeting direction vector
     */
    static Vector3 CalculateTargetingDirection(const Tank& player);
    
    /**
     * Calculate current animation time for targeting line stipple
     * @return Animation time value
     */
    static float CalculateTargetingAnimationTime();
    
    /**
     * Determine HUD colors based on player state
     * @param player The tank to determine colors for
     * @param hudData The HUD data to populate with colors
     */
    static void SetHUDColors(const Tank& player, HUDRenderData& hudData);
    
    /**
     * Calculate combo and special ability values
     * @param player The tank to calculate values for
     * @param playerId The player ID for array indexing
     * @param hudData The HUD data to populate
     */
    static void ExtractComboAndSpecialData(const Tank& player, int playerId, HUDRenderData& hudData);
    
    /**
     * Extract performance data (FPS, delta time)
     * @param hudData The HUD data to populate with performance info
     */
    static void ExtractPerformanceData(HUDRenderData& hudData);
};

#endif // HUDDATAEXTRACTOR_H
