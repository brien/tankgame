#include <gtest/gtest.h>
#include "BrowserFramePolicy.h"
#include "InputTask.h"
#include "GlobalTimer.h"

TEST(BrowserFramePolicyTest, HiddenAndUnfocusedFramesNeverTick)
{
    BrowserFramePolicy policy;
    EXPECT_TRUE(policy.NextFrame(0, false, true, false).reset);
    EXPECT_FALSE(policy.NextFrame(16, false, true, false).reset);
    EXPECT_FALSE(policy.NextFrame(32, false, false, true).tick);
    EXPECT_FALSE(policy.NextFrame(48, true, true, true).tick);
    EXPECT_FALSE(policy.NextFrame(5000, true, false, false).tick);
    const auto resume = policy.NextFrame(6000, false, true, true);
    EXPECT_TRUE(resume.tick);
    EXPECT_TRUE(resume.reset);
    EXPECT_FALSE(policy.NextFrame(6016, false, true, false).reset);
}

TEST(BrowserFramePolicyTest, FocusRoundTripBetweenFramesAndLongGapsResetTiming)
{
    BrowserFramePolicy policy;
    policy.NextFrame(0, false, true, false);
    EXPECT_TRUE(policy.NextFrame(16, false, true, true).reset);
    EXPECT_FALSE(policy.NextFrame(32, false, true, false).reset);
    EXPECT_TRUE(policy.NextFrame(1032, false, true, false).reset);
    EXPECT_FALSE(policy.NextFrame(1048, false, true, false).reset);
}

TEST(BrowserLifecycleTest, FocusResetClearsBothInputSnapshotsAndTimer)
{
    ASSERT_EQ(SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_TIMER), 0);
    InputTask input;
    ASSERT_TRUE(input.Start());
    InputTask::keys[SDL_SCANCODE_W] = 1;
    InputTask::oldKeys[SDL_SCANCODE_W] = 1;
    InputTask::buttons = InputTask::oldButtons = SDL_BUTTON_LMASK;
    InputTask::dX = 30;
    InputTask::dY = -20;
    GlobalTimer timer;
    GlobalTimer::dT = 5.0f;
    timer.Reset();
    InputTask::ResetForFocusChange();
    EXPECT_FALSE(InputTask::CurKey(SDL_SCANCODE_W));
    EXPECT_FALSE(InputTask::KeyUp(SDL_SCANCODE_W));
    EXPECT_FALSE(InputTask::CurMouse(SDL_BUTTON_LEFT));
    EXPECT_FALSE(InputTask::MouseUp(SDL_BUTTON_LEFT));
    EXPECT_EQ(InputTask::dX, 0);
    EXPECT_EQ(InputTask::dY, 0);
    EXPECT_FLOAT_EQ(GlobalTimer::dT, 0.0f);
    EXPECT_EQ(GlobalTimer::lastFrameIndex, GlobalTimer::thisFrameIndex);
    input.Update();
    EXPECT_FALSE(InputTask::KeyDown(SDL_SCANCODE_W));
    input.Stop();
    EXPECT_EQ(InputTask::keys, nullptr);
    SDL_QuitSubSystem(SDL_INIT_EVENTS | SDL_INIT_TIMER);
}
