#include "PlatformGL.h"

#include "BulletRenderer.h"
#include "RenderData.h"
#include "../App.h"
#include "../Logger.h"
#include "RenderContext.h"

BulletRenderer::BulletRenderer() : 
    BaseRenderer(),
    blendEnabled(false)
{
}

bool BulletRenderer::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }
    
    blendEnabled = false;
    
    Logger::Get().Write("BulletRenderer initialized successfully\n");
    return true;
}

void BulletRenderer::Cleanup() {
    BaseRenderer::Cleanup();
    Logger::Get().Write("BulletRenderer cleaned up\n");
}

void BulletRenderer::RenderBullets(const std::vector<BulletRenderData>& bullets) {
    if (!IsReady()) {
        Logger::Get().Write("ERROR: BulletRenderer not initialized\n");
        return;
    }
    
    if (bullets.empty()) {
        return;
    }
    
    // Save OpenGL state
    SetupRenderState();
    
    try {
        
        // Render all bullets
        for (const auto& bullet : bullets) {
            if (bullet.type1 == TankType::TYPE_BLUE) {
                RenderBlueBullet(bullet);
            } else {
                RenderStandardBullet(bullet);
            }
        }
        
        
    } catch (...) {
        Logger::Get().Write("ERROR: Exception occurred during bullet rendering\n");
    }
    
    // Restore OpenGL state
    CleanupRenderState();
}

void BulletRenderer::RenderBlueBullet(const BulletRenderData& bullet) {
    // Blue bullets render 3 geometric pieces: main body and two angled parts
    
    // Main body (center)
    RenderBulletGeometry(bullet, -0.07f, 0.0f, 0.0f, 0.15f);
    
    // First angled part
    RenderBulletGeometry(bullet, 0.03f, -0.06f, -60.0f, 0.2f);
    
    // Second angled part  
    RenderBulletGeometry(bullet, 0.03f, 0.06f, 60.0f, 0.2f);
}

void BulletRenderer::RenderStandardBullet(const BulletRenderData& bullet) {
    // Standard bullets render as a single geometric piece
    RenderBulletGeometry(bullet, -0.05f, 0.0f, 0.0f, 0.2f);
}

void BulletRenderer::RenderBulletGeometry(const BulletRenderData& bullet, float yOffset, float zOffset, float rotationX, float scaleZ) {
    Matrix4 model = Matrix4::Translation(bullet.position.x, bullet.position.y + yOffset,
                                          bullet.position.z) *
        Matrix4::Rotation(bullet.rotation.x, 1, 0, 0) *
        Matrix4::Rotation(-bullet.rotation.y, 0, 1, 0) *
        Matrix4::Rotation(bullet.rotation.z, 0, 0, 1) *
        Matrix4::Translation(0, 0, zOffset) *
        Matrix4::Rotation(rotationX, 1, 0, 0) *
        Matrix4::Scale(1, 1, scaleZ);
    if (App::GetSingleton().graphicsTask) {
        RenderContext::Current().Draw(App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::HorizontalOutline), model,
            bullet.primaryColor.r, bullet.primaryColor.g, bullet.primaryColor.b, 1.0f, false);
    }
    SetupBlendMode();
    float alpha = 0.1f + bullet.power /
        (bullet.type1 == TankType::TYPE_BLUE ? 500.0f : 1000.0f);
    if (App::GetSingleton().graphicsTask) {
        RenderContext::Current().Draw(App::GetSingleton().graphicsTask->Resources().GetGeometry(GeometryResource::HorizontalQuad), model,
            bullet.secondaryColor.r, bullet.secondaryColor.g, bullet.secondaryColor.b, alpha, false);
    }
    RestoreBlendMode();
}

void BulletRenderer::SetupBlendMode() {
    if (!blendEnabled) {
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        blendEnabled = true;
    }
}

void BulletRenderer::RestoreBlendMode() {
    if (blendEnabled) {
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        blendEnabled = false;
    }
}
