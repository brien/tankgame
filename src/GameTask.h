#pragma once

#include "ITask.h"
#include "GameWorld.h"

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
    bool IsGameStarted() const { return gameStarted; }
    bool IsDebugMode() const { return debug; }
    bool IsPaused() const { return paused; }
    bool IsVersusMode() const { return versus; }
    int GetMenuState() const { return menuState; }
    
    // GameWorld access
    GameWorld& GetGameWorld() { return gameWorld; }
    const GameWorld& GetGameWorld() const { return gameWorld; }

private:
    enum class GameState { MENU, PLAYING, GAME_OVER };
    GameState currentState;

    void HandleCommonState();
    void HandleMenuState();
    void HandlePlayingState();
    void HandleGameOverState();

    void SetUpGame();
    void TransitionToState(GameState newState);

    void Visible(bool visible);

    // Phase 1: Add GameWorld alongside existing systems
    GameWorld gameWorld;

    bool paused;
    bool debug;
    bool gameStarted;
    bool gameOver;
    int menuState;
    bool versus;
    int timer;
};
