#include "HUDRenderer.h"
#include "RenderData.h"
#include "../App.h"
#include "../TextureHandler.h"

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <cstdio>
#include <cmath>

HUDRenderer::HUDRenderer() : texturesLoaded(false) {
    // Initialize texture array
    for (int i = 0; i < TEXTURE_COUNT; ++i) {
        hudTextures[i] = 0;
    }
}

bool HUDRenderer::Initialize() {
    bool success = true;
    
    // Load HUD-specific textures
    success &= LoadHUDTextures();
    
    return success;
}

void HUDRenderer::Cleanup() {
    CleanupHUDTextures();
}

void HUDRenderer::RenderPlayerHUD(const HUDRenderData& hudData) {
    SetupHUDRenderState();
    
    // Render targeting line in 3D space
    if (hudData.showTargeting) {
        RenderTargetingLine(hudData);
    }
    
    // Switch to 2D rendering for HUD elements
    SetupHUDProjection();
    
    // Render HUD bars and meters
    if (hudData.showHealthBar) {
        RenderHealthBar(hudData);
    }
    
    if (hudData.showEnergyBar) {
        RenderEnergyBar(hudData);
    }
    
    RenderChargeBar(hudData);
    
    if (hudData.showComboMeter) {
        RenderComboMeter(hudData);
    }
    
    if (hudData.showSpecialMeter) {
        RenderSpecialMeter(hudData);
    }
    
    if (hudData.showFPSCounter) {
        RenderFPSCounter(hudData);
    }
    
    // Render debug information if enabled
    if (hudData.showDebugInfo) {
        // Debug meter visualization
        glBegin(GL_QUADS);
        float debugValue = 2.0f * hudData.deltaTime;
        ApplyColor(Vector3(1.0f, debugValue, 1.0f));
        glVertex3f(0.50f, -0.30f, 0);
        glVertex3f(0.50f, -0.30f + debugValue, 0);
        glVertex3f(0.50f + 0.01f, -0.30f + debugValue, 0);
        glVertex3f(0.50f + 0.01f, -0.30f, 0);
        glEnd();
    }
    
    RestoreGameProjection();
    CleanupHUDRenderState();
}

void HUDRenderer::RenderAllPlayerHUDs(const std::vector<HUDRenderData>& playerHUDs) {
    for (const auto& hudData : playerHUDs) {
        RenderPlayerHUD(hudData);
    }
}

void HUDRenderer::RenderCompleteUI(const UIRenderData& uiData) {
    // Render player HUDs
    RenderAllPlayerHUDs(uiData.playerHUDs);
    
    // Render menu if visible
    if (uiData.menu.isVisible) {
        RenderMenu(uiData.menu);
    }
    
    // Render debug information
    if (uiData.debug.showDebugInfo) {
        RenderDebugInfo(uiData.debug);
    }
}

void HUDRenderer::RenderMenu(const MenuRenderData& menuData) {
    if (!menuData.isVisible) {
        return;
    }
    
    SetupHUDProjection();
    SetupHUDRenderState();
    
    RenderMenuBackground(menuData);
    RenderMenuOptions(menuData);
    
    CleanupHUDRenderState();
    RestoreGameProjection();
}

void HUDRenderer::RenderDebugInfo(const DebugRenderData& debugData) {
    if (!debugData.showDebugInfo) {
        return;
    }
    
    SetupHUDProjection();
    SetupTextRenderState();
    
    // Render performance stats
    if (debugData.showPerformanceStats) {
        char buffer[64];
        sprintf(buffer, "FPS: %.1f", debugData.averageFPS);
        RenderHUDText(buffer, -0.9f, 0.9f, debugData.performanceColor);
        
        sprintf(buffer, "Frame: %.2fms", debugData.frameTime);
        RenderHUDText(buffer, -0.9f, 0.85f, debugData.performanceColor);
    }
    
    // Render render stats
    if (debugData.showRenderStats) {
        char buffer[64];
        sprintf(buffer, "Calls: %d", debugData.renderCallCount);
        RenderHUDText(buffer, -0.9f, 0.8f, debugData.debugTextColor);
        
        sprintf(buffer, "Tris: %d", debugData.triangleCount);
        RenderHUDText(buffer, -0.9f, 0.75f, debugData.debugTextColor);
    }
    
    CleanupTextRenderState();
    RestoreGameProjection();
}

