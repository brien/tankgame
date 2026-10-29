#include <gtest/gtest.h>
#include "BrowserFramePolicy.h"
#include "InputTask.h"
#include "GlobalTimer.h"
#include "SoundTask.h"

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

#if SDL_VERSION_ATLEAST(2, 0, 14)
TEST(BrowserLifecycleTest, JoystickHotplugPreservesSlotsAndBoundsAxes)
{
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    ASSERT_EQ(SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS), 0);
    InputTask input;
    ASSERT_TRUE(input.Start());
    const int first = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 8, 16, 1);
    ASSERT_GE(first, 0);
    input.Update();
    const auto firstId = SDL_JoystickGetDeviceInstanceID(first);
    int firstSlot = -1;
    for (int i = 0; i < 4; ++i)
        if (InputTask::HasJoystick(i) && SDL_JoystickInstanceID(InputTask::joysticks[i]) == firstId) firstSlot = i;
    ASSERT_GE(firstSlot, 0);
    SDL_Joystick* firstHandle = InputTask::joysticks[firstSlot];
    const int second = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 6, 16, 1);
    ASSERT_GE(second, 0);
    input.Update();
    const auto secondId = SDL_JoystickGetDeviceInstanceID(second);
    int secondSlot = -1;
    for (int i = 0; i < 4; ++i)
        if (InputTask::HasJoystick(i) && SDL_JoystickInstanceID(InputTask::joysticks[i]) == secondId) secondSlot = i;
    ASSERT_GE(secondSlot, 0);
    SDL_JoystickSetVirtualAxis(firstHandle, 0, 12000);
    SDL_JoystickSetVirtualAxis(firstHandle, 7, 24000);
    input.Update();
    EXPECT_EQ(InputTask::GetAxis(firstSlot, 0), 12000);
    EXPECT_EQ(InputTask::GetAxis(firstSlot, 7), 24000);
    ASSERT_EQ(SDL_JoystickDetachVirtual(first), 0);
    input.Update();
    EXPECT_FALSE(InputTask::HasJoystick(firstSlot));
    EXPECT_EQ(InputTask::GetAxis(firstSlot, 0), 0);
    EXPECT_EQ(InputTask::GetButton(firstSlot, 0), 0);
    EXPECT_EQ(InputTask::GetHat(firstSlot), SDL_HAT_CENTERED);
    ASSERT_TRUE(InputTask::HasJoystick(secondSlot));
    EXPECT_EQ(SDL_JoystickInstanceID(InputTask::joysticks[secondSlot]), secondId);
    const int replacement = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 4, 16, 0);
    input.Update();
    EXPECT_TRUE(InputTask::HasJoystick(firstSlot));
    EXPECT_EQ(SDL_JoystickInstanceID(InputTask::joysticks[secondSlot]), secondId);
    SDL_JoystickDetachVirtual(replacement);
    for (int i = SDL_NumJoysticks() - 1; i >= 0; --i)
        if (SDL_JoystickGetDeviceInstanceID(i) == secondId) SDL_JoystickDetachVirtual(i);
    input.Update();
    input.Stop();
    EXPECT_EQ(InputTask::joysticks[firstSlot], nullptr);
    EXPECT_EQ(InputTask::joysticks[secondSlot], nullptr);
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS);
}
#endif

TEST(BrowserLifecycleTest, AudioActivationAndFocusPreserveRequestedMusicPause)
{
    // A dummy device exercises the mixer state without requiring audio hardware.
    const char* driver = SDL_getenv("SDL_AUDIODRIVER");
    const std::string previousDriver = driver ? driver : "";
    SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
    SoundTask sound;
    ASSERT_TRUE(sound.Start());
    ASSERT_FALSE(sound.disable);
    Mix_FreeMusic(sound.music);
    const std::string fontPath = TANKGAME_TEST_FONT;
    const std::string musicPath = fontPath.substr(0, fontPath.find("fonts/")) + "sounds/TGTest3.ogg";
    sound.music = Mix_LoadMUS(musicPath.c_str());
    ASSERT_NE(sound.music, nullptr);
    sound.SetActive(false);
    sound.PauseMusic(); // The user can request pause before enabling browser audio.
    sound.PlayMusic(0);
    EXPECT_TRUE(Mix_PausedMusic());
    sound.SetActive(true);
    EXPECT_TRUE(Mix_PausedMusic());
    sound.PauseMusic();
    EXPECT_FALSE(Mix_PausedMusic());
    EXPECT_TRUE(Mix_PlayingMusic());
    sound.SetActive(false);
    EXPECT_TRUE(Mix_PausedMusic());
    sound.SetActive(true);
    EXPECT_FALSE(Mix_PausedMusic());
    sound.Stop();
    sound.Stop();
    EXPECT_EQ(Mix_QuerySpec(nullptr, nullptr, nullptr), 0);
    SDL_setenv("SDL_AUDIODRIVER", previousDriver.c_str(), 1);
}
