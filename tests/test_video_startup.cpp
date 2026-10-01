#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>

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
