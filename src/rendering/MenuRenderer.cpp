#include "MenuRenderer.h"
#include "../App.h"

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

MenuRenderer::MenuRenderer() {
}

bool MenuRenderer::Initialize() {
    // No special initialization needed for menu rendering
    return true;
}

void MenuRenderer::Cleanup() {
    // No special cleanup needed
}

void MenuRenderer::RenderMenu(const MenuRenderData& menuData) {
    if (!menuData.isVisible) {
        return;
    }
    
    SetupMenuRenderState();
    SetupMenuProjection();
    
    // Render menu background (optional)
    RenderMenuBackground(menuData);
    
    // Render all menu options
    for (int i = 0; i < menuData.numOptions; ++i) {
        RenderMenuOption(i, menuData);
    }
    
    RestoreGameProjection();
    CleanupMenuRenderState();
}

void MenuRenderer::SetupMenuProjection() {
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    // Treat 3D like 2D (matching original implementation)
    glTranslated(0, 0, -1);
}

void MenuRenderer::RestoreGameProjection() {
    glEnable(GL_DEPTH_TEST);
}

void MenuRenderer::SetupMenuRenderState() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
}

void MenuRenderer::CleanupMenuRenderState() {
    glPopMatrix();
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}

void MenuRenderer::RenderMenuBackground(const MenuRenderData& menuData) {
    // Optional: Render a semi-transparent background
    // For now, the original menu doesn't have a background, so this is empty
}

void MenuRenderer::RenderMenuOption(int optionIndex, const MenuRenderData& menuData) {
    if (optionIndex < 0 || optionIndex >= menuData.numOptions) {
        return;
    }
    
    bool isSelected = (optionIndex == menuData.selectedOption);
    
    if (isSelected) {
        RenderSelectedOption(optionIndex, menuData);
    } else {
        RenderNormalOption(optionIndex, menuData);
    }
}

void MenuRenderer::RenderSelectedOption(int optionIndex, const MenuRenderData& menuData) {
    float x, y, width, height;
    GetOptionPosition(optionIndex, x, y, width, height);
    
    // Render selection border (line loop) - matches original implementation
    glBegin(GL_LINE_LOOP);
    ApplyColor(Vector3(SELECTED_LINE_COLOR[0], SELECTED_LINE_COLOR[1], SELECTED_LINE_COLOR[2]));
    glVertex3f(x, y, 0);
    glVertex3f(x + width, y, 0);
    glVertex3f(x + width, y - height, 0);
    glVertex3f(x, y - height, 0);
    glEnd();
    
    // Render selection fill (semi-transparent quad)
    glBegin(GL_QUADS);
    ApplyColor(Vector3(SELECTED_FILL_COLOR[0], SELECTED_FILL_COLOR[1], SELECTED_FILL_COLOR[2]), 
               SELECTED_FILL_COLOR[3]);
    glVertex3f(x, y, 0);
    glVertex3f(x + width, y, 0);
    glVertex3f(x + width, y - height, 0);
    glVertex3f(x, y - height, 0);
    glEnd();
}

void MenuRenderer::RenderNormalOption(int optionIndex, const MenuRenderData& menuData) {
    // Normal options don't render anything in the original implementation
    // They are just "invisible" until selected
    // Could add subtle rendering here if desired
}

void MenuRenderer::GetOptionPosition(int optionIndex, float& x, float& y, float& width, float& height) {
    width = MENU_OPTION_WIDTH;
    height = MENU_OPTION_HEIGHT;
    
    // Calculate position based on original menu layout
    switch (optionIndex) {
        case 0: // First option (matches original option==0)
            x = MENU_START_X;
            y = MENU_START_Y;
            break;
        case 1: // Second option (matches original option==1)
            x = 0.03f; // Different X position for second option in original
            y = MENU_START_Y;
            break;
        case 2: // Third option (matches original option==2)
            x = 0.03f;
            y = MENU_START_Y - MENU_OPTION_SPACING; // -0.23f in original
            break;
        default:
            // For additional options, continue the pattern
            x = 0.03f;
            y = MENU_START_Y - (optionIndex - 1) * MENU_OPTION_SPACING;
            break;
    }
}

void MenuRenderer::ApplyColor(const Vector3& color, float alpha) {
    glColor4f(color.x, color.y, color.z, alpha);
}

void MenuRenderer::RenderOptionQuad(float x, float y, float width, float height, 
                                   const Vector3& color, float alpha, bool filled) {
    ApplyColor(color, alpha);
    
    if (filled) {
        glBegin(GL_QUADS);
    } else {
        glBegin(GL_LINE_LOOP);
    }
    
    glVertex3f(x, y, 0);
    glVertex3f(x + width, y, 0);
    glVertex3f(x + width, y - height, 0);
    glVertex3f(x, y - height, 0);
    
    glEnd();
}
