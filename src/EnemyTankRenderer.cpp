#include "EnemyTankRenderer.h"
#include "Tank.h"

void EnemyTankRenderer::DrawEnemyTanks(const std::vector<Tank>& enemyTanks)
{
    SetupEnemyTankRenderState();
    
    for (const Tank& tank : enemyTanks)
    {
        tank.Draw();
    }
    
    RestoreEnemyTankRenderState();
}

// Private helper methods

void EnemyTankRenderer::SetupEnemyTankRenderState()
{
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void EnemyTankRenderer::RestoreEnemyTankRenderState()
{
    // Restore any render state if needed
    // Currently enemy tanks use standard rendering state
}
