#include "EnemyTankRendererImpl.h"

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

EnemyTankRendererImpl::EnemyTankRendererImpl() {
}

bool EnemyTankRendererImpl::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }
    // TODO: Initialize enemy-specific resources
    // - Optimize for batch rendering
    // - Set up efficient geometry rendering
    return true;
}

void EnemyTankRendererImpl::Cleanup() {
    // TODO: Clean up enemy-specific resources
    BaseRenderer::Cleanup();
}

void EnemyTankRendererImpl::Render(const TankRenderData& data) {
    if (!data.alive || data.isPlayer) {
        return;
    }
    
    // Save matrix state
    glPushMatrix();
    
    // Render enemy tank using hardcoded geometry for performance
    RenderEnemyTankGeometry(data);
    
    // Restore matrix state
    glPopMatrix();
}

void EnemyTankRendererImpl::SetupRenderState() {
    BaseRenderer::SetupRenderState();
    
    // Enemy tank render state (optimized for performance)
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CW);
}

void EnemyTankRendererImpl::CleanupRenderState() {
    // Restore OpenGL state
    glPopAttrib();
    BaseRenderer::CleanupRenderState();
}

void EnemyTankRendererImpl::RenderEnemyTankGeometry(const TankRenderData& tank) {
    // Replicate TankRenderer::Draw() logic exactly for compatibility
    
    // Setup body transform
    SetupBodyTransform(tank);
    
    // Draw body with secondary colors and health-based modification
    DrawBody(tank.secondaryColor.r, tank.secondaryColor.g, tank.secondaryColor.b, tank.health, tank.maxHealth);
    
    // Setup barrel transform
    SetupBarrelTransform(tank);
    
    // Draw barrel with primary colors and health-based modification
    DrawBarrel(tank.primaryColor.r, tank.primaryColor.g, tank.primaryColor.b, tank.health, tank.maxHealth);
    
    // Setup turret transform
    SetupTurretTransform();
    
    // Draw turret (uses current color from barrel)
    DrawTurret();
}

void EnemyTankRendererImpl::SetupBodyTransform(const TankRenderData& tank) {
    glTranslatef(tank.position.x, tank.position.y, tank.position.z);
    glRotatef(tank.bodyRotation.x, 1, 0, 0);
    glRotatef(-tank.bodyRotation.y, 0, 1, 0);
    glRotatef(tank.bodyRotation.z, 0, 0, 1);
}

void EnemyTankRendererImpl::SetupBarrelTransform(const TankRenderData& tank) {
    glRotatef(tank.turretRotation.x, 1, 0, 0);
    glRotatef(-tank.turretRotation.y, 0, 1, 0);
    glRotatef(tank.turretRotation.z, 0, 0, 1);
}

void EnemyTankRendererImpl::SetupTurretTransform() {
    glTranslatef(0.1f, 0, 0);
}

void EnemyTankRendererImpl::DrawBody(float r, float g, float b, float energy, float maxEnergy) {
    // Exact copy from TankRenderer::DrawBody for consistency
    if (maxEnergy > 0.0f && energy > 0.0f) {
        float healthFactor = maxEnergy / energy;
        glColor3f((4 * r + healthFactor) / 2, (4 * g + healthFactor) / 2, (4 * b + healthFactor) / 2);
    } else {
        glColor3f(r, g, b);
    }
    
    glScalef(.06f, .06f, .06f);
    glBegin(GL_TRIANGLES);
    
    // Tank body geometry (exact copy from TankRenderer.cpp)
    glNormal3f(0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    
    glNormal3f(0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    
    glNormal3f(-0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    
    glNormal3f(-0.894427f, -0.447214f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    
    glNormal3f(0.182574f, 0.365148f, -0.912871f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    
    glNormal3f(0.000000f, -0.447214f, -0.894427f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    
    glNormal3f(0.000000f, -0.447214f, 0.894427f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    
    glNormal3f(0.182574f, 0.365148f, 0.912871f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 2, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 2, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-5, 2, 5);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-5, 2, -5);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-4, 0, -4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(4, 0, -4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-4, 0, 4);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(4, 0, 4);
    
    glEnd();
    glScalef(1.0f/0.06f, 1.0f/0.06f, 1.0f/0.06f); // Restore scale
}

void EnemyTankRendererImpl::DrawBarrel(float r, float g, float b, float energy, float maxEnergy) {
    // Exact copy from TankRenderer::DrawBarrel for consistency
    if (maxEnergy > 0.0f && energy > 0.0f) {
        float healthFactor = maxEnergy / energy;
        glColor3f((4 * r + healthFactor) / 2, (4 * g + healthFactor) / 2, (4 * b + healthFactor) / 2);
    } else {
        glColor3f(r, g, b);
    }
    
    glScalef(.1f, .1f, .1f);
    glBegin(GL_TRIANGLES);
    
    // Tank barrel geometry (exact copy from TankRenderer.cpp)
    glNormal3f(0.163846f, -0.081923f, 0.983078f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    
    glNormal3f(0.180156f, -0.041001f, 0.982783f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    
    glNormal3f(0.196116f, 0.000000f, -0.980581f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    
    glNormal3f(0.180156f, -0.041001f, -0.982783f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    
    glNormal3f(1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    
    glNormal3f(1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    
    glNormal3f(-0.894427f, 0.447214f, 0.000000f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    
    glNormal3f(-0.894427f, 0.447214f, 0.000000f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, 3);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, 2);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-2, 4, -3);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(3, 4, -2);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, -2);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(3, 2, 2);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, -3);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(-3, 2, 3);
    
    glEnd();
    glScalef(1.0f/0.1f, 1.0f/0.1f, 1.0f/0.1f); // Restore scale
}

void EnemyTankRendererImpl::DrawTurret() {
    // Exact copy from TankRenderer::DrawTurret for consistency
    glScalef(.1f, .1f, .1f);
    glBegin(GL_TRIANGLES);
    
    // Tank turret geometry (exact copy from TankRenderer.cpp)
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(0.000000f, -0.707107f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3f(5, 4, -0.25f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3f(5, 4, 0.25f);
    
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3f(5, 4, 0.25f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, 0.25f);
    
    glNormal3f(0.000000f, -0.707107f, -0.707107f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(-3, 3, 0);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 3, 0);
    
    glNormal3f(0.000000f, -0.707107f, -0.707107f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3i(5, 3, 0);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(5, 4, -0.25f);
    
    glNormal3f(-1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3f(-3, 4, -0.25f);
    glTexCoord2f(0.000000f, 0.000000f);
    glVertex3f(-3, 4, 0.25f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(-3, 3, 0);
    
    glNormal3f(1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3f(5, 4, -0.25f);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3i(5, 3, 0);
    glTexCoord2f(1.000000f, 0.000000f);
    glVertex3f(5, 4, 0.25f);
    
    glNormal3f(0.000000f, -0.707107f, 0.707107f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, 0.25f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3f(5, 4, 0.25f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(5, 3, 0);
    
    glNormal3f(0.000000f, -0.707107f, 0.707107f);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3f(-3, 4, 0.25f);
    glTexCoord2f(0.000000f, 1.000000f);
    glVertex3i(5, 3, 0);
    glTexCoord2f(1.000000f, 1.000000f);
    glVertex3i(-3, 3, 0);
    
    glEnd();
    glScalef(1.0f/0.1f, 1.0f/0.1f, 1.0f/0.1f); // Restore scale
}
