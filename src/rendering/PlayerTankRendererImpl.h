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
    void SetupRenderState() override;
    void CleanupRenderState() override;
    
    // PlayerTankRenderer-specific methods
    void RenderPlayerEffects(const TankRenderData& tank);
    void RenderTargetingUI(const TankRenderData& tank, bool hasEnemyTargets);
    
private:
    // Animation state
    float animationDrift;
    
    // Internal rendering helpers
    void RenderTankBody(const TankRenderData& tank);
    void RenderTankTurret(const TankRenderData& tank);
    void RenderEffectBody(const TankRenderData& tank);
    void RenderEffectTurret(const TankRenderData& tank);
    void RenderTargetingIndicator(const TankRenderData& tank);
    void RenderReadyIndicator(const TankRenderData& tank);
    
    // Render state management
    void SetupPlayerTankRenderState();
    void SetupPlayerEffectsRenderState();
    void SetupTargetingUIRenderState();
    
    // Constants (from PlayerTankRenderer)
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
};
