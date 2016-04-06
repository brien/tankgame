
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
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
}

GameTask::~GameTask()
{
    
}

bool GameTask::Start()
{
    for(int i=0; i<4; i++)
    {
        App::GetSingleton().graphicsTask->cams[i].xpos=40.0;
        App::GetSingleton().graphicsTask->cams[i].ypos=60.0;
        App::GetSingleton().graphicsTask->cams[i].zpos=40.0;
        
        
        App::GetSingleton().graphicsTask->cams[i].xfocus=30.0;
        App::GetSingleton().graphicsTask->cams[i].yfocus=0;
        App::GetSingleton().graphicsTask->cams[i].zfocus=40.0;
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
            App::GetSingleton().soundTask->PlaySound(1);
        }
        
        
        if(InputTask::KeyDown(SDL_SCANCODE_2))
        {
            TankHandler::GetSingleton().numPlayers=2;
        }
        
        if(menuState==0)
        {
            if(InputTask::KeyDown(SDL_SCANCODE_RIGHT) || InputTask::GetAxis(0, 0) > 5000 )
            {
                menuState=1;
            }
        }
        if(menuState==1)
        {
            if(InputTask::KeyDown(SDL_SCANCODE_LEFT) || InputTask::GetAxis(0, 0) < -5000)
            {
                menuState=0;
            }
            if(InputTask::KeyDown(SDL_SCANCODE_DOWN) || InputTask::GetAxis(0,1) > 5000)
            {
                menuState=2;
            }
        }
        if(menuState==2)
        {
            if(InputTask::KeyDown(SDL_SCANCODE_LEFT) || InputTask::GetAxis(0, 0) < -5000 )
            {
                menuState=0;
            }
            if(InputTask::KeyDown(SDL_SCANCODE_UP) || InputTask::GetAxis(0,1) < -5000 )
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
        
        if(InputTask::KeyDown(SDL_SCANCODE_H) && debug)
        {
            LevelHandler::GetSingleton().NextLevel(true);
        }
        if(InputTask::KeyDown(SDL_SCANCODE_L) && debug)
        {
            LevelHandler::GetSingleton().NextLevel(false);
        }
        
        
        App::GetSingleton().graphicsTask->drawHUD=true;
        App::GetSingleton().graphicsTask->drawMenu=false;
        
        FXHandler::GetSingleton().NextFrame();
        TankHandler::GetSingleton().NextFrame();
        /*
         
         for(int i=0; i<TankHandler::GetSingleton().numPlayers; i++)
         {
         
         App::GetSingleton().graphicsTask->cams[i].xpos=(TankHandler::GetSingleton().players[i].x - xzcam*(float)cos((TankHandler::GetSingleton().players[i].ry+TankHandler::GetSingleton().players[i].rty)*DTR));
         App::GetSingleton().graphicsTask->cams[i].ypos=TankHandler::GetSingleton().players[i].y+ycam;
         App::GetSingleton().graphicsTask->cams[i].zpos=(TankHandler::GetSingleton().players[i].z - xzcam*(float)sin((TankHandler::GetSingleton().players[i].ry+TankHandler::GetSingleton().players[i].rty)*DTR));
         
         
         App::GetSingleton().graphicsTask->cams[i].xfocus=(TankHandler::GetSingleton().players[i].x + (float)cos((TankHandler::GetSingleton().players[i].ry+TankHandler::GetSingleton().players[i].rty)*DTR));
         App::GetSingleton().graphicsTask->cams[i].yfocus=TankHandler::GetSingleton().players[i].y+.3;
         App::GetSingleton().graphicsTask->cams[i].zfocus=(TankHandler::GetSingleton().players[i].z + (float)sin((TankHandler::GetSingleton().players[i].ry+TankHandler::GetSingleton().players[i].rty)*DTR));
         }*/
        /*
         if(TankHandler::GetSingleton().twop)
         {
         App::GetSingleton().graphicsTask->cam2.xpos=(TankHandler::GetSingleton().player2.x - xzcam*(float)cos((TankHandler::GetSingleton().player2.ry+TankHandler::GetSingleton().player2.rty)*DTR));
         App::GetSingleton().graphicsTask->cam2.ypos=TankHandler::GetSingleton().player2.y+ycam;
         App::GetSingleton().graphicsTask->cam2.zpos=(TankHandler::GetSingleton().player2.z - xzcam*(float)sin((TankHandler::GetSingleton().player2.ry+TankHandler::GetSingleton().player2.rty)*DTR));
         
         
         App::GetSingleton().graphicsTask->cam2.xfocus=(TankHandler::GetSingleton().player2.x + (float)cos((TankHandler::GetSingleton().player2.ry+TankHandler::GetSingleton().player2.rty)*DTR));
         App::GetSingleton().graphicsTask->cam2.yfocus=TankHandler::GetSingleton().player2.y+.3;
         App::GetSingleton().graphicsTask->cam2.zfocus=(TankHandler::GetSingleton().player2.z + (float)sin((TankHandler::GetSingleton().player2.ry+TankHandler::GetSingleton().player2.rty)*DTR));
         
         }*/
        if( !TankHandler::GetSingleton().players[0].alive && 0)
        {
            gameOver=true;
            App::GetSingleton().graphicsTask->drawHUD=false;
        }
        
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