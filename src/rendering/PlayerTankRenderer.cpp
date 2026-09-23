#include "PlayerTankRenderer.h"
#include "../Tank.h"
#include "../App.h"
#include "../GlobalTimer.h"
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

#ifndef __EMSCRIPTEN__
void PlayerTankRenderer::DrawPlayerTanks(const std::array<Tank, TankHandler::MAX_PLAYERS>& players,
                                        const std::array<float, TankHandler::MAX_PLAYERS>& special,
                                        int numPlayers,
                                        bool hasEnemyTargets)
{
    static float drift = 0;
    drift += GlobalTimer::dT;
    if (drift > DRIFT_RESET_THRESHOLD)
        drift = 0;

    for (int k = 0; k < numPlayers; ++k)
    {
        if (players[k].alive)
        {
            DrawPlayerTank(players[k], drift);
            DrawPlayerEffects(players[k], special, k, drift);
        }

        if (hasEnemyTargets)
        {
            DrawTargetingUI(players[k], drift);
        }
    }
}

void PlayerTankRenderer::DrawPlayerTank(const Tank& player, float drift)
{
    SetupPlayerTankRenderState();
    
    glPushMatrix();
    RenderTankBody(player);
    RenderTankTurret(player);
    glPopMatrix();
    
    RestoreRenderState();
}

void PlayerTankRenderer::DrawPlayerEffects(const Tank& player, 
                                          const std::array<float, 2>& special,
                                          int playerIndex, 
                                          float drift)
{
    SetupPlayerEffectsRenderState();
    
    // Awesome effects are performed using the texture matrix stack
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glTranslatef(TEXTURE_DRIFT_SPEED * drift, 0, 0);
    glMatrixMode(GL_MODELVIEW);

    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[17]);
    Color primaryColor = player.GetPrimaryColor();
    glColor3f(primaryColor.r, primaryColor.g, primaryColor.b);

    glPushMatrix();
    RenderEffectBody(player);
    RenderEffectTurret(player, special, playerIndex);
    glPopMatrix();

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDepthMask(GL_TRUE);
    glPopMatrix();
}

void PlayerTankRenderer::DrawTargetingUI(const Tank& player, float drift)
{
    SetupTargetingUIRenderState();
    
    glPushMatrix();
    glTranslatef(player.x, player.y + TARGETING_HEIGHT_OFFSET, player.z);
    glRotatef(-player.rrl, 0, 1, 0);

    RenderTargetingIndicator(player);
    
    if (player.fireTimer > player.fireRate && player.fireCost < player.energy)
    {
        RenderReadyIndicator(player, drift);
    }

    RestoreRenderState();
    glRotatef(player.rrl, 0, 1, 0);
    glTranslatef(-player.x, -player.y + TARGETING_HEIGHT_OFFSET, -player.z);
    glPopMatrix();
}

// Private helper methods

void PlayerTankRenderer::SetupPlayerTankRenderState()
{
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void PlayerTankRenderer::SetupPlayerEffectsRenderState()
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void PlayerTankRenderer::SetupTargetingUIRenderState()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
}

void PlayerTankRenderer::RestoreRenderState()
{
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
}

void PlayerTankRenderer::RenderTankBody(const Tank& player)
{
    glPushMatrix();
    glTranslatef(player.x, player.y + TANK_HEIGHT_OFFSET, player.z);
    glRotatef(player.rx, 1, 0, 0);
    glRotatef(-player.ry - 90, 0, 1, 0);
    glRotatef(player.rz, 0, 0, 1);

    glFrontFace(GL_CCW);
    Color secondaryColor = player.GetSecondaryColor();
    glColor3f(secondaryColor.r, secondaryColor.g, secondaryColor.b);
    App::GetSingleton().graphicsTask->bodylist.Call(0);
}

void PlayerTankRenderer::RenderTankTurret(const Tank& player)
{
    // Draw turret (continuing from body transformation)
    glTranslatef(0, TURRET_HEIGHT_OFFSET, 0);
    glRotatef(player.rtx, 1, 0, 0);
    glRotatef(-player.rty + 180, 0, 1, 0);
    glRotatef(player.rtz, 0, 0, 1);

    Color primaryColor = player.GetPrimaryColor();
    glColor3f(primaryColor.r, primaryColor.g, primaryColor.b);
    App::GetSingleton().graphicsTask->turretlist.Call(0);

    glFrontFace(GL_CW);
    glPopMatrix();
}

void PlayerTankRenderer::RenderEffectBody(const Tank& player)
{
    glDisable(GL_LIGHTING);
    glTranslatef(player.x, player.y + TANK_HEIGHT_OFFSET, player.z);
    glRotatef(player.rx, 1, 0, 0);
    glRotatef(-player.ry - 90, 0, 1, 0);
    glRotatef(player.rz, 0, 0, 1);

    glFrontFace(GL_CCW);
    App::GetSingleton().graphicsTask->bodylist.Call(0);
}

