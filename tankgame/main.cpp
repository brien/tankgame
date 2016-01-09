//
//  main.cpp
//  tankgame
//
//  Created by Brien Smith Martínez on 3/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#include <iostream>
#include <SDL2/SDL.h>
#include <OpenGL/gl.h>

#include "App.h"
#include "Singleton.h"
#include "InputTask.h"
#include "GlobalTimer.h"

#include "TankHandler.h"
#include "LevelHandler.h"
#include "FXHandler.h"

void App::Run(int argc, char *argv[])
{
    videoTask = new VideoTask;
    graphicsTask = new GraphicsTask;
    soundTask = new SoundTask;
    gameTask = new GameTask;
    globalTimer = new GlobalTimer;
    inputTask = new InputTask;
    
    new TankHandler();
    new LevelHandler();
    new FXHandler();
    
    new TaskHandler();
    
    videoTask->priority = 100;
    TaskHandler::GetSingleton().AddTask(videoTask);
    
    inputTask->priority = 90;
    TaskHandler::GetSingleton().AddTask(inputTask);
    
    graphicsTask->priority = 80;
    TaskHandler::GetSingleton().AddTask(graphicsTask);
    
    soundTask->priority = 70;
    TaskHandler::GetSingleton().AddTask(soundTask);
    
    gameTask->priority = 60;
    TaskHandler::GetSingleton().AddTask(gameTask);
    
    globalTimer->priority = 10;
    TaskHandler::GetSingleton().AddTask(globalTimer);
    
    //videoTask->Start();
    //inputTask->Start();
    //graphicsTask->Start();
    
    //soundTask->Start();
    //gameTask->Start();
    
    gameTask->OnResume();
    
    quit=false;
    
    soundTask->PlayMusic(0);
    
    TaskHandler::GetSingleton().Execute();
    
    /*
    while(!quit)
    {
        videoTask->Update();
        inputTask->Update();
        graphicsTask->Update();
        soundTask->Update();
        if(!gameTask->paused) gameTask->Update();
        globalTimer->Update();
        
        //Take Input and events
        
        if(InputTask::KeyDown(SDL_SCANCODE_PAUSE))
        {
            gameTask->paused=!gameTask->paused;
        }
        
        if(inputTask->KeyDown(SDL_SCANCODE_ESCAPE))
        {
            quit=true;
        }
    }*/
    
    
    inputTask->Stop();
    soundTask->Stop();
    gameTask->Stop();
    globalTimer->Stop();
    
    graphicsTask->Stop();
    videoTask->Stop();
    
    
    delete videoTask;
    delete graphicsTask;
    delete inputTask;
    delete soundTask;
    delete gameTask;
    delete globalTimer;
    
    delete TankHandler::GetSingletonPtr();
    delete LevelHandler::GetSingletonPtr();
    delete FXHandler::GetSingletonPtr();
    
    delete TaskHandler::GetSingletonPtr();
    
}


int main(int argc, char *argv[])
{
    new App();
    App::GetSingleton().Run(argc, argv);
    delete App::GetSingletonPtr();
    
    return 0;
}