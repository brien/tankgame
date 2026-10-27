#include "TextRenderer.h"
#include "ResourceManager.h"
#include "RenderContext.h"
#include "GLFunctions.h"
#include <algorithm>
#include <cstdio>
#include <cinttypes>
#include <cmath>

std::vector<UITextLine> TextRenderer::MenuLines(const MenuRenderData& data, int width, int height) {
    if (!data.isVisible || width <= 0 || height <= 0) return {};
    std::vector<UITextLine> lines;
    // A global legend complements the terrain's title artwork without placing
    // glyphs over its baked-in labels. Keep clear of diagnostics starting at y=72.
    const int count = std::min(std::max(0, data.numOptions),
                               static_cast<int>(data.menuOptions.size()));
    for (int i = 0; i < count; ++i) {
        const bool enabled = i >= static_cast<int>(data.optionEnabled.size()) || data.optionEnabled[i];
        const bool selected = i == data.selectedOption;
        const Vector3 color = !enabled ? data.disabledColor :
            (selected ? data.selectedColor : data.normalColor);
        lines.push_back({std::string(selected ? "> " : "  ") + data.menuOptions[i],
                         12, 12.0f + 20.0f * i, color});
    }
    return lines;
}

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
        std::snprintf(text, sizeof(text), "Calls: %" PRIu64, data.renderCallCount);
        lines.push_back({text, 12, 116, data.debugTextColor});
        std::snprintf(text, sizeof(text), "Tris: %" PRIu64, data.triangleCount);
        lines.push_back({text, 12, 138, data.debugTextColor});
    }
    return lines;
}

std::vector<UITextLine> TextRenderer::StatusLines(const HUDRenderData& data, int width, int height) {
    // Fixed rows keep changing values and visibility from moving other labels.
    // Coordinates are local to the current drawable viewport, including splits.
    if (width <= 0 || height <= 0) return {};
    const float top = float(std::max(0, height - 96));
    const auto number = [](float value) {
        if (!std::isfinite(value)) return std::string("?");
        char text[64];
        std::snprintf(text, sizeof(text), "%.0f", value);
        return std::string(text);
    };
    std::vector<UITextLine> lines;
    if (data.showHealthBar)
        lines.push_back({"Health: " + number(data.health) + " / " + number(data.maxHealth),
            12, top, data.healthColor});
    if (data.showEnergyBar)
        lines.push_back({"Energy: " + number(data.energy) + " / " + number(data.maxEnergy),
            12, top + 22, data.energyColor});
    if (data.showComboMeter)
        lines.push_back({"Combo: " + std::to_string(data.comboNumber),
            12, top + 44, data.comboColor});
    if (data.showSpecialMeter)
        lines.push_back({"Special: " + number(data.special),
            12, top + 66, data.specialColor});
    return lines;
}

void TextRenderer::Render(ResourceManager& resources, const std::vector<UITextLine>& lines) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (lines.empty() || viewport[2] <= 0 || viewport[3] <= 0 || !resources.GetFontTexture()) return;
    const GLboolean depth = glIsEnabled(GL_DEPTH_TEST), blend = glIsEnabled(GL_BLEND);
    const GLboolean cull = glIsEnabled(GL_CULL_FACE);
    const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    GLint scissorBox[4];
    glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
    // A viewport transforms geometry but does not clip it. Keep long status
    // values and very small split views inside their owning player's rectangle.
    int left = viewport[0], bottom = viewport[1];
    int right = left + viewport[2], top = bottom + viewport[3];
    if (scissor) {
        left = std::max(left, scissorBox[0]);
        bottom = std::max(bottom, scissorBox[1]);
        right = std::min(right, scissorBox[0] + scissorBox[2]);
        top = std::min(top, scissorBox[1] + scissorBox[3]);
    }
    glEnable(GL_SCISSOR_TEST);
    glScissor(left, bottom, std::max(0, right - left), std::max(0, top - bottom));
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
    glScissor(scissorBox[0], scissorBox[1], scissorBox[2], scissorBox[3]);
    if (!scissor) glDisable(GL_SCISSOR_TEST);
}
