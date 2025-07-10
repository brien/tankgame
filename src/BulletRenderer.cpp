#include "BulletRenderer.h"
#include "Tank.h"
#include "TankHandler.h"
#include "Bullet.h"

void BulletRenderer::DrawAllBullets(const std::vector<Tank>& enemyTanks,
                                   const std::array<Tank, TankHandler::MAX_PLAYERS>& players,
                                   int numPlayers)
{
    SetupBulletRenderState();
    
    DrawEnemyBullets(enemyTanks);
    DrawPlayerBullets(players, numPlayers);
    
    RestoreBulletRenderState();
}

void BulletRenderer::DrawEnemyBullets(const std::vector<Tank>& enemyTanks)
{
    for (const Tank& tank : enemyTanks)
    {
        DrawBulletsFromTank(tank);
    }
}

void BulletRenderer::DrawPlayerBullets(const std::array<Tank, TankHandler::MAX_PLAYERS>& players, int numPlayers)
{
    for (int k = 0; k < numPlayers; k++)
    {
        DrawBulletsFromTank(players[k]);
    }
}

// Private helper methods

void BulletRenderer::SetupBulletRenderState()
{
    glDisable(GL_TEXTURE_2D);
}

void BulletRenderer::RestoreBulletRenderState()
{
    // Restore any render state if needed
    // Currently bullets handle their own state management
}

void BulletRenderer::DrawBulletsFromTank(const Tank& tank)
{
    for (const Bullet& bullet : tank.bullets)
    {
        if (bullet.IsAlive())
        {
            bullet.Draw();
        }
    }
}