void PlayerTankRenderer::RenderEffectTurret(const Tank& player, 
                                           const std::array<float, 2>& special,
                                           int playerIndex)
{
    // Draw effect turret
    glTranslatef(0, TURRET_HEIGHT_OFFSET, 0);
    glRotatef(player.rtx, 1, 0, 0);
    glRotatef(-player.rty + 180, 0, 1, 0);
    glRotatef(player.rtz, 0, 0, 1);

    Color secondaryColor = player.GetSecondaryColor();
    glColor3f(secondaryColor.r, secondaryColor.g, secondaryColor.b);

    if (special[playerIndex] >= player.fireCost / SPECIAL_CHARGE_THRESHOLD_DIVISOR)
    {
        App::GetSingleton().graphicsTask->turretlist.Call(0);
    }

    glFrontFace(GL_CW);
}

void PlayerTankRenderer::RenderTargetingIndicator(const Tank& player)
{
    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[20]);
    glColor4f(1.0f, player.dist / DISTANCE_COLOR_FACTOR, 0.1f, 1.0);
    App::GetSingleton().graphicsTask->squarelist.Call(0);
}

void PlayerTankRenderer::RenderReadyIndicator(const Tank& player, float drift)
{
    glTranslatef(0, +TARGETING_EFFECT_OFFSET, 0);
    glRotatef(ROTATION_EFFECT_SPEED * drift, 0, 1, 0);

    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[17]);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glScalef(EFFECT_SCALE_FACTOR, EFFECT_SCALE_FACTOR, EFFECT_SCALE_FACTOR);
    App::GetSingleton().graphicsTask->squarelist.Call(0);
    glScalef(EFFECT_SCALE_RESTORE, EFFECT_SCALE_RESTORE, EFFECT_SCALE_RESTORE);
}
#endif

#ifdef __EMSCRIPTEN__
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
#endif

// ===== TankRenderData Interface Implementations =====

void PlayerTankRenderer::RenderPlayerTank(const TankRenderData& tank, float drift)
{
    SetupPlayerTankRenderState();
#ifndef __EMSCRIPTEN__
    glPushMatrix();
#endif
    RenderTankBody(tank);
    RenderTankTurret(tank);
#ifndef __EMSCRIPTEN__
    glPopMatrix();
#endif
    
    RestoreRenderState();
}

void PlayerTankRenderer::RenderPlayerEffects(const TankRenderData& tank, float drift)
{
    SetupPlayerEffectsRenderState();
#ifdef __EMSCRIPTEN__
    RenderEffectBody(tank);
    RenderEffectTurret(tank);
#else
    // Awesome effects are performed using the texture matrix stack
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glTranslatef(TEXTURE_DRIFT_SPEED * drift, 0, 0);
    glMatrixMode(GL_MODELVIEW);

    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[17]);
    glColor3f(tank.primaryColor.r, tank.primaryColor.g, tank.primaryColor.b);

    glPushMatrix();
    RenderEffectBody(tank);
    RenderEffectTurret(tank);
    glPopMatrix();

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDepthMask(GL_TRUE);
    glPopMatrix();
#endif
}

void PlayerTankRenderer::RenderTargetingUI(const TankRenderData& tank, float drift, bool hasEnemyTargets)
{
    if (!hasEnemyTargets) return;
    
    SetupTargetingUIRenderState();
    
#ifdef __EMSCRIPTEN__
    RenderTargetingIndicator(tank);
    if (tank.charge >= tank.fireCost)
        RenderReadyIndicator(tank, drift);
#else
    glPushMatrix();
    glTranslatef(tank.position.x, tank.position.y + TARGETING_HEIGHT_OFFSET, tank.position.z);
    glRotatef(-tank.targetRotation, 0, 1, 0);

    RenderTargetingIndicator(tank);
    
    if (tank.charge >= tank.fireCost)
    {
        RenderReadyIndicator(tank, drift);
    }

    RestoreRenderState();
    glRotatef(tank.targetRotation, 0, 1, 0);
    glTranslatef(-tank.position.x, -tank.position.y + TARGETING_HEIGHT_OFFSET, -tank.position.z);
    glPopMatrix();
#endif
}

// TankRenderData helper methods

void PlayerTankRenderer::RenderTankBody(const TankRenderData& tank)
{
#ifdef __EMSCRIPTEN__
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->bodylist,
        PlayerBodyModel(tank), tank.secondaryColor.r, tank.secondaryColor.g,
        tank.secondaryColor.b);
#else
    glPushMatrix();
    glTranslatef(tank.position.x, tank.position.y + TANK_HEIGHT_OFFSET, tank.position.z);
    glRotatef(tank.bodyRotation.x, 1, 0, 0);
    glRotatef(-tank.bodyRotation.y - 90, 0, 1, 0);
    glRotatef(tank.bodyRotation.z, 0, 0, 1);

    glFrontFace(GL_CCW);
    glColor3f(tank.secondaryColor.r, tank.secondaryColor.g, tank.secondaryColor.b);
    App::GetSingleton().graphicsTask->bodylist.Call(0);
#endif
}

