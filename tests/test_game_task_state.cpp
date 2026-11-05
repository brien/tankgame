#include <gtest/gtest.h>
#include <algorithm>

#include "GameTask.h"
#include "InputTask.h"
#include "GlobalTimer.h"
#include "rendering/HUDDataExtractor.h"

class GameTaskStateTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_TIMER), 0);
        ASSERT_TRUE(input.Start());
        std::fill(InputTask::keys, InputTask::keys + InputTask::keyCount, 0);
        savedDelta = GlobalTimer::dT;
        GlobalTimer::dT = 0.1f;
    }

    void TearDown() override {
        GlobalTimer::dT = savedDelta;
        input.Stop();
        SDL_QuitSubSystem(SDL_INIT_EVENTS | SDL_INIT_TIMER);
    }

    FX* CreateMovingEffect() {
        FX* effect = task.gameWorld.CreateFX(FxType::TYPE_SMOKE,
            1, 2, 3, 4, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        effect->maxTime = 100;
        return effect;
    }

    void Play() { task.TransitionToState(GameTask::GameState::PLAYING); }
    void ReturnToMenu() { task.TransitionToState(GameTask::GameState::MENU); }
    void GameOver() { task.TransitionToState(GameTask::GameState::GAME_OVER); }

    void ExpectMenu(bool visible) {
        const bool started = task.IsGameStarted();
        const auto ui = HUDDataExtractor::ExtractCompleteUIData(
            *task.GetPlayerManager(), started, task.IsPaused(), !started,
            task.GetMenuState(), task.IsDebugMode());
        EXPECT_EQ(ui.menu.isVisible, visible);
        EXPECT_EQ(ui.gameStarted, !visible);
    }

    GameTask task;
    InputTask input;
    float savedDelta = 0;
};

TEST_F(GameTaskStateTest, ReturningFromGameplayRestoresVisibleMenu) {
    ExpectMenu(true);
    Play();
    ExpectMenu(false);
    ReturnToMenu();
    ExpectMenu(true);
    Play();
    ExpectMenu(false);
    ReturnToMenu();
    ExpectMenu(true);
}

TEST_F(GameTaskStateTest, GameOverKeepsGameplayViewUntilReturningToMenu) {
    Play();
    GameOver();
    ExpectMenu(false);
    ReturnToMenu();
    ExpectMenu(true);
}

TEST_F(GameTaskStateTest, MenuFramesFreezeWorldWhileAllowingSelection) {
    Play();
    FX* effect = CreateMovingEffect();
    // Verify this live entity would move and age if updated.
    effect->Update();
    ASSERT_GT(effect->x, 1.0f);
    const float x = effect->x;
    const float age = effect->time;
    ReturnToMenu();

    InputTask::keys[SDL_SCANCODE_RIGHT] = 1;
    task.Update();
    EXPECT_EQ(task.GetMenuState(), 1);
    InputTask::keys[SDL_SCANCODE_RIGHT] = 0;
    for (int frame = 0; frame < 10; ++frame) task.Update();

    EXPECT_FLOAT_EQ(effect->x, x);
    EXPECT_FLOAT_EQ(effect->time, age);
    ExpectMenu(true);
}

TEST_F(GameTaskStateTest, EscapeStopsSimulationOnTheTransitionFrame) {
    Play();
    FX* effect = CreateMovingEffect();
    InputTask::keys[SDL_SCANCODE_ESCAPE] = 1;
    task.Update();

    ExpectMenu(true);
    EXPECT_FLOAT_EQ(effect->x, 1.0f);
    EXPECT_FLOAT_EQ(effect->time, 0.0f);
}