void HUDRenderer::RenderTargetingLine(const HUDRenderData& hudData) {
    glPushMatrix();
    
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    
    // Set targeting line color
    ApplyColor(hudData.targetingColor);
    
    // Setup stippled line animation
    SetupStippledLine(hudData.targetingAnimationTime);
    
    glBegin(GL_LINE_LOOP);
    
    // Draw targeting line from player position
    glVertex3f(hudData.playerPosition.x, hudData.playerPosition.y + 0.1f, hudData.playerPosition.z);
    
    // Calculate target point
    float angle = hudData.playerRotation.y * (M_PI / 180.0f);
    float distance = 32.0f;
    float targetX = hudData.playerPosition.x + distance * cos(angle);
    float targetZ = hudData.playerPosition.z + distance * sin(angle);
    
    ApplyColor(Vector3(0.0f, 0.0f, 0.0f)); // Black end point
    glVertex3f(targetX, hudData.playerPosition.y + 0.1f, targetZ);
    
    glEnd();
    
    CleanupStippledLine();
    glPopMatrix();
}

void HUDRenderer::RenderHealthBar(const HUDRenderData& hudData) {
    // Health bar background
    RenderQuad(-0.51f, 0.34f, 0.29f, 0.03f, Vector3(0.2f, 0.2f, 0.2f));
    
    // Health bar fill
    float healthPercent = hudData.health / hudData.maxHealth;
    if (healthPercent > 1.0f) healthPercent = 1.0f;
    
    RenderQuad(-0.51f, 0.34f, 0.29f * healthPercent, 0.03f, hudData.healthColor);
    
    // Health bar border
    glBegin(GL_LINE_LOOP);
    ApplyColor(Vector3(1.0f, 0.8f, 0.8f));
    glVertex3f(-0.51f, 0.37f, 0);
    glVertex3f(-0.51f + 0.29f, 0.37f, 0);
    glVertex3f(-0.51f + 0.29f, 0.34f, 0);
    glVertex3f(-0.51f, 0.34f, 0);
    glEnd();
    
    // Health icon
    if (texturesLoaded) {
        RenderTexturedQuad(-0.55f, 0.34f, 0.03f, 0.03f, 
                          hudTextures[TEXTURE_HEALTH_ICON], Vector3(1.0f, 0.6f, 0.6f));
    }
}

void HUDRenderer::RenderEnergyBar(const HUDRenderData& hudData) {
    // Energy bar background
    RenderQuad(-0.51f, 0.29f, 0.29f, 0.03f, Vector3(0.2f, 0.2f, 0.2f));
    
    // Energy bar fill
    float energyPercent = hudData.energy / hudData.maxEnergy;
    if (energyPercent > 1.0f) energyPercent = 1.0f;
    
    RenderQuad(-0.51f, 0.29f, 0.29f * energyPercent, 0.03f, hudData.energyColor);
    
    // Energy bar border
    glBegin(GL_LINE_LOOP);
    ApplyColor(Vector3(0.5f, 1.0f, 1.0f));
    glVertex3f(-0.51f, 0.32f, 0);
    glVertex3f(-0.51f + 0.29f, 0.32f, 0);
    glVertex3f(-0.51f + 0.29f, 0.29f, 0);
    glVertex3f(-0.51f, 0.29f, 0);
    glEnd();
    
    // Energy icon
    if (texturesLoaded) {
        RenderTexturedQuad(-0.55f, 0.29f, 0.03f, 0.03f, 
                          hudTextures[TEXTURE_ENERGY_ICON], Vector3(0.6f, 0.6f, 1.0f));
    }
}

