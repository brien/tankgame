#include <gtest/gtest.h>
#include "rendering/FontAtlas.h"
#include "rendering/TextRenderer.h"
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
