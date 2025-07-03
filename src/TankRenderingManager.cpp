#include "TankRenderingManager.h"
#include "PlayerTankRenderer.h"
#include "EnemyTankRenderer.h"
#include "BulletRenderer.h"
#include "Tank.h"

void TankRenderingManager::RenderAllTanks(const std::array<Tank, 4>& players,
                                         const std::vector<Tank>& enemyTanks,
                                         const std::array<float, 2>& special,
                                         int numPlayers)
{
    bool hasEnemyTargets = ShouldShowTargetingUI(enemyTanks);
    
    // Render player tanks with effects and targeting UI
    RenderPlayerTanks(players, special, numPlayers, hasEnemyTargets);
    
    // Render enemy tanks
    RenderEnemyTanks(enemyTanks);
}

void TankRenderingManager::RenderAllBullets(const std::array<Tank, 4>& players,
                                           const std::vector<Tank>& enemyTanks,
                                           int numPlayers)
{
    BulletRenderer::DrawAllBullets(enemyTanks, players, numPlayers);
}

void TankRenderingManager::RenderPlayerTanks(const std::array<Tank, 4>& players,
                                            const std::array<float, 2>& special,
                                            int numPlayers,
                                            bool hasEnemyTargets)
{
    PlayerTankRenderer::DrawPlayerTanks(players, special, numPlayers, hasEnemyTargets);
}

void TankRenderingManager::RenderEnemyTanks(const std::vector<Tank>& enemyTanks)
{
    EnemyTankRenderer::DrawEnemyTanks(enemyTanks);
}

// Private helper methods

bool TankRenderingManager::ShouldShowTargetingUI(const std::vector<Tank>& enemyTanks)
{
    return !enemyTanks.empty();
}
