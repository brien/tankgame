#include "BulletRenderer.h"
#include <GL/gl.h>

void BulletRenderer::Draw(const std::vector<Bullet>& bullets)
{
    SetupBulletRenderState();
    for (const Bullet& bullet : bullets)
    {
        if (bullet.IsAlive())
        {
            bullet.Draw();
        }
    }
    RestoreBulletRenderState();
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
