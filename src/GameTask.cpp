#include <iostream>
#include "App.h"
#include "GlobalTimer.h"
#include "GameTask.h"
#include "InputTask.h"
#include "LevelHandler.h"
#include "TankHandler.h"
#include "GameWorld.h"
#include "PlayerManager.h"
#include "Logger.h"
#include "events/Events.h"

void GameTask::SetUpGame()
{
    Logger::Get().Write("GameTask: Setting up new game...\n");
    
    // Clear any pending events from previous level/game
    Events::Clear();
    Logger::Get().Write("GameTask: Cleared event queue\n");
    
    playerManager.ReleaseTanks();
    gameWorld.Clear();
    versus = (menuState == 2);
    playerManager.ResetSession(menuState == 0 ? 1 : 2, versus);
    gameOver = false;
    timer = 0;

    LevelHandler::GetSingleton().SetPlayerManager(&playerManager);
    LevelHandler::GetSingleton().Init();
    if (!LevelHandler::GetSingleton().Load("levels/level0@@.txt"))
    {
        Logger::Get().Write("LevelHandler failed to load level.\n");
    }
    TankHandler::GetSingleton().Init();
    
    // Spawn player tanks through PlayerManager
    Logger::Get().Write("GameTask: Spawning player tanks via PlayerManager...\n");
    playerManager.SpawnPlayerTanks();
    Logger::Get().Write("GameTask: Player tanks spawned\n");
    
    Logger::Get().Write("GameTask: Game setup complete\n");
}

GameTask::GameTask()
{
    versus = false;
    gameOver = false;
    menuState = 0;
    timer = 0;
    debug = false;
    currentState = GameState::MENU;
#ifndef __EMSCRIPTEN__
    SDL_SetRelativeMouseMode(SDL_TRUE);
#endif
}

GameTask::~GameTask()
{
}

bool GameTask::Start()
{
    for (int i = 0; i < 4; i++)
    {
        App::GetSingleton().graphicsTask->cams[i].SetPos(40.0, 60.0, 40.0);
        App::GetSingleton().graphicsTask->cams[i].SetFocus(30.0, 0.0, 40.0);
    }

    LevelHandler::GetSingleton().Load("./levels/title@@.txt");

    debug = false;
    
    // Initialize GameWorld with collision and combat systems
    gameWorld.Initialize();
    
    // Pass GameWorld to GraphicsTask and recreate SceneDataBuilder
    // (GraphicsTask starts before GameTask, so initial creation had nullptr gameWorld)
    App::GetSingleton().graphicsTask->SetGameWorld(&gameWorld);
    
    // Connect handlers to GameWorld (singletons are now initialized)
    TankHandler::GetSingleton().SetGameWorld(&gameWorld);
    LevelHandler::GetSingleton().SetGameWorld(&gameWorld);

    return true;
}

void GameTask::Stop()
{
    gameWorld.Shutdown();
}

void GameTask::Visible(bool visible)
{
}

void GameTask::OnResume()
{
    Logger::Get().Write("GameTask: OnResume - Initializing systems...\n");
    TankHandler::GetSingleton().Init();
    Logger::Get().Write("GameTask: Initializing PlayerManager...\n");
    playerManager.Initialize(&gameWorld);
    
    Logger::Get().Write("GameTask: OnResume complete\n");
}

void GameTask::OnSuspend()
{
    Visible(false);
}

void GameTask::Update()
{
    HandleCommonState();
     
    switch (currentState)
    {
    case GameState::MENU:
        HandleMenuState();
        break;
    case GameState::PLAYING:
        HandlePlayingState();
        break;
    case GameState::PAUSED:
        HandlePausedState();
        break;
    case GameState::GAME_OVER:
        HandleGameOverState();
        break;
    }

    // Debug output (only occasionally to avoid spam)
    static int debugCounter = 0;
    
    if (++debugCounter % 60 == 0) { // Log every 60 frames (~1 second at 60fps)
        Logger::Get().Write("GameWorld entities - Tanks: %zu, Bullets: %zu, Items: %zu, Effects: %zu | FPS: %.1f\n", 
                           gameWorld.GetTanks().size(), gameWorld.GetBullets().size(), 
                           gameWorld.GetItems().size(), gameWorld.GetFX().size(), GlobalTimer::GetFPS());
    }
}

void GameTask::HandleCommonState()
{
    if (InputTask::KeyDown(SDL_SCANCODE_M))
    {
        App::GetSingleton().soundTask->PauseMusic();
    }
}

