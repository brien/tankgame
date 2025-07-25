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
#include "../TankHandler.h"
#include "RenderData.h"

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
    // Main rendering methods (legacy Tank interface)
    static void DrawPlayerTanks(const std::array<Tank, TankHandler::MAX_PLAYERS>& players, 
                               const std::array<float, TankHandler::MAX_PLAYERS>& special,
                               int numPlayers, 
                               bool hasEnemyTargets);
    
    static void DrawPlayerTank(const Tank& player, float drift);
    static void DrawPlayerEffects(const Tank& player, const std::array<float, TankHandler::MAX_PLAYERS>& special, 
                                 int playerIndex, float drift);
    static void DrawTargetingUI(const Tank& player, float drift);

    // New TankRenderData interface
    static void RenderPlayerTank(const TankRenderData& tank, float drift);
    static void RenderPlayerEffects(const TankRenderData& tank, float drift);
    static void RenderTargetingUI(const TankRenderData& tank, float drift, bool hasEnemyTargets);

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
    
    // Internal rendering helpers (legacy Tank interface)
    static void RenderTankBody(const Tank& player);
    static void RenderTankTurret(const Tank& player);
    static void RenderEffectBody(const Tank& player);
    static void RenderEffectTurret(const Tank& player, const std::array<float, TankHandler::MAX_PLAYERS>& special, int playerIndex);
    static void RenderTargetingIndicator(const Tank& player);
    static void RenderReadyIndicator(const Tank& player, float drift);
    
    // Internal rendering helpers (TankRenderData interface)
    static void RenderTankBody(const TankRenderData& tank);
    static void RenderTankTurret(const TankRenderData& tank);
    static void RenderEffectBody(const TankRenderData& tank);
    static void RenderEffectTurret(const TankRenderData& tank);
    static void RenderTargetingIndicator(const TankRenderData& tank);
    static void RenderReadyIndicator(const TankRenderData& tank, float drift);
};
