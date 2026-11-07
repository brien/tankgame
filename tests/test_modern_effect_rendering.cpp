#include <gtest/gtest.h>

#include "App.h"
#include "rendering/EffectRenderer.h"
#include "rendering/RenderContext.h"
#include "rendering/RendererMode.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

// Behavioral pixel/depth checks derived from the recorded effects fixture.
// No compatibility rendering or driver-specific golden-image equality is needed.
class ModernEffectRenderingTest : public ::testing::Test
{
protected:
    static constexpr int Width = 320;
    static constexpr int Height = 240;
    App app;
    SoundTask sound;
    VideoTask video;
    bool started = false;
    std::unique_ptr<ResourceManager> resources;
    std::unique_ptr<EffectRenderer> renderer;

    void SetUp() override
    {
        const char* enabled = std::getenv("TANKGAME_TEST_GL");
        if (!enabled || std::strcmp(enabled, "1") != 0)
            GTEST_SKIP() << "Set TANKGAME_TEST_GL=1 to test a real graphics context";
        if (!RendererMode::IsModern())
            GTEST_SKIP() << "This regression contract exercises modern rendering";
        app.soundTask = &sound;
        started = video.Start();
        ASSERT_TRUE(started);
        int width = 0, height = 0;
        SDL_GL_GetDrawableSize(SDL_GL_GetCurrentWindow(), &width, &height);
        ASSERT_GE(width, Width);
        ASSERT_GE(height, Height);
        resources.reset(new ResourceManager);
        ASSERT_TRUE(resources->Initialize());
        renderer.reset(new EffectRenderer(resources.get()));
        ASSERT_TRUE(renderer->Initialize());
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0, 0, 0, 1);
        RenderContext::Current().SetLighting(false);
        ASSERT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
    }

    void TearDown() override
    {
        if (renderer) renderer->Cleanup();
        renderer.reset();
        if (resources) resources->Cleanup();
        resources.reset();
        if (started) video.Stop();
    }

    std::vector<unsigned char> Pixels()
    {
        std::vector<unsigned char> pixels(Width * Height * 4);
        glReadPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        return pixels;
    }

    std::vector<float> Depth()
    {
        std::vector<float> depth(Width * Height);
        glReadPixels(0, 0, Width, Height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
        return depth;
    }
};

constexpr int ModernEffectRenderingTest::Width;
constexpr int ModernEffectRenderingTest::Height;

TEST_F(ModernEffectRenderingTest, EveryEffectDrawsAndAdditivePassesPreserveDepthAndState)
{
    auto& context = RenderContext::Current();
    context.SetProjection(Matrix4::Perspective(45, float(Width) / Height, .1f, 100));
    context.SetView(Matrix4::LookAt(0, 4, 4, 0, 0, 0, 0, 1, 0));
    glViewport(0, 0, Width, Height);
    for (int type = 0; type < 8; ++type) {
        SCOPED_TRACE(type);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const auto before = Pixels();
        const auto depth = Depth();
        EffectRenderData effect;
        effect.type = static_cast<FxType>(type);
        effect.scale = 1;
        effect.r = .3f; effect.g = .6f; effect.b = .8f; effect.alpha = .75f;
        renderer->RenderEffects({effect});
        const auto after = Pixels();
        int changed = 0;
        for (size_t i = 0; i < before.size(); i += 4)
            changed += !std::equal(before.begin() + i, before.begin() + i + 3, after.begin() + i);
        EXPECT_GT(changed, 0) << "An empty pass must not satisfy the state checks";
        // Types 0 and 6 intentionally include depth-writing outlines.
        if (type != 0 && type != 6) EXPECT_EQ(Depth(), depth);
        GLboolean depthWrite = GL_FALSE;
        GLint source = 0, destination = 0;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
        glGetIntegerv(GL_BLEND_SRC, &source);
        glGetIntegerv(GL_BLEND_DST, &destination);
        EXPECT_TRUE(depthWrite);
        EXPECT_FALSE(glIsEnabled(GL_BLEND));
        EXPECT_TRUE(glIsEnabled(GL_DEPTH_TEST));
        EXPECT_TRUE(glIsEnabled(GL_CULL_FACE));
        EXPECT_EQ(source, GL_SRC_ALPHA);
        EXPECT_EQ(destination, GL_ONE_MINUS_SRC_ALPHA);
        EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
    }
}

TEST_F(ModernEffectRenderingTest, OpaqueOcclusionAndSplitScreenIsolation)
{
    EffectRenderData effect;
    effect.type = FxType::TYPE_THREE;
    effect.position = Vector3(3.5f, .8f, 3);
    effect.rotation.x = 90;
    effect.alpha = .5f;
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int view = 0; view < 2; ++view) {
        SCOPED_TRACE(view);
        glViewport(view * Width / 2, 0, Width / 2, Height);
        auto& context = RenderContext::Current();
        context.SetProjection(Matrix4::Perspective(45, float(Width / 2) / Height, .1f, 100));
        context.SetView(Matrix4::LookAt(3.5f, 1, -5, 3.5f, 1, 3, 0, 1, 0));
        glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE); glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
        context.Draw(resources->GetGeometry(GeometryResource::TerrainCube),
            Matrix4::Translation(3.5f, 1, view == 0 ? 2 : 5) * Matrix4::Scale(4, 4, .2f),
            .1f, .15f, .2f);
        const auto before = Pixels();
        const auto depth = Depth();
        renderer->RenderEffects({effect});
        const auto after = Pixels();
        int activeChanged = 0, otherChanged = 0;
        for (int y = 0; y < Height; ++y) {
            for (int x = 0; x < Width; ++x) {
                const int i = (y * Width + x) * 4;
                if (!std::equal(before.begin() + i, before.begin() + i + 3, after.begin() + i)) {
                    if (x / (Width / 2) == view) ++activeChanged;
                    else ++otherChanged;
                }
            }
        }
        EXPECT_EQ(otherChanged, 0);
        if (view == 0) EXPECT_EQ(activeChanged, 0);
        else EXPECT_GT(activeChanged, 0);
        EXPECT_EQ(Depth(), depth);
        EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
    }
}
