#include "HUDMeterRenderer.h"
#include "HUDLayout.h"
#include "ResourceManager.h"
#include "RenderContext.h"
#include "GLFunctions.h"

void HUDMeterRenderer::Render(ResourceManager& resources, const HUDRenderData& data) {
    const auto draws = HUDLayout::Build(data);
    if (draws.empty()) return;
    const GLboolean depth = glIsEnabled(GL_DEPTH_TEST), blend = glIsEnabled(GL_BLEND);
    const GLboolean cull = glIsEnabled(GL_CULL_FACE);
    GLboolean depthWrite;
    GLint srcRGB, dstRGB, srcAlpha, dstAlpha, equationRGB, equationAlpha, activeTexture, texture0;
    GLfloat lineWidth;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
    glGetFloatv(GL_LINE_WIDTH, &lineWidth);
    glGetIntegerv(GL_BLEND_SRC_RGB, &srcRGB); glGetIntegerv(GL_BLEND_DST_RGB, &dstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha); glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &equationRGB); glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &equationAlpha);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    GLFunctions::ActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture0);
    glDisable(GL_DEPTH_TEST); glDepthMask(GL_FALSE); glDisable(GL_CULL_FACE);
    GLFunctions::BlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glLineWidth(1);

    RenderContext ui = RenderContext::Current();
    ui.SetProjection(Matrix4::Identity());
    ui.SetView(Matrix4::Identity());
    for (const auto& draw : draws) {
        if (draw.blend == HUDLayout::Blend::Opaque) glDisable(GL_BLEND);
        else {
            glEnable(GL_BLEND);
            if (draw.blend == HUDLayout::Blend::Additive) glBlendFunc(GL_ONE, GL_ONE);
            else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        BasicMaterial material;
        material.red = draw.color.x; material.green = draw.color.y; material.blue = draw.color.z;
        material.alpha = draw.alpha;
        if (draw.icon != HUDLayout::Icon::None) material.texture = resources.GetTextureHandler().GetTexture(
            draw.icon == HUDLayout::Icon::Health ? TEXTURE_HEART : TEXTURE_BANG);
        const GeometryResource geometry = draw.shape == HUDLayout::Shape::Quad ? GeometryResource::UIQuad :
            draw.shape == HUDLayout::Shape::Outline ? GeometryResource::UIOutline : GeometryResource::DebugLine;
        const Matrix4 model = Matrix4::Translation(draw.x, draw.y, -1) * Matrix4::Scale(draw.width, draw.height, 0);
        ui.Draw(resources.GetGeometry(geometry), model, material);
    }

    glBindTexture(GL_TEXTURE_2D, texture0);
    GLFunctions::ActiveTexture(activeTexture);
    GLFunctions::BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    GLFunctions::BlendEquationSeparate(equationRGB, equationAlpha);
    glLineWidth(lineWidth); glDepthMask(depthWrite);
    if (depth) glEnable(GL_DEPTH_TEST);
    if (blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (cull) glEnable(GL_CULL_FACE);
}
