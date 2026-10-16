#include <gtest/gtest.h>

#include "rendering/ViewportManager.h"

TEST(ViewportManagerTest, SinglePlayerUsesEntireFramebuffer)
{
    ViewportManager manager;
    manager.SetupSinglePlayer(1280, 720);

    ASSERT_EQ(manager.GetNumViewports(), 1);
    const Viewport &viewport = manager.GetViewport(0);
    EXPECT_EQ(viewport.x, 0);
    EXPECT_EQ(viewport.y, 0);
    EXPECT_EQ(viewport.width, 1280);
    EXPECT_EQ(viewport.height, 720);
    EXPECT_FLOAT_EQ(viewport.GetAspectRatio(), 1280.0f / 720.0f);
}

TEST(ViewportManagerTest, TwoPlayersUseNonOverlappingHorizontalHalves)
{
    ViewportManager manager;
    manager.SetupSplitScreen(2, 1280, 720);

    ASSERT_EQ(manager.GetNumViewports(), 2);
    const Viewport &player0 = manager.GetViewport(0);
    const Viewport &player1 = manager.GetViewport(1);

    EXPECT_EQ(player0.x, 0);
    EXPECT_EQ(player0.y, 0);
    EXPECT_EQ(player0.width, 1280);
    EXPECT_EQ(player0.height, 360);

    EXPECT_EQ(player1.x, 0);
    EXPECT_EQ(player1.y, 360);
    EXPECT_EQ(player1.width, 1280);
    EXPECT_EQ(player1.height, 360);

    EXPECT_EQ(player0.y + player0.height, player1.y);
    EXPECT_FLOAT_EQ(player0.GetAspectRatio(), player1.GetAspectRatio());
}

TEST(ViewportManagerTest, CoopAndVersusSharePlayerIndexLayout)
{
    // ViewportManager intentionally has no game-mode branch: co-op and versus
    // both map player 0 to the lower viewport and player 1 to the upper one.
    ViewportManager coop;
    ViewportManager versus;
    coop.SetupSplitScreen(2, 800, 600);
    versus.SetupSplitScreen(2, 800, 600);

    for (int player = 0; player < 2; ++player)
    {
        const Viewport &coopViewport = coop.GetViewport(player);
        const Viewport &versusViewport = versus.GetViewport(player);
        EXPECT_EQ(coopViewport.x, versusViewport.x);
        EXPECT_EQ(coopViewport.y, versusViewport.y);
        EXPECT_EQ(coopViewport.width, versusViewport.width);
        EXPECT_EQ(coopViewport.height, versusViewport.height);
    }
}

TEST(ViewportManagerTest, ResizingSplitScreenCoversEveryDrawablePixel)
{
    ViewportManager manager;
    // Odd sizes, a simulated 2x DPI transition, and back to the original size.
    for (const auto& size : {std::make_pair(801, 601), std::make_pair(1602, 1202),
                             std::make_pair(997, 503), std::make_pair(801, 601)})
    {
        manager.SetupSplitScreen(2, size.first, size.second);
        ASSERT_EQ(manager.GetNumViewports(), 2);
        const auto& lower = manager.GetViewport(0);
        const auto& upper = manager.GetViewport(1);
        EXPECT_EQ(manager.GetScreenWidth(), size.first);
        EXPECT_EQ(manager.GetScreenHeight(), size.second);
        EXPECT_EQ(lower.y, 0);
        EXPECT_EQ(lower.y + lower.height, upper.y);
        EXPECT_EQ(upper.y + upper.height, size.second);
        EXPECT_EQ(lower.width, size.first);
        EXPECT_EQ(upper.width, size.first);
        EXPECT_FLOAT_EQ(lower.GetAspectRatio(), float(size.first) / lower.height);
        EXPECT_FLOAT_EQ(upper.GetAspectRatio(), float(size.first) / upper.height);
    }
}

TEST(ViewportManagerTest, EmptyViewsKeepPlayerIndicesUntilRestored)
{
    ViewportManager manager;
    manager.SetupSplitScreen(2, 801, 1);
    ASSERT_EQ(manager.GetNumViewports(), 2);
    EXPECT_FALSE(manager.GetViewport(0).IsValid());
    EXPECT_TRUE(manager.GetViewport(1).IsValid());
    EXPECT_EQ(manager.GetViewport(1).height, 1);

    manager.SetupSplitScreen(2, 0, 0);
    ASSERT_EQ(manager.GetNumViewports(), 2);
    EXPECT_FALSE(manager.GetViewport(0).IsValid());
    EXPECT_FALSE(manager.GetViewport(1).IsValid());
    EXPECT_FLOAT_EQ(manager.GetViewport(0).GetAspectRatio(), 1.f);

    manager.SetupSplitScreen(2, 801, 601);
    ASSERT_EQ(manager.GetNumViewports(), 2);
    EXPECT_EQ(manager.GetViewport(0).height, 300);
    EXPECT_EQ(manager.GetViewport(1).height, 301);
    manager.SetupSinglePlayer(801, 601);
    ASSERT_EQ(manager.GetNumViewports(), 1);
    EXPECT_EQ(manager.GetViewport(0).height, 601);
}
