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
    
protected:
    
private:
    
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
    
    // Hardcoded geometry methods (from TankRenderer.cpp)
    void DrawEnemyTankBody(float r, float g, float b, float energy, float maxEnergy);
    void DrawEnemyTankBarrel(float r, float g, float b, float energy, float maxEnergy);
    void DrawEnemyTankTurret();
};
