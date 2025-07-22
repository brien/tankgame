#include "HUDDataExtractor.h"
#include "../Tank.h"
#include "../TankHandler.h"
#include "../App.h"
#include "../GlobalTimer.h"
#include "RenderData.h"

#include <cmath>
#include <array>

HUDRenderData HUDDataExtractor::ExtractPlayerHUD(const Tank& player, int playerId) {
    HUDRenderData hudData;
    
    // Basic player information
    hudData.playerId = playerId;
    hudData.playerPosition = Vector3(player.x, player.y, player.z);
    hudData.playerRotation = Vector3(player.rx, player.ry + player.rty, player.rz);
    
    // Health and energy data
    hudData.health = player.energy;  // Note: In Tank class, 'energy' is actually health
    hudData.maxHealth = player.maxEnergy;
    hudData.energy = player.energy;
    hudData.maxEnergy = player.maxEnergy;
    hudData.charge = player.charge;
    hudData.maxCharge = player.maxCharge;
    hudData.fireCost = player.fireCost;
    
    // Calculate firing ability
    hudData.canFire = (player.charge >= player.fireCost);
    
    // Extract combo and special data
    ExtractComboAndSpecialData(player, playerId, hudData);
    
    // Set HUD colors based on player state
    SetHUDColors(player, hudData);
    
    // Extract performance data
    ExtractPerformanceData(hudData);
    
    // Calculate targeting animation time
    hudData.targetingAnimationTime = CalculateTargetingAnimationTime();
    
    // Set visibility flags based on game state
    hudData.showTargeting = true;
    hudData.showHealthBar = true;
    hudData.showEnergyBar = true;
    hudData.showComboMeter = true;
    hudData.showSpecialMeter = true;
    hudData.showFPSCounter = true;
    
    // Debug mode detection
    if (App::GetSingleton().gameTask) {
        hudData.showDebugInfo = App::GetSingleton().gameTask->IsDebugMode();
    }
    
    return hudData;
}

std::vector<HUDRenderData> HUDDataExtractor::ExtractAllPlayerHUDs(
    const std::array<Tank, 2>& players, int numPlayers) {
    
    std::vector<HUDRenderData> playerHUDs;
    playerHUDs.reserve(numPlayers);
    
    for (int i = 0; i < numPlayers && i < 2; ++i) {
        playerHUDs.push_back(ExtractPlayerHUD(players[i], i));
    }
    
    return playerHUDs;
}

UIRenderData HUDDataExtractor::ExtractCompleteUIData(
    const TankHandler& tankHandler,
    bool gameStarted,
    bool isPaused,
    bool showMenu,
    int menuState,
    bool showDebug) {
    
    UIRenderData uiData;
    
    // Extract player HUD data
    uiData.playerHUDs = ExtractAllPlayerHUDs(tankHandler.players, tankHandler.numPlayers);
    
    // Extract menu data
    uiData.menu = ExtractMenuData(menuState, showMenu);
    
    // Extract debug data
    uiData.debug = ExtractDebugData(showDebug);
    
    // Set global UI state
    uiData.gameStarted = gameStarted;
    uiData.isPaused = isPaused;
    uiData.numPlayers = tankHandler.numPlayers;
    
    return uiData;
}

MenuRenderData HUDDataExtractor::ExtractMenuData(int menuState, bool isVisible) {
    MenuRenderData menuData;
    
    menuData.isVisible = isVisible;
    menuData.selectedOption = menuState; // menuState indicates which option is selected
    
    // Set up the tank game menu options based on the original DrawMenu logic
    menuData.type = MenuType::MAIN_MENU; // For now, assume main menu
    
    // Based on the original menu system, there are 3 options (0, 1, 2)
    menuData.menuOptions = {"Start Game", "Options", "Quit"};
    menuData.optionEnabled = {true, true, true};
    menuData.numOptions = 3;
    
    // Ensure selected option is within bounds
    if (menuData.selectedOption < 0) {
        menuData.selectedOption = 0;
    } else if (menuData.selectedOption >= menuData.numOptions) {
        menuData.selectedOption = menuData.numOptions - 1;
    }
    
    // Set colors based on the original menu appearance
    menuData.backgroundColor = Vector3(0.0f, 0.0f, 0.1f);    // Dark background
    menuData.selectedColor = Vector3(0.0f, 1.0f, 1.0f);      // Cyan for selected (line)
    menuData.normalColor = Vector3(0.5f, 0.5f, 0.5f);        // Gray for normal
    menuData.disabledColor = Vector3(0.3f, 0.3f, 0.3f);      // Darker gray for disabled
    
    // Menu selection fill color (from original: glColor4f(0.0f,0.4f,0.4f, 0.1f))
    menuData.fadeAlpha = 1.0f;
    menuData.selectionAnimationTime = 0.0f; // TODO: Add animation if needed
    
    return menuData;
}