void HUDRenderer::RenderChargeBar(const HUDRenderData& hudData) {
    // Charge bar fill
    float chargePercent = hudData.charge / hudData.maxCharge;
    if (chargePercent > 1.0f) chargePercent = 1.0f;
    
    // Enable blending if can't fire
    if (!hudData.canFire) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    glBegin(GL_QUADS);
    ApplyColor(hudData.chargeColor, hudData.canFire ? 1.0f : 0.5f);
    glVertex3f(-0.51f, 0.32f, 0);
    glVertex3f(-0.51f + 0.29f * chargePercent, 0.32f, 0);
    glVertex3f(-0.51f + 0.29f * chargePercent, 0.29f, 0);
    glVertex3f(-0.51f, 0.29f, 0);
    glEnd();
    
    // Fire cost indicator line
    glBegin(GL_LINES);
    ApplyColor(Vector3(0.5f, 1.0f, 1.0f));
    float costPosition = -0.51f + 0.29f * (hudData.fireCost / hudData.maxCharge);
    glVertex3f(costPosition, 0.32f, 0);
    glVertex3f(costPosition, 0.29f, 0);
    glEnd();
    
    if (!hudData.canFire) {
        glDisable(GL_BLEND);
    }
}

void HUDRenderer::RenderComboMeter(const HUDRenderData& hudData) {
    float comboPercent = hudData.combo / 100.0f;
    
    glBegin(GL_QUADS);
    ApplyColor(hudData.comboColor);
    glVertex3f(0.50f, -0.37f, 0);
    glVertex3f(0.50f, -0.37f + comboPercent, 0);
    glVertex3f(0.50f + 0.01f, -0.37f + comboPercent, 0);
    glVertex3f(0.50f + 0.01f, -0.37f, 0);
    glEnd();
}

void HUDRenderer::RenderSpecialMeter(const HUDRenderData& hudData) {
    float specialPercent = hudData.special / 100.0f;
    
    // Enable blending if special not available
    if (!hudData.hasSpecialAvailable) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    glBegin(GL_QUADS);
    ApplyColor(hudData.specialColor, hudData.hasSpecialAvailable ? 1.0f : 0.5f);
    glVertex3f(0.52f, -0.37f, 0);
    glVertex3f(0.52f, -0.37f + specialPercent, 0);
    glVertex3f(0.52f + 0.01f, -0.37f + specialPercent, 0);
    glVertex3f(0.52f + 0.01f, -0.37f, 0);
    glEnd();
    
    // Special ability cost indicators
    float costPercentage = hudData.fireCost / 500.0f;
    int numIndicators = static_cast<int>(specialPercent / costPercentage);
    
    ApplyColor(Vector3(1.0f, 1.0f, 1.0f));
    glBegin(GL_LINES);
    for (int i = 1; i <= numIndicators; ++i) {
        float y = -0.37f + costPercentage * i;
        glVertex3f(0.52f, y, 0);
        glVertex3f(0.52f + 0.01f, y, 0);
    }
    
    // Base line
    glVertex3f(0.52f, -0.37f, 0);
    glVertex3f(0.52f + 0.01f, -0.37f, 0);
    glEnd();
    
    if (!hudData.hasSpecialAvailable) {
        glDisable(GL_BLEND);
    }
}

void HUDRenderer::RenderFPSCounter(const HUDRenderData& hudData) {
    char buffer[32];
    sprintf(buffer, "FPS: %.2f", hudData.currentFPS);
    
    // For now, just position it somewhere visible
    // TODO: Implement proper text rendering when TextRenderer is available
    // RenderHUDText(buffer, 0.6f, 0.9f, Vector3(1.0f, 1.0f, 1.0f));
}

void HUDRenderer::RenderMenuBackground(const MenuRenderData& menuData) {
    // Simple semi-transparent background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBegin(GL_QUADS);
    ApplyColor(menuData.backgroundColor, menuData.fadeAlpha * 0.8f);
    glVertex3f(-1.0f, -1.0f, 0);
    glVertex3f(1.0f, -1.0f, 0);
    glVertex3f(1.0f, 1.0f, 0);
    glVertex3f(-1.0f, 1.0f, 0);
    glEnd();
    
    glDisable(GL_BLEND);
}

