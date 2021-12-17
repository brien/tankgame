
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
    gameStarted=false;
    versus=false;
    gameOver=false;
    menuState=0;
    timer = 0;
    paused = false;
    debug = false;
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
    App::GetSingleton().graphicsTask->drawHUD=false;
    
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
    static float ycam=0.8f;
    static float xzcam=1.0f;
    
    if(InputTask::KeyDown(SDL_SCANCODE_ESCAPE))
    {
        TaskHandler::GetSingleton().KillAllTasks();
    }
    
    if(!gameStarted)
    {
        App::GetSingleton().graphicsTask->drawMenu=true;
        FXHandler::GetSingleton().NextFrame();
        
        if ( InputTask::KeyDown(SDL_SCANCODE_RETURN) || InputTask::MouseDown(1) )
        {
            TankHandler::GetSingleton().isInputJoy=false;
            if(menuState>0)
            {
                TankHandler::GetSingleton().numPlayers=2;
            }
            if(menuState==1)
            {
                versus=false;
            }
            
            if(menuState==2)
            {
                versus=true;
            }
            
            SetUpGame();
            gameStarted=true;
        }
        
        if ( InputTask::GetButton(0, 0) || InputTask::GetButton(0, 1) )
        {
            TankHandler::GetSingleton().isInputJoy=true;
            if(menuState>0)
            {
                TankHandler::GetSingleton().numPlayers=2;
            }
            if(menuState==1)
            {
                versus=false;
            }
            if(menuState==2)
            {
                versus=true;
            }
            
            SetUpGame();
            gameStarted=true;
        }
        
        if ( InputTask::KeyDown(SDL_SCANCODE_I) )
        {
            debug=true;
            App::GetSingleton().soundTask->PlayChannel(1);
        }
        
        if(InputTask::KeyDown(SDL_SCANCODE_2))
        {
            TankHandler::GetSingleton().numPlayers=2;
        }
        
        if(menuState==0)
        {
            if(InputTask::KeyDown(SDL_SCANCODE_RIGHT) || InputTask::KeyDown(SDL_SCANCODE_D) || InputTask::GetAxis(0, 0) > 5000 )
            {
                menuState=1;
            }
        }
        if(menuState==1)
        {
            if(InputTask::KeyDown(SDL_SCANCODE_LEFT) || InputTask::KeyDown(SDL_SCANCODE_A) || InputTask::GetAxis(0, 0) < -5000)
            {
                menuState=0;
            }
            if(InputTask::KeyDown(SDL_SCANCODE_DOWN) || InputTask::KeyDown(SDL_SCANCODE_S) || InputTask::GetAxis(0,1) > 5000)
            {
                menuState=2;
            }
        }
        if(menuState==2)
        {
            if(InputTask::KeyDown(SDL_SCANCODE_LEFT) || InputTask::KeyDown(SDL_SCANCODE_A) || InputTask::GetAxis(0, 0) < -5000 )
            {
                menuState=0;
            }
            if(InputTask::KeyDown(SDL_SCANCODE_UP) || InputTask::KeyDown(SDL_SCANCODE_W) || InputTask::GetAxis(0,1) < -5000 )
            {
                menuState=1;
            }
        }
        
    }
    
    if(InputTask::KeyDown(SDL_SCANCODE_M))
    {
        App::GetSingleton().soundTask->PauseMusic();
    }
    
    
    if(gameStarted && !gameOver)
    {
        if(InputTask::KeyDown(SDL_SCANCODE_2))
        {
            TankHandler::GetSingleton().numPlayers=2;
        }
        if(InputTask::KeyDown(SDL_SCANCODE_1))
        {
            ycam=.8;
            xzcam=1;
        }
        if(InputTask::KeyDown(SDL_SCANCODE_C))
        {
            ycam=1.2;
            xzcam=2;
        }
        if(InputTask::KeyDown(SDL_SCANCODE_3))
        {
            ycam=20.2;
            xzcam=0.2;
        }
        
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
        
        App::GetSingleton().graphicsTask->drawHUD=true;
        App::GetSingleton().graphicsTask->drawMenu=false;
        
        FXHandler::GetSingleton().NextFrame();
        TankHandler::GetSingleton().NextFrame();
    }
    
    if(gameOver)
    {
        GameOver();
    }
}

void GameTask::GameOver()
{
    
}

void GameTask::DisplayScore()
{
    
}

void GameTask::DisplayTime()
{
    
}

void GameTask::Stop()
{
    
}