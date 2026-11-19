#pragma once

#include "ITankRenderer.h"
#include "BaseRenderer.h"
#include "RenderData.h"

/**
 * Player tank renderer implementing the ITankRenderer interface.
 * 
 * This renderer specializes in rendering player tanks with advanced visual effects,
 * targeting UI, charge indicators, and special animations. It provides high-quality
 * rendering optimized for the player experience.
 * 
 * Features:
 * - Textured tank effects with animated drift
 * - Targeting UI with enemy indicators
 * - Charge-based visual feedback
 * - Health-based color modifications
 * - Special effect rendering for bonuses
 */
class PlayerTankRendererImpl : public BaseRenderer, public ITankRenderer {
public:
    PlayerTankRendererImpl();
    virtual ~PlayerTankRendererImpl() = default;
    
    // ITankRenderer interface implementation
    bool Initialize() override;
    void Cleanup() override;
    void Render(const TankRenderData& data) override;
    void RenderOpaque(const TankRenderData& data) override;
    void RenderOverlays(const TankRenderData& data) override;
    void SetupRenderState() override;
    void CleanupRenderState() override;
    
    // PlayerTankRenderer-specific methods
    void RenderPlayerEffects(const TankRenderData& tank);
    void RenderTargetingUI(const TankRenderData& tank, bool hasEnemyTargets);
    
private:
    // Animation state
    float animationDrift;
    
    // Render state management
    void SetupPlayerTankRenderState();
    
    static constexpr float DRIFT_RESET_THRESHOLD = 1.0f;
};
