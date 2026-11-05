#include <gtest/gtest.h>

#include "GameTask.h"
#include "rendering/HUDDataExtractor.h"

class GameTaskStateTest : public ::testing::Test {
protected:
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