void GameTask::HandleMenuState()
{
    // Menu frames do not advance the simulation.

    if (InputTask::KeyDown(SDL_SCANCODE_RETURN) || InputTask::MouseDown(1))
    {
        // isInputJoy managed by PlayerManager
        playerManager.SetInputJoystick(false);
        SetUpGame();
        TransitionToState(GameState::PLAYING);
        return;
    }

    if (InputTask::KeyDown(SDL_SCANCODE_I))
    {
        debug = true;
        App::GetSingleton().soundTask->PlayChannel(1);
    }

    if (InputTask::KeyDown(SDL_SCANCODE_2))
    {
        menuState = 1;
    }

    if (menuState == 0)
    {
        if (InputTask::KeyDown(SDL_SCANCODE_RIGHT) || InputTask::KeyDown(SDL_SCANCODE_D) || InputTask::GetAxis(0, 0) > 5000)
        {
            menuState = 1;
        }
    }
    if (menuState == 1)
    {
        if (InputTask::KeyDown(SDL_SCANCODE_LEFT) || InputTask::KeyDown(SDL_SCANCODE_A) || InputTask::GetAxis(0, 0) < -5000)
        {
            menuState = 0;
        }
        if (InputTask::KeyDown(SDL_SCANCODE_DOWN) || InputTask::KeyDown(SDL_SCANCODE_S) || InputTask::GetAxis(0, 1) > 5000)
        {
            menuState = 2;
        }
    }
    if (menuState == 2)
    {
        if (InputTask::KeyDown(SDL_SCANCODE_LEFT) || InputTask::KeyDown(SDL_SCANCODE_A) || InputTask::GetAxis(0, 0) < -5000)
        {
            menuState = 0;
        }
        if (InputTask::KeyDown(SDL_SCANCODE_UP) || InputTask::KeyDown(SDL_SCANCODE_W) || InputTask::GetAxis(0, 1) < -5000)
        {
            menuState = 1;
        }
    }

    if (InputTask::KeyDown(SDL_SCANCODE_ESCAPE))
    {
        TaskHandler::GetSingleton().KillAllTasks();
    }
}

void GameTask::HandlePlayingState()
{
    // Pause before processing events, movement, or combat this frame.
    if (InputTask::KeyDown(SDL_SCANCODE_ESCAPE))
    {
        TransitionToState(GameState::PAUSED);
        return;
    }

    if (debug)
    {
        if (InputTask::KeyDown(SDL_SCANCODE_H))
        {
            LevelHandler::GetSingleton().RequestNextLevel(true);
        }
        if (InputTask::KeyDown(SDL_SCANCODE_L))
        {
            LevelHandler::GetSingleton().RequestNextLevel(false);
        }
    }

    // Process events first (handles collision queries, notifications, etc.)
    Events::ProcessQueuedEvents();
    
    // New unified update replaces individual handler updates
    gameWorld.Update();
    
    // Player management through PlayerManager
    playerManager.NextFrame();
    
    // Item management (TODO: move to GameWorld or ItemManager)
    LevelHandler::GetSingleton().UpdateItems();
    LevelHandler::GetSingleton().ItemCollision();

    if (!gameOver) LevelHandler::GetSingleton().ProcessPendingLevelChange();

    if (gameOver)
    {
        TransitionToState(GameState::GAME_OVER);
    }
}

void GameTask::HandlePausedState()
{
    if (InputTask::KeyDown(SDL_SCANCODE_ESCAPE))
    {
        TransitionToState(GameState::MENU);
    }
    else if (InputTask::KeyDown(SDL_SCANCODE_RETURN))
    {
        TransitionToState(GameState::PLAYING);
    }
}

void GameTask::HandleGameOverState()
{
    if (InputTask::KeyDown(SDL_SCANCODE_RETURN))
    {
        TransitionToState(GameState::MENU);
    }
}

void GameTask::TransitionToState(GameState newState)
{
    if (newState == GameState::MENU && currentState != GameState::MENU)
    {
        // Drop events holding entity pointers before destroying the old world.
        Events::Clear();
        playerManager.ReleaseTanks();
        gameWorld.Clear();
        playerManager.ResetSession(1, false);
        versus = false;
        gameOver = false;
        timer = 0;

        auto& level = LevelHandler::GetSingleton();
        level.Init(); // Also cancels deferred level changes.
        if (!level.Load("levels/title@@.txt"))
        {
            Logger::Get().Write("LevelHandler failed to load title level.\n");
        }
    }
    currentState = newState;
}
