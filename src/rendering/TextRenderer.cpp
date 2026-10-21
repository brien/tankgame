#include "TextRenderer.h"
#include "ResourceManager.h"
#include "RenderContext.h"
#include "GLFunctions.h"
#include <algorithm>
#include <cstdio>
#include <cmath>

std::vector<UITextLine> TextRenderer::PlayerLines(const HUDRenderData& data, int width) {
    if (!data.showFPSCounter || width <= 0) return {};
    char text[64];
    std::snprintf(text, sizeof(text), "FPS: %.1f", std::isfinite(data.currentFPS) ? data.currentFPS : 0.0f);
    return {{text, float(std::max(8, width - 160)), 12, Vector3(1,1,1)}};
}

std::vector<UITextLine> TextRenderer::DebugLines(const DebugRenderData& data) {
    std::vector<UITextLine> lines;
    if (!data.showDebugInfo) return lines;
    char text[64];
    if (data.showPerformanceStats) {
        std::snprintf(text, sizeof(text), "FPS: %.1f", data.averageFPS);
        lines.push_back({text, 12, 72, data.performanceColor});
        std::snprintf(text, sizeof(text), "Frame: %.2fms", data.frameTime);
        lines.push_back({text, 12, 94, data.performanceColor});
    }
    if (data.showRenderStats) {
        std::snprintf(text, sizeof(text), "Calls: %d", data.renderCallCount);
        lines.push_back({text, 12, 116, data.debugTextColor});
        std::snprintf(text, sizeof(text), "Tris: %d", data.triangleCount);
        lines.push_back({text, 12, 138, data.debugTextColor});
    }
    return lines;
}

void TextRenderer::Render(ResourceManager& resources, const std::vector<UITextLine>& lines) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (lines.empty() || viewport[2] <= 0 || viewport[3] <= 0 || !resources.GetFontTexture()) return;
    const GLboolean depth = glIsEnabled(GL_DEPTH_TEST), blend = glIsEnabled(GL_BLEND);
    const GLboolean cull = glIsEnabled(GL_CULL_FACE);
    GLboolean depthWrite;
    GLint srcRGB, dstRGB, srcAlpha, dstAlpha, equationRGB, equationAlpha, activeTexture, texture0;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
    glGetIntegerv(GL_BLEND_SRC_RGB, &srcRGB); glGetIntegerv(GL_BLEND_DST_RGB, &dstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha); glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &equationRGB); glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &equationAlpha);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    GLFunctions::ActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture0);
    glDisable(GL_DEPTH_TEST); glDepthMask(GL_FALSE); glDisable(GL_CULL_FACE);
    GLFunctions::BlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    RenderContext ui = RenderContext::Current();
    ui.SetProjection(Matrix4::Identity());
    ui.SetView(Matrix4::Identity());
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const auto& atlas = resources.GetFontAtlas();
    const float scale = 18.0f / atlas.lineHeight;
    for (const auto& line : lines) {
        float x = line.x;
        BasicMaterial material;
        material.texture = resources.GetFontTexture();
        material.red = line.color.x; material.green = line.color.y; material.blue = line.color.z;
        for (unsigned char character : line.text) {
            const auto& glyph = atlas.glyphs[FontAtlas::GlyphIndex(character)];
            const float w = glyph.width * scale, h = glyph.height * scale;
            if (character != ' ') {
                const Matrix4 model = Matrix4::Translation(2*x/viewport[2]-1, 1-2*(line.y+h)/viewport[3], 0) *
                    Matrix4::Scale(2*w/viewport[2], 2*h/viewport[3], 1);
                ui.Draw(resources.GetFontGlyph(character), model, material);
            }
            x += glyph.advance * scale;
        }
    }

    glBindTexture(GL_TEXTURE_2D, texture0);
    GLFunctions::ActiveTexture(activeTexture);
    GLFunctions::BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    GLFunctions::BlendEquationSeparate(equationRGB, equationAlpha);
    glDepthMask(depthWrite);
    if (depth) glEnable(GL_DEPTH_TEST);
    if (blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (cull) glEnable(GL_CULL_FACE);
}
