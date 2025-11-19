#include <iostream>
#include "App.h"
#include "GameTask.h"
#include "InputTask.h"
#include "LevelHandler.h"
#include "TankHandler.h"
#include "FXHandler.h"
#include "GameWorld.h"
#include "events/Events.h"

void GameTask::SetUpGame()
{
    LevelHandler::GetSingleton().Init();
    if (!LevelHandler::GetSingleton().Load("levels/level0@@.txt"))
    {
        std::cerr << "LevelHandler failed to load level." << std::endl;
    }
    TankHandler::GetSingleton().Init();
}

GameTask::GameTask()
{
    gameStarted = false;
    versus = false;
    gameOver = false;
    menuState = 0;
    timer = 0;
    paused = false;
    debug = false;
    currentState = GameState::MENU;
    SDL_SetRelativeMouseMode(SDL_TRUE);
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
    App::GetSingleton().graphicsTask->drawHUD = false;

    LevelHandler::GetSingleton().Load("./levels/title@@.txt");

    paused = false;
    debug = false;
    
    // Initialize GameWorld with collision and combat systems
    gameWorld.Initialize();
    
    // Connect handlers to GameWorld (singletons are now initialized)
    FXHandler::GetSingleton().SetGameWorld(&gameWorld);
    BulletHandler::GetSingleton().SetGameWorld(&gameWorld);
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
    TankHandler::GetSingleton().Init();
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
    case GameState::GAME_OVER:
        HandleGameOverState();
        break;
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
    App::GetSingleton().graphicsTask->drawMenu = true;
    // Update GameWorld for menu effects (now handles FX through GameWorld)
    gameWorld.Update();

    if (InputTask::KeyDown(SDL_SCANCODE_RETURN) || InputTask::MouseDown(1))
    {
        TankHandler::GetSingleton().isInputJoy = false;
        if (menuState > 0)
        {
            TankHandler::GetSingleton().numPlayers = 2;
        }
        versus = (menuState == 2);
        SetUpGame();
        gameStarted = true;
        TransitionToState(GameState::PLAYING);
    }

    if (InputTask::KeyDown(SDL_SCANCODE_I))
    {
        debug = true;
        App::GetSingleton().soundTask->PlayChannel(1);
    }

    if (InputTask::KeyDown(SDL_SCANCODE_2))
    {
        TankHandler::GetSingleton().numPlayers = 2;
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
    if (debug)
    {
        if (InputTask::KeyDown(SDL_SCANCODE_H))
        {
            LevelHandler::GetSingleton().NextLevel(true);
        }
        if (InputTask::KeyDown(SDL_SCANCODE_L))
        {
            LevelHandler::GetSingleton().NextLevel(false);
        }
    }

    App::GetSingleton().graphicsTask->drawHUD = true;
    App::GetSingleton().graphicsTask->drawMenu = false;

    // Process events first (handles collision queries, notifications, etc.)
    Events::ProcessQueuedEvents();
    
    // New unified update replaces individual handler updates
    gameWorld.Update();
    
    // Keep BulletHandler cleanup for transition safety
    BulletHandler::GetSingleton().NextFrame();
    // FXHandler now delegates to GameWorld, so NextFrame() is effectively a no-op

    if (InputTask::KeyDown(SDL_SCANCODE_ESCAPE))
    {
        TransitionToState(GameState::MENU);
    }

    if (gameOver)
    {
        TransitionToState(GameState::GAME_OVER);
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
    currentState = newState;
}