void PlayerTankRenderer::RenderTankTurret(const TankRenderData& tank)
{
#ifdef __EMSCRIPTEN__
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->turretlist,
        PlayerTurretModel(tank), tank.primaryColor.r, tank.primaryColor.g,
        tank.primaryColor.b);
#else
    // Draw turret (continuing from body transformation)
    glTranslatef(0, TURRET_HEIGHT_OFFSET, 0);
    glRotatef(tank.turretRotation.x, 1, 0, 0);
    glRotatef(-tank.turretRotation.y + 180, 0, 1, 0);
    glRotatef(tank.turretRotation.z, 0, 0, 1);

    glColor3f(tank.primaryColor.r, tank.primaryColor.g, tank.primaryColor.b);
    App::GetSingleton().graphicsTask->turretlist.Call(0);

    glFrontFace(GL_CW);
    glPopMatrix();
#endif
}

void PlayerTankRenderer::RenderEffectBody(const TankRenderData& tank)
{
#ifdef __EMSCRIPTEN__
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->bodylist,
        PlayerBodyModel(tank), tank.primaryColor.r, tank.primaryColor.g, tank.primaryColor.b);
#else
    glDisable(GL_LIGHTING);
    glTranslatef(tank.position.x, tank.position.y + TANK_HEIGHT_OFFSET, tank.position.z);
    glRotatef(tank.bodyRotation.x, 1, 0, 0);
    glRotatef(-tank.bodyRotation.y - 90, 0, 1, 0);
    glRotatef(tank.bodyRotation.z, 0, 0, 1);

    glFrontFace(GL_CCW);
    App::GetSingleton().graphicsTask->bodylist.Call(0);
#endif
}

void PlayerTankRenderer::RenderEffectTurret(const TankRenderData& tank)
{
#ifdef __EMSCRIPTEN__
    if (tank.charge >= tank.fireCost / SPECIAL_CHARGE_THRESHOLD_DIVISOR)
        RenderContext::Current().Draw(App::GetSingleton().graphicsTask->turretlist,
            PlayerTurretModel(tank), tank.secondaryColor.r, tank.secondaryColor.g,
            tank.secondaryColor.b);
#else
    // Draw effect turret
    glTranslatef(0, TURRET_HEIGHT_OFFSET, 0);
    glRotatef(tank.turretRotation.x, 1, 0, 0);
    glRotatef(-tank.turretRotation.y + 180, 0, 1, 0);
    glRotatef(tank.turretRotation.z, 0, 0, 1);

    glColor3f(tank.secondaryColor.r, tank.secondaryColor.g, tank.secondaryColor.b);

    if (tank.charge >= tank.fireCost / SPECIAL_CHARGE_THRESHOLD_DIVISOR)
    {
        App::GetSingleton().graphicsTask->turretlist.Call(0);
    }

    glFrontFace(GL_CW);
#endif
}

void PlayerTankRenderer::RenderTargetingIndicator(const TankRenderData& tank)
{
#ifdef __EMSCRIPTEN__
    const Matrix4 model = Matrix4::Translation(tank.position.x,
        tank.position.y + TARGETING_HEIGHT_OFFSET, tank.position.z) *
        Matrix4::Rotation(-tank.targetRotation, 0, 1, 0);
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->squarelist, model,
                                  1.0f, tank.health / 50.0f, 0.1f, 1.0f);
#else
    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[20]);
    glColor4f(1.0f, tank.health / 50.0f, 0.1f, 1.0); // Use health as distance approximation
    App::GetSingleton().graphicsTask->squarelist.Call(0);
#endif
}

void PlayerTankRenderer::RenderReadyIndicator(const TankRenderData& tank, float drift)
{
#ifdef __EMSCRIPTEN__
    const Matrix4 model = Matrix4::Translation(tank.position.x,
        tank.position.y + TARGETING_HEIGHT_OFFSET, tank.position.z) *
        Matrix4::Rotation(-tank.targetRotation, 0, 1, 0) *
        Matrix4::Translation(0, TARGETING_EFFECT_OFFSET, 0) *
        Matrix4::Rotation(ROTATION_EFFECT_SPEED * drift, 0, 1, 0) *
        Matrix4::Scale(EFFECT_SCALE_FACTOR, EFFECT_SCALE_FACTOR, EFFECT_SCALE_FACTOR);
    RenderContext::Current().Draw(App::GetSingleton().graphicsTask->squarelist, model,
                                  1.0f, 1.0f, 1.0f, 1.0f);
#else
    glTranslatef(0, +TARGETING_EFFECT_OFFSET, 0);
    glRotatef(ROTATION_EFFECT_SPEED * drift, 0, 1, 0);

    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[17]);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glScalef(EFFECT_SCALE_FACTOR, EFFECT_SCALE_FACTOR, EFFECT_SCALE_FACTOR);
    App::GetSingleton().graphicsTask->squarelist.Call(0);
    glScalef(EFFECT_SCALE_RESTORE, EFFECT_SCALE_RESTORE, EFFECT_SCALE_RESTORE);
#endif
}
