//
//  main.cpp
//  tankgame
//
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
    if(!Logger::Get().Init())return;

    SDL_version compiled;
    SDL_version linked;
    
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    printf("We compiled against SDL version %d.%d.%d ...\n",
           compiled.major, compiled.minor, compiled.patch);
    printf("But we are linking against SDL version %d.%d.%d.\n",
           linked.major, linked.minor, linked.patch);
    

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

    gameTask->OnResume();
    
    quit=false;
    
    soundTask->PlayMusic(0);
    
    Logger::Get().Write("Initialization complete. About to enter TaskHandler Execute Loop. \n");
    TaskHandler::GetSingleton().Execute();
    
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