void HUDRenderer::RenderMenuOptions(const MenuRenderData& menuData) {
    float startY = 0.3f;
    float spacing = 0.1f;
    
    for (int i = 0; i < menuData.numOptions; ++i) {
        bool isSelected = (i == menuData.selectedOption);
        Vector3 color = isSelected ? menuData.selectedColor : menuData.normalColor;
        
        if (i < static_cast<int>(menuData.optionEnabled.size()) && !menuData.optionEnabled[i]) {
            color = menuData.disabledColor;
        }
        
        float y = startY - i * spacing;
        RenderMenuOption(menuData.menuOptions[i], 0.0f, y, isSelected, color);
    }
}

void HUDRenderer::RenderMenuOption(const std::string& text, float x, float y, 
                                  bool selected, const Vector3& color) {
    // TODO: Implement proper text rendering when TextRenderer is available
    // For now, just render a colored quad to indicate the option
    if (selected) {
        glBegin(GL_QUADS);
        ApplyColor(color);
        glVertex3f(x - 0.2f, y - 0.02f, 0);
        glVertex3f(x + 0.2f, y - 0.02f, 0);
        glVertex3f(x + 0.2f, y + 0.02f, 0);
        glVertex3f(x - 0.2f, y + 0.02f, 0);
        glEnd();
    }
}

void HUDRenderer::SetupHUDProjection() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslated(0, 0, -1);
    
    glDisable(GL_DEPTH_TEST);
}

void HUDRenderer::RestoreGameProjection() {
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void HUDRenderer::SetupHUDRenderState() {
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
}

void HUDRenderer::CleanupHUDRenderState() {
    glDisable(GL_BLEND);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
}

void HUDRenderer::SetupTextRenderState() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void HUDRenderer::CleanupTextRenderState() {
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void HUDRenderer::ApplyColor(const Vector3& color, float alpha) {
    glColor4f(color.x, color.y, color.z, alpha);
}

void HUDRenderer::SetupStippledLine(float animationTime) {
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(16, static_cast<int>(animationTime));
}

void HUDRenderer::CleanupStippledLine() {
    glDisable(GL_LINE_STIPPLE);
}

void HUDRenderer::RenderQuad(float x, float y, float width, float height, const Vector3& color) {
    glBegin(GL_QUADS);
    ApplyColor(color);
    glVertex3f(x, y, 0);
    glVertex3f(x + width, y, 0);
    glVertex3f(x + width, y + height, 0);
    glVertex3f(x, y + height, 0);
    glEnd();
}

void HUDRenderer::RenderTexturedQuad(float x, float y, float width, float height, 
                                    unsigned int textureId, const Vector3& color) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    glBegin(GL_QUADS);
    ApplyColor(color);
    glTexCoord2f(0, 1);
    glVertex3f(x, y + height, 0);
    glTexCoord2f(1, 1);
    glVertex3f(x + width, y + height, 0);
    glTexCoord2f(1, 0);
    glVertex3f(x + width, y, 0);
    glTexCoord2f(0, 0);
    glVertex3f(x, y, 0);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
}

void HUDRenderer::RenderHUDText(const char* text, float x, float y, const Vector3& color) {
    // TODO: Implement when TextRenderer is available in Step 22
    // For now, this is a placeholder
}

bool HUDRenderer::LoadHUDTextures() {
    // Get texture handler from App
    if (!App::GetSingleton().graphicsTask) {
        return false;
    }
    
    auto* textureArray = App::GetSingleton().graphicsTask->textureHandler.GetTextureArray();
    
    // Map HUD textures to existing game textures
    hudTextures[TEXTURE_HEALTH_ICON] = textureArray[TEXTURE_HEART];
    hudTextures[TEXTURE_ENERGY_ICON] = textureArray[TEXTURE_BANG];
    // TODO: Load additional HUD-specific textures as needed
    
    texturesLoaded = true;
    return true;
}

void HUDRenderer::CleanupHUDTextures() {
    // HUD textures are managed by TextureHandler, so no cleanup needed
    texturesLoaded = false;
}
