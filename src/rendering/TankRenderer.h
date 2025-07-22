#pragma once

#include "ITankRenderer.h"
#include "BaseRenderer.h"
#include "RenderData.h"
#include <vector>

/**
 * Unified tank renderer following the new rendering architecture.
 * Handles rendering of all tank types (player, enemy) using TankRenderData.
 * Replaces the existing TankRenderer, PlayerTankRenderer, and EnemyTankRenderer.
 * 
 * Implements the ITankRenderer interface while inheriting from BaseRenderer
 * for common rendering functionality.
 */
class TankRenderer : public BaseRenderer, public ITankRenderer {
public:
    TankRenderer();
    virtual ~TankRenderer() = default;
    
    // ITankRenderer interface implementation
    bool Initialize() override;
    void Cleanup() override;
    void Render(const TankRenderData& data) override;
    void RenderMultiple(const std::vector<TankRenderData>& tanks) override;
    void SetupRenderState() override;
    void CleanupRenderState() override;
    
    /**
     * Legacy method for backward compatibility.
     * @deprecated Use RenderMultiple() instead
     */
    void RenderTanks(const std::vector<TankRenderData>& tanks);
    
    /**
     * Legacy method for backward compatibility.
     * @deprecated Use Render() instead
     */
    void RenderTank(const TankRenderData& tank);
    
protected:
    
private:
    /**
     * Renders the tank body using display lists and color.
     * @param tank Tank render data
     */
    void RenderTankBody(const TankRenderData& tank);
    
    /**
     * Renders the tank turret/cannon using display lists and color.
     * @param tank Tank render data
     */
    void RenderTankTurret(const TankRenderData& tank);
    
    /**
     * Renders special effects for tanks (hit flash, charge effects, etc.).
     * @param tank Tank render data
     */
    void RenderTankEffects(const TankRenderData& tank);
    
    /**
     * Renders HUD elements for player tanks (health, charge bars, targeting UI).
     * @param tank Tank render data
     */
    void RenderTankHUD(const TankRenderData& tank);
    
    /**
     * Sets up color based on tank type and health.
     * @param tank Tank render data
     * @param isPrimary Whether to use primary (r,g,b) or secondary (r2,g2,b2) color
     */
    void SetTankColor(const TankRenderData& tank, bool isPrimary = true);
    
    /**
     * Applies health-based color modification.
     * @param r Red component
     * @param g Green component  
     * @param b Blue component
     * @param health Current health
     * @param maxHealth Maximum health
     */
    void ApplyHealthColor(float& r, float& g, float& b, float health, float maxHealth);
    
    /**
     * Renders a player tank with full effects and UI.
     * @param tank Tank render data for a player tank
     */
    void RenderPlayerTank(const TankRenderData& tank);
    
    /**
     * Renders an enemy tank with basic rendering.
     * @param tank Tank render data for an enemy tank
     */
    void RenderEnemyTank(const TankRenderData& tank);
    
    /**
     * Renders basic tank geometry (body and turret) common to both player and enemy tanks.
     * @param tank Tank render data
     */
    void RenderBasicTank(const TankRenderData& tank);
    
    /**
     * Renders player-specific visual effects (charge effects, texture animations).
     * @param tank Tank render data for a player tank
     */
    void RenderPlayerEffects(const TankRenderData& tank);
    
    /**
     * Renders player targeting UI elements.
     * @param tank Tank render data for a player tank
     */
    void RenderPlayerTargetingUI(const TankRenderData& tank);
    
    // Hardcoded geometry methods (from TankRenderer.cpp)
    void DrawEnemyTankBody(float r, float g, float b, float energy, float maxEnergy);
    void DrawEnemyTankBarrel(float r, float g, float b, float energy, float maxEnergy);
    void DrawEnemyTankTurret();
};
