#include <gtest/gtest.h>
#include <limits>
#include "rendering/FontAtlas.h"
#include "rendering/TextRenderer.h"
#include "rendering/HUDDataExtractor.h"
#include "GraphicsTask.h"
#include <SDL2/SDL_ttf.h>

TEST(FontAtlasTest, GraphicsTaskConstructionDoesNotAcquireTtf) {
    // Construction precedes resource initialization and must not open an unused
    // legacy font or retain a TTF reference across the catalogue's scoped loads.
    const int initialized = TTF_WasInit();
    {
        GraphicsTask graphics;
        EXPECT_EQ(TTF_WasInit(), initialized);
        EXPECT_EQ(graphics.resourceManager, nullptr);
    }
    EXPECT_EQ(TTF_WasInit(), initialized);
}

TEST(FontAtlasTest, BundledFontProducesBoundedRgbaGlyphsAndCoverage) {
    const int initialized = TTF_WasInit();
    const auto atlas = FontAtlas::Load(TANKGAME_TEST_FONT);
    EXPECT_EQ(TTF_WasInit(), initialized);
    ASSERT_TRUE(atlas.image.IsValid());
    ASSERT_GT(atlas.lineHeight, 0);
    bool coverage = false;
    for (std::size_t i = 3; i < atlas.image.pixels.size(); i += 4) coverage |= atlas.image.pixels[i] != 0;
    EXPECT_TRUE(coverage);
    for (const auto& glyph : atlas.glyphs) {
        EXPECT_GT(glyph.advance, 0);
        ASSERT_EQ(glyph.geometry.vertices.size(), 4u);
        for (const auto& vertex : glyph.geometry.vertices) {
            EXPECT_GT(vertex.u, 0); EXPECT_LT(vertex.u, 1);
            EXPECT_GT(vertex.v, 0); EXPECT_LT(vertex.v, 1);
        }
        EXPECT_GT(glyph.geometry.vertices[0].v, glyph.geometry.vertices[2].v);
    }
}
TEST(FontAtlasTest, MissingFontUnwindsTtfAndUnsupportedBytesUseFallback) {
    const int initialized = TTF_WasInit();
    EXPECT_THROW(FontAtlas::Load("missing-font.ttf"), std::runtime_error);
    EXPECT_EQ(TTF_WasInit(), initialized);
    EXPECT_EQ(FontAtlas::GlyphIndex(0), FontAtlas::GlyphIndex('?'));
    EXPECT_EQ(FontAtlas::GlyphIndex(255), FontAtlas::GlyphIndex('?'));
    EXPECT_EQ(FontAtlas::GlyphIndex('~'), FontAtlas::Count - 1);
}
TEST(UITextTest, PlayerVisibilityAndGlobalDebugFlagsAreIndependent) {
    HUDRenderData hud; hud.currentFPS = 60;
    const auto lines = TextRenderer::PlayerLines(hud, 800);
    ASSERT_EQ(lines.size(), 1u); EXPECT_EQ(lines[0].text, "FPS: 60.0");
    hud.showFPSCounter = false;
    EXPECT_TRUE(TextRenderer::PlayerLines(hud, 800).empty());
    hud.showFPSCounter = true;
    EXPECT_TRUE(TextRenderer::PlayerLines(hud, 0).empty());
    DebugRenderData debug;
    EXPECT_TRUE(TextRenderer::DebugLines(debug).empty());
    debug.showDebugInfo = true; debug.showPerformanceStats = true;
    EXPECT_EQ(TextRenderer::DebugLines(debug).size(), 2u);
    debug.showRenderStats = true;
    EXPECT_EQ(TextRenderer::DebugLines(debug).size(), 4u);
    debug.showPerformanceStats = false;
    ASSERT_EQ(TextRenderer::DebugLines(debug).size(), 2u);
    EXPECT_EQ(TextRenderer::DebugLines(debug)[0].text, "Calls: 0");
}

