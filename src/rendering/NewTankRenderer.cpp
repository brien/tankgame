#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "NewTankRenderer.h"
#include "PlayerTankRenderer.h"
#include "../App.h"

NewTankRenderer::NewTankRenderer() {
    // Constructor - base class handles initialization
}

bool NewTankRenderer::Initialize() {
    if (!BaseRenderer::Initialize()) {
        return false;
    }
    // TODO: Initialize tank-specific resources here
    // - Load tank meshes if not using display lists
    // - Initialize shaders for tank rendering
    // - Set up any tank-specific OpenGL state
    return true;
}

void NewTankRenderer::Cleanup() {
    // TODO: Clean up tank-specific resources here
    // - Release tank meshes
    // - Clean up shaders
    // - Release any tank-specific OpenGL resources
    BaseRenderer::Cleanup();
}

void NewTankRenderer::Render(const TankRenderData& data) {
    if (!data.alive) {
        return;
    }
    
    if (data.isPlayer) {
        RenderPlayerTank(data);
    } else {
        RenderEnemyTank(data);
    }
}

void NewTankRenderer::RenderMultiple(const std::vector<TankRenderData>& tanks) {
    if (tanks.empty()) {
        return;
    }
    
    SetupRenderState();
    
    // Use default implementation but with proper state management
    ITankRenderer::RenderMultiple(tanks);
    
    CleanupRenderState();
}

void NewTankRenderer::RenderTanks(const std::vector<TankRenderData>& tanks) {
    // Legacy wrapper for backward compatibility
    RenderMultiple(tanks);
}

void NewTankRenderer::RenderTank(const TankRenderData& tank) {
    // Legacy wrapper for backward compatibility
    Render(tank);
}

void NewTankRenderer::SetupRenderState() {
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

void NewTankRenderer::CleanupRenderState() {
    // Restore all OpenGL state
    glPopAttrib();
    
    BaseRenderer::CleanupRenderState();
}

void NewTankRenderer::RenderTankBody(const TankRenderData& tank) {
    // Set secondary color for tank body
    SetTankColor(tank, false); // false = use secondary color (r2, g2, b2)
    
    // Apply health-based color modification
    float r = tank.r2, g = tank.g2, b = tank.b2;
    ApplyHealthColor(r, g, b, tank.health, tank.maxHealth);
    glColor3f(r, g, b);
    
    // Use the existing body display list from GraphicsTask
    App::GetSingleton().graphicsTask->bodylist.Call(0);
}

void NewTankRenderer::RenderTankTurret(const TankRenderData& tank) {
    // Set primary color for turret/cannon
    SetTankColor(tank, true); // true = use primary color (r, g, b)
    
    // Apply health-based color modification
    float r = tank.r, g = tank.g, b = tank.b;
    ApplyHealthColor(r, g, b, tank.health, tank.maxHealth);
    glColor3f(r, g, b);
    
    // Render cannon first
    App::GetSingleton().graphicsTask->cannonlist.Call(0);
    
    // Move for turret position and render turret
    glTranslatef(0.1f, 0, 0);
    App::GetSingleton().graphicsTask->turretlist.Call(0);
}

void NewTankRenderer::RenderTankEffects(const TankRenderData& tank) {
    // TODO: Implement special effects rendering
    // - Hit flash effects (using tank.hitAlpha)
    // - Bonus effects (using tank.bonus, tank.bonusTime)
    // - Motion trails, jump effects, etc.
    
    // For now, just implement hit flash
    if (tank.hitAlpha > 0.0f) {
        glPushMatrix();
        glTranslatef(tank.position.x, tank.position.y, tank.position.z);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, tank.hitAlpha);
        
        // Render a simple white flash effect
        glScalef(1.2f, 1.2f, 1.2f);
        App::GetSingleton().graphicsTask->bodylist.Call(0);
        
        glDisable(GL_BLEND);
        glPopMatrix();
    }
}

void NewTankRenderer::RenderTankHUD(const TankRenderData& tank) {
    // TODO: Implement HUD rendering for player tanks
    // - Health bars
    // - Charge bars  
    // - Targeting UI
    // - Bonus indicators
    
    // This would typically be rendered in screen space, not world space
    // For now, leave empty - HUD rendering might stay separate
}

void NewTankRenderer::SetTankColor(const TankRenderData& tank, bool isPrimary) {
    if (isPrimary) {
        glColor3f(tank.r, tank.g, tank.b);
    } else {
        glColor3f(tank.r2, tank.g2, tank.b2);
    }
}

void NewTankRenderer::ApplyHealthColor(float& r, float& g, float& b, float health, float maxHealth) {
    if (maxHealth > 0.0f && health > 0.0f) {
        // Apply health-based brightness (same logic as original TankRenderer)
        float healthFactor = maxHealth / health;
        r = (4.0f * r + healthFactor) / 2.0f;
        g = (4.0f * g + healthFactor) / 2.0f;
        b = (4.0f * b + healthFactor) / 2.0f;
        
        // Clamp to valid range
        r = (r > 1.0f) ? 1.0f : r;
        g = (g > 1.0f) ? 1.0f : g;
        b = (b > 1.0f) ? 1.0f : b;
    }
}

