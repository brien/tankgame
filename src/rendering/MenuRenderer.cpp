#include "MenuRenderer.h"

#include "ResourceManager.h"
#include "RenderContext.h"
#include "GLFunctions.h"
#include "TextRenderer.h"

MenuRenderer::MenuRenderer(ResourceManager* resources) : resources(resources) {}

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

    RenderModernMenu(menuData);
    if (resources) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        TextRenderer::Render(*resources, TextRenderer::MenuLines(menuData, viewport[2], viewport[3]));
    }
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

void MenuRenderer::RenderModernMenu(const MenuRenderData& menuData) {
    if (!resources || menuData.selectedOption < 0 || menuData.selectedOption >= menuData.numOptions) return;
    const GLboolean depth = glIsEnabled(GL_DEPTH_TEST), blend = glIsEnabled(GL_BLEND);
    const GLboolean cull = glIsEnabled(GL_CULL_FACE);
    GLboolean depthWrite;
    GLint srcRGB, dstRGB, srcAlpha, dstAlpha, equationRGB, equationAlpha;
    GLfloat lineWidth;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
    glGetFloatv(GL_LINE_WIDTH, &lineWidth);
    glGetIntegerv(GL_BLEND_SRC_RGB, &srcRGB); glGetIntegerv(GL_BLEND_DST_RGB, &dstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha); glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &equationRGB); glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &equationAlpha);
    glDisable(GL_DEPTH_TEST); glDepthMask(GL_FALSE); glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE);
    GLFunctions::BlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glLineWidth(1.0f);

    // The reference menu is a camera-independent plane at z=-1 using the
    // viewport's perspective projection. Copy the context so world state survives.
    RenderContext ui = RenderContext::Current();
    ui.SetView(Matrix4::Identity());
    float x, y, width, height;
    GetOptionPosition(menuData.selectedOption, x, y, width, height);
    const Matrix4 model = Matrix4::Translation(x, y, -1) * Matrix4::Scale(width, -height, 1);
    ui.Draw(resources->GetGeometry(GeometryResource::UIOutline), model, 0, 1, 1, 1, false);
    // ONE/ONE deliberately ignores alpha, matching the reference's additive fill.
    ui.Draw(resources->GetGeometry(GeometryResource::UIQuad), model, 0, .4f, .4f, .1f, false);

    GLFunctions::BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    GLFunctions::BlendEquationSeparate(equationRGB, equationAlpha);
    glLineWidth(lineWidth); glDepthMask(depthWrite);
    if (depth) glEnable(GL_DEPTH_TEST);
    if (!blend) glDisable(GL_BLEND);
    if (cull) glEnable(GL_CULL_FACE);
}
