#pragma once

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <array>

// Forward declarations
class Tank;

/**
 * @brief Handles rendering of player tanks with special effects and visual enhancements
 * 
 * This class is responsible for rendering player tanks with advanced visual effects,
 * including textured effects, targeting UI, and special charge indicators.
 * Separated from TankHandler to improve code organization and maintainability.
 */
class PlayerTankRenderer 
{
public:
    // Main rendering methods
    static void DrawPlayerTanks(const std::array<Tank, 4>& players, 
                               const std::array<float, 2>& special,
                               int numPlayers, 
                               bool hasEnemyTargets);
    
    static void DrawPlayerTank(const Tank& player, float drift);
    static void DrawPlayerEffects(const Tank& player, const std::array<float, 2>& special, 
                                 int playerIndex, float drift);
    static void DrawTargetingUI(const Tank& player, float drift);

    // Rendering constants (extracted from TankHandler)
    static constexpr float TANK_HEIGHT_OFFSET = 0.05f;
    static constexpr float TURRET_HEIGHT_OFFSET = 0.10f;
    static constexpr float TARGETING_HEIGHT_OFFSET = 0.25f;
    static constexpr float TARGETING_EFFECT_OFFSET = 0.018f;
    static constexpr float DISTANCE_COLOR_FACTOR = 50.0f;
    static constexpr float TEXTURE_DRIFT_SPEED = 100.0f;
    static constexpr float DRIFT_RESET_THRESHOLD = 1.0f;
    static constexpr float ROTATION_EFFECT_SPEED = 100.0f;
    static constexpr float EFFECT_SCALE_FACTOR = 0.2f;
    static constexpr float EFFECT_SCALE_RESTORE = 5.0f;
    static constexpr float SPECIAL_CHARGE_THRESHOLD_DIVISOR = 5.0f;

private:
    // Helper methods for organizing the rendering pipeline
    static void SetupPlayerTankRenderState();
    static void SetupPlayerEffectsRenderState();
    static void SetupTargetingUIRenderState();
    static void RestoreRenderState();
    
    // Internal rendering helpers
    static void RenderTankBody(const Tank& player);
    static void RenderTankTurret(const Tank& player);
    static void RenderEffectBody(const Tank& player);
    static void RenderEffectTurret(const Tank& player, const std::array<float, 2>& special, int playerIndex);
    static void RenderTargetingIndicator(const Tank& player);
    static void RenderReadyIndicator(const Tank& player, float drift);
};
