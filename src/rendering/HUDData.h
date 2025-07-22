#ifndef HUDDATA_H
#define HUDDATA_H

#include <vector>
#include <string>
#include "RenderData.h"  // For Vector3

/**
 * HUD Rendering Data Structures
 * These structures contain all data needed to render UI elements
 * without requiring access to game logic objects.
 */

/**
 * Data for rendering a single player's HUD
 */
struct HUDRenderData {
    // Player identification
    int playerId;                       // Player ID for this HUD
    Vector3 playerPosition;             // Player world position
    Vector3 playerRotation;             // Player rotation (body + turret for targeting line)
    
    // Health and energy bars
    float health;                       // Current health (0.0 - maxHealth)
    float maxHealth;                    // Maximum health
    float energy;                       // Current energy (0.0 - maxEnergy)  
    float maxEnergy;                    // Maximum energy
    float charge;                       // Current charge (0.0 - maxCharge)
    float maxCharge;                    // Maximum charge
    float fireCost;                     // Cost to fire (for charge indicator line)
    
    // Special abilities and combo system
    float combo;                        // Combo meter (0.0 - 100.0)
    float special;                      // Special ability meter (0.0 - 100.0)
    int comboNumber;                    // Current combo number for display
    
    // HUD element visibility and states
    bool showTargeting;                 // Show targeting line
    bool showHealthBar;                 // Show health bar
    bool showEnergyBar;                 // Show energy bar
    bool showComboMeter;                // Show combo meter
    bool showSpecialMeter;              // Show special ability meter
    bool showFPSCounter;                // Show FPS counter
    bool showDebugInfo;                 // Show debug information
    
    // Colors for UI elements
    Vector3 healthColor;                // Health bar color
    Vector3 energyColor;                // Energy bar color
    Vector3 chargeColor;                // Charge bar color
    Vector3 targetingColor;             // Targeting line color
    Vector3 comboColor;                 // Combo meter color
    Vector3 specialColor;               // Special meter color
    
    // Animation and visual effects
    float targetingAnimationTime;       // Animation time for targeting line stipple
    float reloadAnimationTime;          // Animation time for reload effects
    bool canFire;                       // Whether player can fire (affects UI highlighting)
    bool hasSpecialAvailable;           // Whether special ability is available
    
    // Performance data
    float currentFPS;                   // Current frames per second
    float deltaTime;                    // Current frame delta time for debug
    
    // Constructor with default values
    HUDRenderData() :
        playerId(0),
        playerPosition(0, 0, 0),
        playerRotation(0, 0, 0),
        health(100.0f),
        maxHealth(100.0f),
        energy(100.0f),
        maxEnergy(100.0f),
        charge(100.0f),
        maxCharge(100.0f),
        fireCost(10.0f),
        combo(0.0f),
        special(0.0f),
        comboNumber(0),
        showTargeting(true),
        showHealthBar(true),
        showEnergyBar(true),
        showComboMeter(true),
        showSpecialMeter(true),
        showFPSCounter(true),
        showDebugInfo(false),
        healthColor(1.0f, 0.4f, 0.6f),
        energyColor(1.0f, 0.4f, 0.6f),
        chargeColor(0.5f, 1.0f, 1.0f),
        targetingColor(1.0f, 0.0f, 0.0f),
        comboColor(1.0f, 1.0f, 0.1f),
        specialColor(0.5f, 1.0f, 1.0f),
        targetingAnimationTime(0.0f),
        reloadAnimationTime(0.0f),
        canFire(true),
        hasSpecialAvailable(false),
        currentFPS(60.0f),
        deltaTime(0.016f)
    {
    }
};

/**
 * Menu rendering data
 */
enum class MenuType {
    NONE,
    MAIN_MENU,
    PAUSE_MENU,
    SETTINGS_MENU,
    GAME_OVER_MENU,
    LEVEL_SELECT_MENU
};

struct MenuRenderData {
    MenuType type;                      // Type of menu being displayed
    int selectedOption;                 // Currently selected menu option
    int numOptions;                     // Total number of menu options
    std::vector<std::string> menuOptions;  // Text for each menu option
    std::vector<bool> optionEnabled;    // Whether each option is enabled
    
    // Menu styling
    Vector3 backgroundColor;            // Background color
    Vector3 selectedColor;              // Color for selected option
    Vector3 normalColor;                // Color for normal options
    Vector3 disabledColor;              // Color for disabled options
    
    // Animation state
    float fadeAlpha;                    // Menu fade in/out alpha
    float selectionAnimationTime;       // Animation time for selection highlight
    bool isVisible;                     // Whether menu is currently visible
    
    // Constructor
    MenuRenderData() :
        type(MenuType::NONE),
        selectedOption(0),
        numOptions(0),
        backgroundColor(0.0f, 0.0f, 0.1f),
        selectedColor(1.0f, 1.0f, 0.0f),
        normalColor(1.0f, 1.0f, 1.0f),
        disabledColor(0.5f, 0.5f, 0.5f),
        fadeAlpha(1.0f),
        selectionAnimationTime(0.0f),
        isVisible(false)
    {
    }
};

/**
 * Debug rendering data
 */
struct DebugRenderData {
    bool showDebugInfo;                 // Whether to show debug information
    bool showPerformanceStats;          // Show performance statistics
    bool showRenderStats;               // Show rendering statistics
    
    // Performance metrics
    float frameTime;                    // Current frame time in milliseconds
    float averageFPS;                   // Average FPS over time
    int renderCallCount;                // Number of render calls this frame
    int triangleCount;                  // Number of triangles rendered
    
    // Debug colors
    Vector3 debugTextColor;             // Color for debug text
    Vector3 performanceColor;           // Color for performance text
    
    DebugRenderData() :
        showDebugInfo(false),
        showPerformanceStats(false),
        showRenderStats(false),
        frameTime(16.0f),
        averageFPS(60.0f),
        renderCallCount(0),
        triangleCount(0),
        debugTextColor(1.0f, 1.0f, 0.0f),
        performanceColor(0.0f, 1.0f, 1.0f)
    {
    }
};

/**
 * Complete UI rendering data for all players
 */
struct UIRenderData {
    std::vector<HUDRenderData> playerHUDs;  // HUD data for each player
    MenuRenderData menu;                     // Current menu state
    DebugRenderData debug;                   // Debug information
    
    // Global UI state
    bool gameStarted;                        // Whether game has started
    bool isPaused;                           // Whether game is paused
    int numPlayers;                          // Number of active players
    
    UIRenderData() :
        gameStarted(false),
        isPaused(false),
        numPlayers(0)
    {
    }
};

#endif // HUDDATA_H
