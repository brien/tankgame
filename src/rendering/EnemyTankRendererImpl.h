#pragma once

#include "ITankRenderer.h"
#include "BaseRenderer.h"
#include "RenderData.h"

/**
 * Enemy tank renderer implementing the ITankRenderer interface.
 * 
 * This renderer specializes in rendering enemy tanks with basic geometry and
 * efficient rendering. It focuses on performance over visual effects since
 * enemy tanks are typically numerous and don't need the same level of detail
 * as player tanks.
 * 
 * Features:
 * - Hardcoded geometry for performance
 * - Health-based color modifications
 * - Minimal effects for efficiency
 * - Optimized for batch rendering
 */
class EnemyTankRendererImpl : public BaseRenderer, public ITankRenderer {
public:
    EnemyTankRendererImpl();
    virtual ~EnemyTankRendererImpl() = default;
    
    // ITankRenderer interface implementation
    bool Initialize() override;
    void Cleanup() override;
    void Render(const TankRenderData& data) override;
    void SetupRenderState() override;
    void CleanupRenderState() override;
    
    // EnemyTankRenderer-specific methods
    void RenderEnemyTankGeometry(const TankRenderData& tank);
    
private:
    // Internal rendering helpers (using hardcoded geometry from TankRenderer)
    void RenderTankBody(const TankRenderData& tank);
    void RenderTankBarrel(const TankRenderData& tank);
    void RenderTankTurret(const TankRenderData& tank);
    
    // Hardcoded geometry methods (from original TankRenderer.cpp)
    void DrawBody(float r, float g, float b, float energy, float maxEnergy);
    void DrawBarrel(float r, float g, float b, float energy, float maxEnergy);
    void DrawTurret();
    
    // Transform helpers
    void SetupBodyTransform(const TankRenderData& tank);
    void SetupBarrelTransform(const TankRenderData& tank);
    void SetupTurretTransform();
};
