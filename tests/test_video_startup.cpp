#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "rendering/ModernRenderer.h"
#include "rendering/GpuGeometry.h"
#include "rendering/Matrix4.h"

#include "App.h"
#include "rendering/GLFunctions.h"
#include "rendering/GpuTexture.h"

// Opt in on a machine with a display and OpenGL:
// TANKGAME_TEST_GL=1 ./tankgame_tests --gtest_filter=VideoStartupTest.*
// Run in separate processes with and without TANKGAME_RENDERER=modern.
class VideoStartupTest : public ::testing::Test
{
protected:
    App app;
    SoundTask sound;
    VideoTask video;
    bool started = false;

    void SetUp() override
    {
        const char* enabled = std::getenv("TANKGAME_TEST_GL");
        if (!enabled || std::strcmp(enabled, "1") != 0)
            GTEST_SKIP() << "Set TANKGAME_TEST_GL=1 to test a real graphics context";

        app.soundTask = &sound;
        started = video.Start();
        ASSERT_TRUE(started);
    }

    void TearDown() override
    {
        if (started) video.Stop();
    }
};

TEST_F(VideoStartupTest, LoadsTextureFunctionsBeforeUpload)
{
    // Check before the upload so a missing loader fails without a null call.
    ASSERT_TRUE(GLFunctions::IsInitialized());
    const ImageData image{2, 2, PixelFormat::RGBA8,
                          std::vector<std::uint8_t>(2 * 2 * 4, 255)};
    const GpuTexture texture(image);
    texture.Bind();

    GLint minFilter = 0;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
    EXPECT_EQ(minFilter, GL_LINEAR_MIPMAP_LINEAR);
    EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
}

TEST_F(VideoStartupTest, RefreshesDrawablePixelsAfterWindowResize)
{
    SDL_Window* window = SDL_GL_GetCurrentWindow();
    ASSERT_NE(window, nullptr);
    EXPECT_NE(SDL_GetWindowFlags(window) & SDL_WINDOW_ALLOW_HIGHDPI, 0u);
    EXPECT_NE(SDL_GetWindowFlags(window) & SDL_WINDOW_RESIZABLE, 0u);
    // Settings may select fullscreen; use a window for deterministic size changes.
    ASSERT_EQ(SDL_SetWindowFullscreen(window, 0), 0);
    for (const auto& size : {std::make_pair(801, 601), std::make_pair(997, 503)})
    {
        SDL_SetWindowSize(window, size.first, size.second);
        SDL_PumpEvents();
        int width = 0, height = 0;
        SDL_GL_GetDrawableSize(window, &width, &height);
        ASSERT_TRUE(video.RefreshDrawableSize());
        EXPECT_EQ(VideoTask::scrWidth, width);
        EXPECT_EQ(VideoTask::scrHeight, height);
    }
    EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
}

// Exercise the actual submission boundary, including quad triangulation and
// line topology, independently of the selected game renderer.

TEST_F(VideoStartupTest, CountsSubmittedDrawsAndTriangles)
{
    ModernRenderer renderer;
    const Matrix4 identity;
    const BasicMaterial material;
    const GpuGeometry quad(SimpleGeometry::CreateHorizontalSquare());
    const GpuGeometry loop(SimpleGeometry::CreateHorizontalSquare(.5f, PrimitiveTopology::LINE_LOOP));
    Geometry segment;
    segment.topology = PrimitiveTopology::LINES;
    segment.vertices.resize(2);
    const GpuGeometry line(segment);
    EXPECT_EQ(renderer.GetSubmissionStats().drawCalls, 0u);
    EXPECT_THROW(renderer.Draw(quad, nullptr, material, identity.Data(), false), std::invalid_argument);
    EXPECT_EQ(renderer.GetSubmissionStats().drawCalls, 0u);
    renderer.Draw(quad, identity.Data(), material, identity.Data(), false);
    renderer.Draw(quad, identity.Data(), material, identity.Data(), false);
    renderer.Draw(loop, identity.Data(), material, identity.Data(), false);
    renderer.Draw(line, identity.Data(), material, identity.Data(), false);
    EXPECT_EQ(renderer.GetSubmissionStats().drawCalls, 4u);
    EXPECT_EQ(renderer.GetSubmissionStats().triangles, 4u);
    EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
    ModernRenderer recreated;
    EXPECT_EQ(recreated.GetSubmissionStats().drawCalls, 0u);
    EXPECT_EQ(recreated.GetSubmissionStats().triangles, 0u);
}

#ifndef __EMSCRIPTEN__
// Exercise the macOS context contract on any desktop driver that supports it.
class CoreRendererTest : public ::testing::Test
{
protected:
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    bool videoInitialized = false;

    void SetUp() override
    {
        const char* enabled = std::getenv("TANKGAME_TEST_GL");
        if (!enabled || std::strcmp(enabled, "1") != 0)
            GTEST_SKIP() << "Set TANKGAME_TEST_GL=1 to test a real graphics context";
        ASSERT_EQ(SDL_InitSubSystem(SDL_INIT_VIDEO), 0) << SDL_GetError();
        videoInitialized = true;
        SDL_GL_ResetAttributes();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        window = SDL_CreateWindow("Core renderer test", 0, 0, 64, 64,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        ASSERT_NE(window, nullptr) << SDL_GetError();
        context = SDL_GL_CreateContext(window);
        ASSERT_NE(context, nullptr) << SDL_GetError();
        std::string error;
        ASSERT_TRUE(GLFunctions::Initialize(error)) << error;
        ASSERT_TRUE(GLFunctions::IsCoreProfile());
    }

    void TearDown() override
    {
        if (context) {
            GLFunctions::Shutdown();
            SDL_GL_DeleteContext(context);
        }
        if (window) SDL_DestroyWindow(window);
        if (videoInitialized) {
            SDL_GL_ResetAttributes();
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
    }
};

TEST_F(CoreRendererTest, DrawsVisiblePixelsAfterVertexArrayBindingChanges)
{
    const Matrix4 identity;
    BasicMaterial material;
    material.green = material.blue = 0.0f;
    Geometry triangle;
    triangle.topology = PrimitiveTopology::TRIANGLES;
    triangle.vertices = {{-1, -1, 0}, {1, -1, 0}, {0, 1, 0}};
    const GpuGeometry geometry(triangle);
    glViewport(0, 0, 64, 64);
    glClearColor(0, 0, 0, 1);
    // Also cover destruction/recreation while the same context remains alive.
    for (int iteration = 0; iteration < 2; ++iteration) {
        ModernRenderer renderer;
        for (int draw = 0; draw < 2; ++draw) {
            GLFunctions::BindVertexArray(0);
            glClear(GL_COLOR_BUFFER_BIT);
            renderer.Draw(geometry, identity.Data(), material, identity.Data(), false);
            unsigned char pixel[4] = {};
            glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
            EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
            EXPECT_GT(pixel[0], 240);
            EXPECT_LT(pixel[1], 10);
            EXPECT_LT(pixel[2], 10);
        }
    }
    EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
}
#endif
