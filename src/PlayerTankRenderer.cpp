#include "PlayerTankRenderer.h"
#include "Tank.h"
#include "App.h"
#include "GlobalTimer.h"

void PlayerTankRenderer::DrawPlayerTanks(const std::array<Tank, 4>& players,
                                        const std::array<float, 2>& special,
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
    glColor3f(player.r, player.g, player.b);

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
    
    if (player.fireTimer > player.fireRate && player.fireCost < player.charge)
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
    glColor3f(player.r2, player.g2, player.b2);
    App::GetSingleton().graphicsTask->bodylist.Call(0);
}

void PlayerTankRenderer::RenderTankTurret(const Tank& player)
{
    // Draw turret (continuing from body transformation)
    glTranslatef(0, TURRET_HEIGHT_OFFSET, 0);
    glRotatef(player.rtx, 1, 0, 0);
    glRotatef(-player.rty + 180, 0, 1, 0);
    glRotatef(player.rtz, 0, 0, 1);

    glColor3f(player.r, player.g, player.b);
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

    glColor3f(player.r2, player.g2, player.b2);

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
