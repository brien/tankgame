#include <iostream>
#include "App.h"
#include "GameTask.h"
#include "InputTask.h"
#include "LevelHandler.h"
#include "TankHandler.h"
#include "FXHandler.h"

void GameTask::SetUpGame()
{
    LevelHandler::GetSingleton().Init();
    if(!LevelHandler::GetSingleton().Load("levels/level0@@.txt"))
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
    for(int i=0; i<4; i++)
    {
        App::GetSingleton().graphicsTask->cams[i].SetPos(40.0, 60.0, 40.0);
        App::GetSingleton().graphicsTask->cams[i].SetFocus(30.0, 0.0, 40.0);
    }
    App::GetSingleton().graphicsTask->drawHUD = false;
    
    LevelHandler::GetSingleton().Load("./levels/title@@.txt");
    
    paused = false;
    debug = false;
    
    return true;
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

void GameTask::HandleMenuState()
{
    App::GetSingleton().graphicsTask->drawMenu = true;
    FXHandler::GetSingleton().NextFrame();

    if (InputTask::KeyDown(SDL_SCANCODE_RETURN) || InputTask::MouseDown(1))
    {
        TankHandler::GetSingleton().isInputJoy = false;
        if (menuState > 0)
        {
            TankHandler::GetSingleton().numPlayers = 2;
        }
        versus = (menuState == 2);
        SetUpGame();
        TransitionToState(GameState::PLAYING);
    }

    if (InputTask::KeyDown(SDL_SCANCODE_ESCAPE))
    {
        TaskHandler::GetSingleton().KillAllTasks();
    }
}

void GameTask::HandlePlayingState()
{
    App::GetSingleton().graphicsTask->drawHUD = true;
    App::GetSingleton().graphicsTask->drawMenu = false;

    FXHandler::GetSingleton().NextFrame();
    TankHandler::GetSingleton().NextFrame();
    BulletHandler::GetSingleton().NextFrame();

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
    GameOver();

    if (InputTask::KeyDown(SDL_SCANCODE_RETURN))
    {
        TransitionToState(GameState::MENU);
    }
}

void GameTask::TransitionToState(GameState newState)
{
    currentState = newState;
}