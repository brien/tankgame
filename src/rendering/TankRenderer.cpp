#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "TankRenderer.h"
#include "PlayerTankRenderer.h"
#include "../App.h"

TankRenderer::TankRenderer() {
    // Constructor - base class handles initialization
}

bool TankRenderer::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }
    // TODO: Initialize tank-specific resources here
    // - Load tank meshes if not using display lists
    // - Initialize shaders for tank rendering
    // - Set up any tank-specific OpenGL state
    return true;
}

void TankRenderer::Cleanup() {
    // TODO: Clean up tank-specific resources here
    // - Release tank meshes
    // - Clean up shaders
    // - Release any tank-specific OpenGL resources
    BaseRenderer::Cleanup();
}

void TankRenderer::Render(const TankRenderData& data) {
    if (!data.alive) {
        return;
    }
    
    if (data.isPlayer) {
        RenderPlayerTank(data);
    } else {
        RenderEnemyTank(data);
    }
}

void TankRenderer::RenderMultiple(const std::vector<TankRenderData>& tanks) {
    if (tanks.empty()) {
        return;
    }
    
    SetupRenderState();
    
    // Use default implementation but with proper state management
    ITankRenderer::RenderMultiple(tanks);
    
    CleanupRenderState();
}

void TankRenderer::SetupRenderState() {
    BaseRenderer::SetupRenderState();
    
    // Save current OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Set up OpenGL state for tank rendering
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glFrontFace(GL_CW);
}

void TankRenderer::CleanupRenderState() {
    // Restore all OpenGL state
    glPopAttrib();
    
    BaseRenderer::CleanupRenderState();
}

void TankRenderer::RenderPlayerTank(const TankRenderData& tank) {
    // Calculate drift for animations (static to maintain continuity)
    static float drift = 0;
    drift += 0.016f; // Approximate 60 FPS delta time
    if (drift > 1.0f) drift = 0;
    
    // Delegate to PlayerTankRenderer for all player tank rendering
    PlayerTankRenderer::RenderPlayerTank(tank, drift);

    PlayerTankRenderer::RenderPlayerEffects(tank, drift);
    
    // Render targeting UI (assume enemies exist for now - could be improved)
    bool hasEnemyTargets = true; // TODO: Pass this information from game logic

    PlayerTankRenderer::RenderTargetingUI(tank, drift, hasEnemyTargets);
}

void TankRenderer::RenderEnemyTank(const TankRenderData& tank) {
    // Enemy tanks use TankRenderer::Draw logic (hardcoded geometry)
    // Save state before enemy tank rendering
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Setup enemy tank render state
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CW);
    
    glPushMatrix();
    
    // Replicate TankRenderer::Draw() logic exactly
    // SetupBodyTransform
    glTranslatef(tank.position.x, tank.position.y, tank.position.z);
    glRotatef(tank.bodyRotation.x, 1, 0, 0);
    glRotatef(-tank.bodyRotation.y, 0, 1, 0);
    glRotatef(tank.bodyRotation.z, 0, 0, 1);
    
    // DrawBody with energy-based health coloring (secondary colors)
    DrawEnemyTankBody(tank.secondaryColor.r, tank.secondaryColor.g, tank.secondaryColor.b, tank.health, tank.maxHealth);
    
    // SetupBarrelTransform
    glRotatef(tank.turretRotation.x, 1, 0, 0);
    glRotatef(-tank.turretRotation.y, 0, 1, 0);
    glRotatef(tank.turretRotation.z, 0, 0, 1);
    
    // DrawBarrel with energy-based health coloring (primary colors)
    DrawEnemyTankBarrel(tank.primaryColor.r, tank.primaryColor.g, tank.primaryColor.b, tank.health, tank.maxHealth);
    
    // SetupTurretTransform
    glTranslatef(0.1f, 0, 0);
    
    // DrawTurret (uses current color from barrel)
    DrawEnemyTankTurret();
    
    glPopMatrix();
    
    // Restore state
    glPopAttrib();
}

void TankRenderer::DrawEnemyTankBody(float r, float g, float b, float energy, float maxEnergy) {
    // Exact copy from TankRenderer::DrawBody
    glColor3f((4 * r + (maxEnergy / energy)) / 2, (4 * g + (maxEnergy / energy)) / 2, (4 * b + (maxEnergy / energy)) / 2);
    
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
    glScalef(10, 10, 10);
}

void TankRenderer::DrawEnemyTankBarrel(float r, float g, float b, float energy, float maxEnergy) {
    // Exact copy from TankRenderer::DrawBarrel
    glColor3f((4 * r + (maxEnergy / energy)) / 2, (4 * g + (maxEnergy / energy)) / 2, (4 * b + (maxEnergy / energy)) / 2);
    
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
    glScalef(10, 10, 10);
}

void TankRenderer::DrawEnemyTankTurret() {
    // Exact copy from TankRenderer::DrawTurret
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
    glScalef(10, 10, 10);
}
