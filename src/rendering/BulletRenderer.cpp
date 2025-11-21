#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "BulletRenderer.h"
#include "RenderData.h"
#include "../App.h"
#include "../Logger.h"

BulletRenderer::BulletRenderer() : 
    BaseRenderer(),
    blendEnabled(false),
    texturesEnabled(false)
{
}

bool BulletRenderer::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }
    
    blendEnabled = false;
    texturesEnabled = false;
    
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
        SetupBulletRendering();
        
        // Render all bullets
        for (const auto& bullet : bullets) {
            if (bullet.type1 == TankType::TYPE_BLUE) {
                RenderBlueBullet(bullet);
            } else {
                RenderStandardBullet(bullet);
            }
        }
        
        CleanupBulletRendering();
        
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
    glPushMatrix();
    
    // Position the bullet
    glTranslatef(bullet.position.x, bullet.position.y + yOffset, bullet.position.z);
    
    // Apply rotations
    glRotatef(bullet.rotation.x, 1, 0, 0);
    glRotatef(-bullet.rotation.y, 0, 1, 0);
    glRotatef(bullet.rotation.z, 0, 0, 1);
    
    // Apply offset for multi-part bullets (blue type)
    if (zOffset != 0.0f) {
        glTranslatef(0, 0, zOffset);
    }
    
    // Apply additional rotation for angled parts
    if (rotationX != 0.0f) {
        glRotatef(rotationX, 1, 0, 0);
    }
    
    // Set primary color and render outline
    glColor3f(bullet.primaryColor.r, bullet.primaryColor.g, bullet.primaryColor.b);
    glScalef(1, 1, scaleZ);
    
    // Render the bullet outline using the square display list
    if (App::GetSingleton().graphicsTask) {
        App::GetSingleton().graphicsTask->squarelist2.Call(0);
    }
    
    // Render the glowing effect with secondary color
    SetupBlendMode();
    
    // Calculate alpha based on power (matching original logic)
    float alpha = 0.1f;
    if (bullet.type1 == TankType::TYPE_BLUE) {
        alpha += bullet.power / 500.0f;
    } else {
        alpha += bullet.power / 1000.0f;
    }
    
    glColor4f(bullet.secondaryColor.r, bullet.secondaryColor.g, bullet.secondaryColor.b, alpha);
    
    // Render the glowing inner part
    if (App::GetSingleton().graphicsTask) {
        App::GetSingleton().graphicsTask->squarelist.Call(0);
    }
    
    RestoreBlendMode();
    
    glPopMatrix();
}

void BulletRenderer::SetupBulletRendering() {
    // Disable textures for bullet rendering
    glDisable(GL_TEXTURE_2D);
    texturesEnabled = false;
}

void BulletRenderer::CleanupBulletRendering() {
    // Restore any global state if needed
    // The individual geometry rendering handles its own cleanup
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
