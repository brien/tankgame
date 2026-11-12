#include <gtest/gtest.h>
#include <algorithm>

#include "App.h"
#include "LevelHandler.h"
#include "TankHandler.h"
#include "events/Events.h"
#include "Bullet.h"
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
        app.gameTask = &task;
        app.soundTask = &sound;
        sound.disable = true;
        LevelHandler::Create();
        TankHandler::Create();
        LevelHandler::GetSingleton().SetGameWorld(&task.gameWorld);
        TankHandler::GetSingleton().SetGameWorld(&task.gameWorld);
        task.gameWorld.Initialize();
        task.playerManager.Initialize(&task.gameWorld);
        savedDelta = GlobalTimer::dT;
        GlobalTimer::dT = 0.1f;
    }

    void TearDown() override {
        Events::Clear();
        task.playerManager.ReleaseTanks();
        task.playerManager.Shutdown();
        task.gameWorld.Clear();
        task.gameWorld.Shutdown();
        TankHandler::Destroy();
        LevelHandler::Destroy();
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

    void StartMode(int mode) {
        ReturnToMenu();
        task.menuState = mode;
        std::fill(InputTask::keys, InputTask::keys + InputTask::keyCount, 0);
        InputTask::keys[SDL_SCANCODE_RETURN] = 1;
        task.Update();
        InputTask::keys[SDL_SCANCODE_RETURN] = 0;
    }

    GameWorld& World() { return task.gameWorld; }
    void ExpectOwnedPlayers(int count) {
        auto* manager = task.GetPlayerManager();
        ASSERT_EQ(manager->GetNumPlayers(), count);
        int playerTanks = 0;
        for (const auto& tank : World().GetTanks()) if (tank->isPlayer) ++playerTanks;
        EXPECT_EQ(playerTanks, count);
        for (int p = 0; p < count; ++p) {
            auto* player = manager->GetPlayer(p);
            ASSERT_NE(player, nullptr);
            auto* tank = player->GetControlledTank();
            ASSERT_NE(tank, nullptr);
            EXPECT_TRUE(std::any_of(World().GetTanks().begin(), World().GetTanks().end(),
                [tank](const auto& owned) { return owned.get() == tank; }));
            EXPECT_TRUE(tank->alive);
            EXPECT_EQ(tank->identity, TankIdentity::Player(p));
        }
        if (count == 1) EXPECT_EQ(manager->GetPlayer(1), nullptr);
    }

    App app;
    SoundTask sound;
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

TEST_F(GameTaskStateTest, EveryModeTransitionStartsFreshSession) {
    for (int from = 0; from < 3; ++from) {
        for (int to = 0; to < 3; ++to) {
            SCOPED_TRACE(from);
            SCOPED_TRACE(to);
            StartMode(from);
            auto* player = task.GetPlayerManager()->GetPlayer(0);
            player->AddScore(100);
            player->AddWin();
            player->AddCombo(10, 2);
            player->SetSpecialCharge(50);
            World().CreateBullet(Bullet{});
            CreateMovingEffect();
            StartMode(to);
            ExpectOwnedPlayers(to == 0 ? 1 : 2);
            EXPECT_EQ(task.IsVersusMode(), to == 2);
            EXPECT_EQ(task.GetPlayerManager()->IsVersusMode(), to == 2);
            EXPECT_TRUE(World().GetBullets().empty());
            EXPECT_TRUE(World().GetFX().empty());
            player = task.GetPlayerManager()->GetPlayer(0);
            EXPECT_EQ(player->GetScore(), 0);
            EXPECT_EQ(player->GetWins(), 0);
            EXPECT_FLOAT_EQ(player->GetCombo(), 0);
            EXPECT_FLOAT_EQ(player->GetSpecialCharge(), 0);
            EXPECT_EQ(World().GetTanks().size(), static_cast<size_t>(
                LevelHandler::GetSingleton().GetEnemyCountForLevel(48) + (to == 0 ? 1 : 2)));
        }
    }
}

TEST_F(GameTaskStateTest, CoopDeathReloadsBothPlayersAndDiscardsOldEvents) {
    StartMode(1);
    auto* manager = task.GetPlayerManager();
    for (int deadPlayer : {0, 1, 0, 1}) {
        auto* tank = manager->GetPlayer(deadPlayer)->GetControlledTank();
        tank->alive = false;
        tank->deadtime = 1;
        auto* bullet = World().CreateBullet(Bullet{});
        Events::Post(BulletTimeoutEvent(bullet, 100));
        manager->NextFrame();
        // Reload is deferred until the caller has finished using entity pointers.
        EXPECT_EQ(manager->GetPlayer(deadPlayer)->GetControlledTank(), tank);
        LevelHandler::GetSingleton().ProcessPendingLevelChange();
        Events::ProcessQueuedEvents();
        ExpectOwnedPlayers(2);
        EXPECT_TRUE(World().GetBullets().empty());
        EXPECT_EQ(LevelHandler::GetSingleton().levelNumber, 48);
    }
}

TEST_F(GameTaskStateTest, VersusAwardsOneWinPerRoundAndRestoresBothPlayers) {
    StartMode(2);
    auto* manager = task.GetPlayerManager();
    for (int round = 1; round <= 3; ++round) {
        manager->GetPlayer(1)->GetControlledTank()->alive = false;
        for (int frame = 0; frame < 10; ++frame) manager->NextFrame();
        EXPECT_EQ(manager->GetPlayer(0)->GetWins(), round);
        EXPECT_EQ(manager->GetPlayer(1)->GetWins(), 0);
        manager->GetPlayer(1)->GetControlledTank()->deadtime = 2;
        manager->NextFrame();
        LevelHandler::GetSingleton().ProcessPendingLevelChange();
        ExpectOwnedPlayers(2);
    }
    EXPECT_TRUE(manager->HasWinner());
    GameOver();
    StartMode(0);
    ExpectOwnedPlayers(1);
    EXPECT_FALSE(manager->HasWinner());
}

TEST_F(GameTaskStateTest, SimultaneousVersusDeathsDoNotAwardWin) {
    StartMode(2);
    auto* manager = task.GetPlayerManager();
    for (int p = 0; p < 2; ++p) {
        manager->GetPlayer(p)->GetControlledTank()->alive = false;
        manager->GetPlayer(p)->GetControlledTank()->deadtime = 2;
    }
    manager->NextFrame();
    LevelHandler::GetSingleton().ProcessPendingLevelChange();
    ExpectOwnedPlayers(2);
    EXPECT_EQ(manager->GetPlayer(0)->GetWins(), 0);
    EXPECT_EQ(manager->GetPlayer(1)->GetWins(), 0);
}

TEST_F(GameTaskStateTest, InputLevelAdvanceIsAppliedAfterBothPlayersUpdate) {
    StartMode(1);
    InputTask::keys[SDL_SCANCODE_T] = 1;
    task.Update();
    InputTask::keys[SDL_SCANCODE_T] = 0;
    ExpectOwnedPlayers(2);
    EXPECT_EQ(LevelHandler::GetSingleton().levelNumber, 49);
    task.Update();
    ExpectOwnedPlayers(2);
}

TEST_F(GameTaskStateTest, VersusUsesDistinctAuthoredSpawns) {
    StartMode(2);
    auto& level = LevelHandler::GetSingleton();
    auto* manager = task.GetPlayerManager();
    for (int p = 0; p < 2; ++p) {
        const auto* tank = manager->GetPlayer(p)->GetControlledTank();
        EXPECT_FLOAT_EQ(tank->x, level.enemy[8 + p][0]);
        EXPECT_FLOAT_EQ(tank->z, level.enemy[8 + p][1]);
        EXPECT_FLOAT_EQ(tank->y, level.GetTerrainHeight(tank->x, tank->z));
    }
    EXPECT_NE(manager->GetPlayer(0)->GetControlledTank()->x,
              manager->GetPlayer(1)->GetControlledTank()->x);
}