void NewTankRenderer::RenderPlayerTank(const TankRenderData& tank) {
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

void NewTankRenderer::RenderEnemyTank(const TankRenderData& tank) {
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
    DrawEnemyTankBody(tank.r2, tank.g2, tank.b2, tank.health, tank.maxHealth);
    
    // SetupBarrelTransform
    glRotatef(tank.turretRotation.x, 1, 0, 0);
    glRotatef(-tank.turretRotation.y, 0, 1, 0);
    glRotatef(tank.turretRotation.z, 0, 0, 1);
    
    // DrawBarrel with energy-based health coloring (primary colors)
    DrawEnemyTankBarrel(tank.r, tank.g, tank.b, tank.health, tank.maxHealth);
    
    // SetupTurretTransform
    glTranslatef(0.1f, 0, 0);
    
    // DrawTurret (uses current color from barrel)
    DrawEnemyTankTurret();
    
    glPopMatrix();
    
    // Restore state
    glPopAttrib();
}

void NewTankRenderer::RenderBasicTank(const TankRenderData& tank) {
    // Common tank rendering logic for both player and enemy tanks
    
    // Apply tank body transformation
    glTranslatef(tank.position.x, tank.position.y, tank.position.z);
    glRotatef(tank.bodyRotation.x, 1, 0, 0);
    glRotatef(-tank.bodyRotation.y, 0, 1, 0);
    glRotatef(tank.bodyRotation.z, 0, 0, 1);
    
    // Render tank body
    RenderTankBody(tank);
    
    // Apply turret transformation (relative to body)
    glRotatef(tank.turretRotation.x, 1, 0, 0);
    glRotatef(-tank.turretRotation.y, 0, 1, 0);
    glRotatef(tank.turretRotation.z, 0, 0, 1);
    
    // Render turret and cannon
    RenderTankTurret(tank);
}

void NewTankRenderer::RenderPlayerEffects(const TankRenderData& tank) {
    // Player tank effects based on PlayerTankRenderer::DrawPlayerEffects
    // Save state before effects rendering
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Setup effects render state (from PlayerTankRenderer::SetupPlayerEffectsRenderState)
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    // Apply texture drift for awesome effects (0.01f * drift from original)
    static float drift = 0;
    drift += 0.016f; // Approximate frame time for animation
    if (drift > 1000.0f) drift = 0;
    
    // Awesome effects are performed using the texture matrix stack
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glTranslatef(0.01f * drift, 0, 0); // TEXTURE_DRIFT_SPEED from original
    glMatrixMode(GL_MODELVIEW);
    
    // Bind effect texture
    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[17]);
    glColor3f(tank.r, tank.g, tank.b);
    
    glPushMatrix();
    
    // Render effect body (from PlayerTankRenderer::RenderEffectBody)
    glDisable(GL_LIGHTING);
    glTranslatef(tank.position.x, tank.position.y + 0.1f, tank.position.z); // TANK_HEIGHT_OFFSET
    glRotatef(tank.bodyRotation.x, 1, 0, 0);
    glRotatef(-tank.bodyRotation.y - 90, 0, 1, 0);
    glRotatef(tank.bodyRotation.z, 0, 0, 1);

    glFrontFace(GL_CCW);
    App::GetSingleton().graphicsTask->bodylist.Call(0);
    
    // Render effect turret (from PlayerTankRenderer::RenderEffectTurret)
    glTranslatef(0, 0.1f, 0); // TURRET_HEIGHT_OFFSET
    glRotatef(tank.turretRotation.x, 1, 0, 0);
    glRotatef(-tank.turretRotation.y + 180, 0, 1, 0);
    glRotatef(tank.turretRotation.z, 0, 0, 1);

    glColor3f(tank.r2, tank.g2, tank.b2);

    // Only render turret effect if charge is sufficient (SPECIAL_CHARGE_THRESHOLD_DIVISOR = 4.0f from original)
    if (tank.charge >= tank.fireCost / 4.0f) {
        App::GetSingleton().graphicsTask->turretlist.Call(0);
    }

    glFrontFace(GL_CW);
    glPopMatrix();
    
    // Restore texture matrix
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    // Restore all state
    glPopAttrib();
}

void NewTankRenderer::RenderPlayerTargetingUI(const TankRenderData& tank) {
    // Player targeting UI based on PlayerTankRenderer::DrawTargetingUI
    // Save state before targeting UI rendering
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Setup targeting UI render state (from PlayerTankRenderer::SetupTargetingUIRenderState)
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    
    glPushMatrix();
    glTranslatef(tank.position.x, tank.position.y + 0.25f, tank.position.z); // TARGETING_HEIGHT_OFFSET = 0.25f
    glRotatef(-tank.targetRotation, 0, 1, 0); // Use targetRotation (tank.rrl) for pointing to closest enemy

    // Render targeting indicator (from PlayerTankRenderer::RenderTargetingIndicator)
    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[20]);
    glColor4f(1.0f, 0.5f, 0.1f, 1.0); // Orange targeting color
    App::GetSingleton().graphicsTask->squarelist.Call(0);

    // Render ready indicator if tank can fire (from original logic)
    if (tank.charge >= tank.fireCost) {
        static float effectDrift = 0;
        effectDrift += 0.016f;
        if (effectDrift > 1000.0f) effectDrift = 0;

        glTranslatef(0, 0.018f, 0); // TARGETING_EFFECT_OFFSET = 0.018f
        glRotatef(100.0f * effectDrift, 0, 1, 0); // Spinning effect

        glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[17]);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glScalef(0.5f, 0.5f, 0.5f);
        App::GetSingleton().graphicsTask->squarelist.Call(0);
        glScalef(2.0f, 2.0f, 2.0f);
    }

    glPopMatrix();
    
    // Restore all state
    glPopAttrib();
}

void NewTankRenderer::DrawEnemyTankBody(float r, float g, float b, float energy, float maxEnergy) {
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

void NewTankRenderer::DrawEnemyTankBarrel(float r, float g, float b, float energy, float maxEnergy) {
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

void NewTankRenderer::DrawEnemyTankTurret() {
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