TEST(UITextTest, StatusUsesPlayerValuesColorsAndIndependentVisibility) {
    HUDRenderData hud;
    hud.health = 37; hud.maxHealth = 150;
    hud.energy = 12; hud.maxEnergy = 200;
    hud.comboNumber = 7; hud.special = 125;
    hud.showFPSCounter = false;
    const auto lines = TextRenderer::StatusLines(hud, 801, 301);
    ASSERT_EQ(lines.size(), 4u);
    EXPECT_EQ(lines[0].text, "Health: 37 / 150");
    EXPECT_EQ(lines[1].text, "Energy: 12 / 200");
    EXPECT_EQ(lines[2].text, "Combo: 7");
    EXPECT_EQ(lines[3].text, "Special: 125");
    EXPECT_FLOAT_EQ(lines[0].color.x, hud.healthColor.x);
    EXPECT_FLOAT_EQ(lines[1].color.y, hud.energyColor.y);
    EXPECT_FLOAT_EQ(lines[2].color.z, hud.comboColor.z);
    EXPECT_FLOAT_EQ(lines[3].color.x, hud.specialColor.x);
    hud.showHealthBar = hud.showComboMeter = false;
    const auto hidden = TextRenderer::StatusLines(hud, 801, 301);
    ASSERT_EQ(hidden.size(), 2u);
    EXPECT_EQ(hidden[0].text, lines[1].text);
    EXPECT_EQ(hidden[1].text, lines[3].text);
    EXPECT_EQ(hidden[0].y, lines[1].y);
    EXPECT_EQ(hidden[1].y, lines[3].y);
    hud.showEnergyBar = hud.showSpecialMeter = false;
    EXPECT_TRUE(TextRenderer::StatusLines(hud, 801, 301).empty());
}

TEST(UITextTest, StatusUsesLocalViewportAndHandlesInvalidNumbers) {
    HUDRenderData hud;
    const auto full = TextRenderer::StatusLines(hud, 801, 601);
    const auto split = TextRenderer::StatusLines(hud, 801, 300);
    ASSERT_EQ(full.size(), split.size());
    for (std::size_t i = 0; i < full.size(); ++i) {
        EXPECT_FLOAT_EQ(full[i].y - split[i].y, 301);
        EXPECT_GE(split[i].y, 0);
        EXPECT_LT(split[i].y + 18, 300);
    }
    EXPECT_TRUE(TextRenderer::StatusLines(hud, 0, 300).empty());
    EXPECT_TRUE(TextRenderer::StatusLines(hud, 801, 0).empty());
    hud.health = std::numeric_limits<float>::quiet_NaN();
    hud.energy = std::numeric_limits<float>::infinity();
    hud.maxEnergy = 0;
    const auto invalid = TextRenderer::StatusLines(hud, 801, 300);
    EXPECT_EQ(invalid[0].text, "Health: ? / 100");
    EXPECT_EQ(invalid[1].text, "Energy: ? / 0");
}

TEST(UITextTest, MenuLegendMatchesModesAndTracksSelection) {
    for (int selected = 0; selected < 3; ++selected) {
        const auto menu = HUDDataExtractor::ExtractMenuData(selected, true);
        const auto lines = TextRenderer::MenuLines(menu, 801, 601);
        ASSERT_EQ(lines.size(), 3u);
        const char* names[] = {"Single player", "Co-op", "Versus"};
        for (int i = 0; i < 3; ++i) {
            EXPECT_EQ(lines[i].text, std::string(i == selected ? "> " : "  ") + names[i]);
            EXPECT_FLOAT_EQ(lines[i].color.y, i == selected ? menu.selectedColor.y : menu.normalColor.y);
            EXPECT_LT(lines[i].y + 18, 72);
        }
    }
}

TEST(UITextTest, MenuLegendHandlesHiddenDisabledAndIncompleteSnapshots) {
    auto menu = HUDDataExtractor::ExtractMenuData(1, false);
    EXPECT_TRUE(TextRenderer::MenuLines(menu, 801, 601).empty());
    menu.isVisible = true;
    EXPECT_TRUE(TextRenderer::MenuLines(menu, 0, 601).empty());
    EXPECT_TRUE(TextRenderer::MenuLines(menu, 801, 0).empty());
    menu.optionEnabled[1] = false;
    auto lines = TextRenderer::MenuLines(menu, 801, 301);
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_FLOAT_EQ(lines[1].color.y, menu.disabledColor.y);
    menu.numOptions = 9;
    menu.optionEnabled.clear();
    EXPECT_EQ(TextRenderer::MenuLines(menu, 801, 301).size(), 3u);
    menu.selectedOption = -1;
    lines = TextRenderer::MenuLines(menu, 801, 301);
    EXPECT_EQ(lines[0].text, "  Single player");
    menu.menuOptions.clear();
    EXPECT_TRUE(TextRenderer::MenuLines(menu, 801, 301).empty());
    menu.numOptions = -1;
    EXPECT_TRUE(TextRenderer::MenuLines(menu, 801, 301).empty());
}

TEST(TextRendererTest, FormatsLargeSubmissionCountsWithoutTruncation)
{
    DebugRenderData debug;
    debug.showDebugInfo = debug.showRenderStats = true;
    debug.renderCallCount = 4294967296ULL;
    debug.triangleCount = 18446744073709551615ULL;
    const auto lines = TextRenderer::DebugLines(debug);
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0].text, "Calls: 4294967296");
    EXPECT_EQ(lines[1].text, "Tris: 18446744073709551615");
}
