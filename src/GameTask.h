#pragma once

#include "ITask.h"
#include "GameWorld.h"
#include "PlayerManager.h"

class GameTask : public ITask
{
public:
    GameTask();
    virtual ~GameTask();

    bool Start() override;
    void Update() override;
    void Stop() override;
    void OnSuspend() override;
    void OnResume() override;

    // Public getter methods for private flags
    bool IsGameStarted() const { return currentState != GameState::MENU; }
    bool IsDebugMode() const { return debug; }
    bool IsPaused() const { return currentState == GameState::PAUSED; }
    bool IsVersusMode() const { return versus; }
    int GetMenuState() const { return menuState; }
    
    // Access to player management
    PlayerManager* GetPlayerManager() { return &playerManager; }
    const PlayerManager* GetPlayerManager() const { return &playerManager; }

private:
    friend class GameTaskStateTest;

    enum class GameState { MENU, PLAYING, PAUSED, GAME_OVER };
    GameState currentState;

    void HandleCommonState();
    void HandleMenuState();
    void HandlePlayingState();
    void HandlePausedState();
    void HandleGameOverState();

    void SetUpGame();
    void TransitionToState(GameState newState);

    void Visible(bool visible);

    // Phase 1: Add GameWorld alongside existing systems
    GameWorld gameWorld;
    PlayerManager playerManager;

    bool debug;
    bool gameOver;
    int menuState;
    bool versus;
    int timer;
};
