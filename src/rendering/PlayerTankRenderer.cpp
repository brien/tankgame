#include "PlayerTankRenderer.h"
#include "../App.h"
#include "RenderContext.h"

namespace
{
Matrix4 PlayerBodyModel(const TankRenderData& tank)
{
    return Matrix4::Translation(tank.position.x,
                                tank.position.y + PlayerTankRenderer::TANK_HEIGHT_OFFSET,
                                tank.position.z) *
        Matrix4::Rotation(tank.bodyRotation.x, 1, 0, 0) *
        Matrix4::Rotation(-tank.bodyRotation.y - 90, 0, 1, 0) *
        Matrix4::Rotation(tank.bodyRotation.z, 0, 0, 1);
}

Matrix4 PlayerTurretModel(const TankRenderData& tank)
{
    return PlayerBodyModel(tank) *
        Matrix4::Translation(0, PlayerTankRenderer::TURRET_HEIGHT_OFFSET, 0) *
        Matrix4::Rotation(tank.turretRotation.x, 1, 0, 0) *
        Matrix4::Rotation(-tank.turretRotation.y + 180, 0, 1, 0) *
        Matrix4::Rotation(tank.turretRotation.z, 0, 0, 1);
}
}

void PlayerTankRenderer::SetupPlayerTankRenderState()
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void PlayerTankRenderer::SetupPlayerEffectsRenderState()
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void PlayerTankRenderer::SetupTargetingUIRenderState()
{
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
}

void PlayerTankRenderer::RestoreRenderState()
{
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

// ===== TankRenderData Interface Implementations =====

void PlayerTankRenderer::RenderPlayerTank(const TankRenderData& tank, float drift)
{
    SetupPlayerTankRenderState();
    RenderTankBody(tank);
    RenderTankTurret(tank);
    
    RestoreRenderState();
}

void PlayerTankRenderer::RenderPlayerEffects(const TankRenderData& tank, float drift)
{
    SetupPlayerEffectsRenderState();
    RenderEffectBody(tank);
    RenderEffectTurret(tank);
}

void PlayerTankRenderer::RenderTargetingUI(const TankRenderData& tank, float drift, bool hasEnemyTargets)
{
    if (!hasEnemyTargets) return;
    
    SetupTargetingUIRenderState();
    
    RenderTargetingIndicator(tank);
    if (tank.charge >= tank.fireCost)
        RenderReadyIndicator(tank, drift);
    RestoreRenderState();
}

// TankRenderData helper methods

void PlayerTankRenderer::RenderTankBody(const TankRenderData& tank)
{
    glFrontFace(GL_CCW);
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::TankBody),
        PlayerBodyModel(tank), tank.secondaryColor.r, tank.secondaryColor.g,
        tank.secondaryColor.b);
}

void PlayerTankRenderer::RenderTankTurret(const TankRenderData& tank)
{
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::TankTurret),
        PlayerTurretModel(tank), tank.primaryColor.r, tank.primaryColor.g,
        tank.primaryColor.b);
    glFrontFace(GL_CW);
}

void PlayerTankRenderer::RenderEffectBody(const TankRenderData& tank)
{
    BasicMaterial material = App::GetSingleton().graphicsTask->Resources().GetMaterial(MaterialResource::StarOverlay);
    material.red = tank.primaryColor.r;
    material.green = tank.primaryColor.g;
    material.blue = tank.primaryColor.b;
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::TankBody),
                                  PlayerBodyModel(tank), material);
}

void PlayerTankRenderer::RenderEffectTurret(const TankRenderData& tank)
{
    if (tank.charge >= tank.specialChargeCost) {
        BasicMaterial material = App::GetSingleton().graphicsTask->Resources().GetMaterial(MaterialResource::StarOverlay);
        material.red = tank.secondaryColor.r;
        material.green = tank.secondaryColor.g;
        material.blue = tank.secondaryColor.b;
        RenderContext::Current().Draw(App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::TankTurret),
                                      PlayerTurretModel(tank), material);
    }
}

void PlayerTankRenderer::RenderTargetingIndicator(const TankRenderData& tank)
{
    const Matrix4 model = Matrix4::Translation(tank.position.x,
        tank.position.y + TARGETING_HEIGHT_OFFSET, tank.position.z) *
        Matrix4::Rotation(-tank.targetRotation, 0, 1, 0);
    BasicMaterial material = App::GetSingleton().graphicsTask->Resources().GetMaterial(MaterialResource::RingOverlay);
    material.red = 1.0f;
    material.green = tank.health / 50.0f;
    material.blue = 0.1f;
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::HorizontalQuad), model, material);
}

void PlayerTankRenderer::RenderReadyIndicator(const TankRenderData& tank, float drift)
{
    const Matrix4 model = Matrix4::Translation(tank.position.x,
        tank.position.y + TARGETING_HEIGHT_OFFSET, tank.position.z) *
        Matrix4::Rotation(-tank.targetRotation, 0, 1, 0) *
        Matrix4::Translation(0, TARGETING_EFFECT_OFFSET, 0) *
        Matrix4::Rotation(ROTATION_EFFECT_SPEED * drift, 0, 1, 0) *
        Matrix4::Scale(EFFECT_SCALE_FACTOR, EFFECT_SCALE_FACTOR, EFFECT_SCALE_FACTOR);
    BasicMaterial material = App::GetSingleton().graphicsTask->Resources().GetMaterial(MaterialResource::StarOverlay);
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::HorizontalQuad), model, material);
}
