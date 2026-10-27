#include "HUDRenderer.h"
#include "HUDLayout.h"
#include "HUDMeterRenderer.h"
#include "TargetingLineRenderer.h"
#include "TextRenderer.h"
#include "ResourceManager.h"
#include "RendererMode.h"
#include "GLFunctions.h"
#include <cmath>

HUDRenderer::HUDRenderer(ResourceManager& resources) : resources(resources) {}

bool HUDRenderer::Initialize() { return true; }
void HUDRenderer::Cleanup() {}

void HUDRenderer::RenderPlayerHUD(const HUDRenderData& data) {
    if (RendererMode::IsModern()) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        TargetingLineRenderer::Render(resources, data);
        HUDMeterRenderer::Render(resources, data);
        TextRenderer::Render(resources, TextRenderer::PlayerLines(data, viewport[2]));
        return;
    }
#ifndef __EMSCRIPTEN__
    // Keep the native targeting reference while sharing all meter descriptions.
    // Preserve the caller's fixed-function state and both matrix stacks.
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    GLint matrixMode;
    glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLFunctions::BlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glLineWidth(1);
    GLFunctions::ActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    if (data.showTargeting) RenderTargetingLine(data);
    glDepthMask(GL_FALSE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, 0, -1);
    glDisable(GL_DEPTH_TEST);
    for (const auto& draw : HUDLayout::Build(data)) {
        if (draw.blend == HUDLayout::Blend::Opaque) glDisable(GL_BLEND);
        else {
            glEnable(GL_BLEND);
            if (draw.blend == HUDLayout::Blend::Additive) glBlendFunc(GL_ONE, GL_ONE);
            else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (draw.icon != HUDLayout::Icon::None) {
            glEnable(GL_TEXTURE_2D);
            const auto* textures = resources.GetTextureHandler().GetTextureArray();
            glBindTexture(GL_TEXTURE_2D, textures[draw.icon == HUDLayout::Icon::Health ? TEXTURE_HEART : TEXTURE_BANG]);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        } else glDisable(GL_TEXTURE_2D);
        glColor4f(draw.color.x, draw.color.y, draw.color.z, draw.alpha);
        glBegin(draw.shape == HUDLayout::Shape::Line ? GL_LINES :
                draw.shape == HUDLayout::Shape::Outline ? GL_LINE_LOOP : GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(draw.x, draw.y, 0);
        if (draw.shape != HUDLayout::Shape::Line) {
            glTexCoord2f(1, 0); glVertex3f(draw.x + draw.width, draw.y, 0);
        }
        glTexCoord2f(1, 1); glVertex3f(draw.x + draw.width, draw.y + draw.height, 0);
        if (draw.shape != HUDLayout::Shape::Line) {
            glTexCoord2f(0, 1); glVertex3f(draw.x, draw.y + draw.height, 0);
        }
        glEnd();
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(matrixMode);
    glPopAttrib();
#endif
}

void HUDRenderer::RenderDebugInfo(const DebugRenderData& data) {
    // Compatibility diagnostic text was a placeholder; preserve that policy.
    if (RendererMode::IsModern()) TextRenderer::Render(resources, TextRenderer::DebugLines(data));
}

#ifndef __EMSCRIPTEN__
void HUDRenderer::RenderTargetingLine(const HUDRenderData& hudData) {
    glPushMatrix();
    
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    
    // Set targeting line color
    glColor3f(hudData.targetingColor.x, hudData.targetingColor.y, hudData.targetingColor.z);
    
    // Setup stippled line animation
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(16, static_cast<int>(hudData.targetingAnimationTime));
    
    glBegin(GL_LINE_LOOP);
    
    // Draw targeting line from player position
    glVertex3f(hudData.playerPosition.x, hudData.playerPosition.y + 0.1f, hudData.playerPosition.z);
    
    // Calculate target point
    float angle = hudData.playerRotation.y * (M_PI / 180.0f);
    float distance = 32.0f;
    float targetX = hudData.playerPosition.x + distance * cos(angle);
    float targetZ = hudData.playerPosition.z + distance * sin(angle);
    
    glColor3f(0, 0, 0); // Black end point
    glVertex3f(targetX, hudData.playerPosition.y + 0.1f, targetZ);
    
    glEnd();
    
    glDisable(GL_LINE_STIPPLE);
    glPopMatrix();
}

#endif