DebugRenderData HUDDataExtractor::ExtractDebugData(bool showDebug) {
    DebugRenderData debugData;
    
    debugData.showDebugInfo = showDebug;
    debugData.showPerformanceStats = showDebug;
    debugData.showRenderStats = showDebug;
    
    if (showDebug) {
        // Extract performance metrics
        debugData.frameTime = GlobalTimer::dT * 1000.0f; // Convert to milliseconds
        debugData.averageFPS = 1.0f / GlobalTimer::dT;
        
        // TODO: Extract render stats from rendering pipeline when available
        debugData.renderCallCount = 0;
        debugData.triangleCount = 0;
    }
    
    return debugData;
}

Vector3 HUDDataExtractor::CalculateTargetingDirection(const Tank& player) {
    // Calculate targeting direction based on tank rotation
    float angle = (player.ry + player.rty) * (M_PI / 180.0f); // Convert to radians
    float distance = 32.0f; // Targeting line length
    
    return Vector3(
        player.x + distance * cos(angle),
        player.y + 0.1f, // Slightly above ground
        player.z + distance * sin(angle)
    );
}

float HUDDataExtractor::CalculateTargetingAnimationTime() {
    // Create animated stipple pattern for targeting line
    static int animationCounter = 1;
    
    if (animationCounter > 60000) {
        animationCounter = 1;
    }
    
    animationCounter *= 2;
    return static_cast<float>(animationCounter);
}

void HUDDataExtractor::SetHUDColors(const Tank& player, HUDRenderData& hudData) {
    // Health bar color changes based on health percentage
    float healthPercent = hudData.health / hudData.maxHealth;
    
    if (healthPercent > 0.7f) {
        hudData.healthColor = Vector3(0.4f, 1.0f, 0.4f); // Green for high health
    } else if (healthPercent > 0.3f) {
        hudData.healthColor = Vector3(1.0f, 1.0f, 0.4f); // Yellow for medium health
    } else {
        hudData.healthColor = Vector3(1.0f, 0.4f, 0.4f); // Red for low health
    }
    
    // Energy bar color (always same color scheme as original)
    float energyPercent = hudData.energy / hudData.maxEnergy;
    hudData.energyColor = Vector3(1.0f, 0.4f * energyPercent, 0.6f * energyPercent);
    
    // Charge bar color
    hudData.chargeColor = Vector3(0.5f, hudData.charge / hudData.maxCharge, 1.0f);
    
    // Targeting line color
    hudData.targetingColor = Vector3(1.0f, 0.0f, 0.0f);
    
    // Combo and special colors
    hudData.comboColor = Vector3(1.0f, 2.0f * (hudData.combo / 100.0f), 0.1f);
    hudData.specialColor = Vector3(0.5f, hudData.special / 100.0f, 1.0f);
}

void HUDDataExtractor::ExtractComboAndSpecialData(const Tank& player, int playerId, HUDRenderData& hudData) {
    // Access TankHandler singleton for combo and special data
    // Note: This uses the existing array indexing pattern from the original code
    int arrayIndex = (-1 * player.id) - 1; // Convert player ID to array index
    
    // Safely access arrays with bounds checking
    if (arrayIndex >= 0 && arrayIndex < TankHandler::GetSingleton().numPlayers) {
        hudData.combo = TankHandler::GetSingleton().combo[arrayIndex];
        hudData.special = TankHandler::GetSingleton().special[arrayIndex];
        hudData.comboNumber = TankHandler::GetSingleton().comboNum[arrayIndex];
        
        // Check if special ability is available
        hudData.hasSpecialAvailable = (hudData.special >= hudData.fireCost / 5.0f);
    } else {
        // Default values if array access would be out of bounds
        hudData.combo = 0.0f;
        hudData.special = 0.0f;
        hudData.comboNumber = 0;
        hudData.hasSpecialAvailable = false;
    }
}

void HUDDataExtractor::ExtractPerformanceData(HUDRenderData& hudData) {
    // Extract current FPS and delta time
    hudData.currentFPS = 1.0f / GlobalTimer::dT;
    hudData.deltaTime = GlobalTimer::dT;
    
    // Clamp FPS to reasonable range for display
    if (hudData.currentFPS > 999.0f) {
        hudData.currentFPS = 999.0f;
    }
}
