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
    
    
    videoTask->Start();
    inputTask->Start();
    graphicsTask->Start();
    
    soundTask->Start();
    gameTask->Start();
    
    gameTask->OnResume();
    
    quit=false;
    
    soundTask->PlayMusic(0);
    
    while(!quit)
    {
        
        videoTask->Update();
        inputTask->Update();
        graphicsTask->Update();
        soundTask->Update();
        if(!gameTask->paused) gameTask->Update();
        globalTimer->Update();
        
        //Take Input and events
        
        if(InputTask::KeyDown(SDLK_PAUSE))
        {
            gameTask->paused=!gameTask->paused;
        }
        
        if(inputTask->KeyDown(SDLK_ESCAPE))
        {
            quit=true;
        }
        
        
    }
    
    
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
    
}


int main(int argc, char *argv[])
{
    new App();
    App::GetSingleton().Run(argc, argv);
    delete App::GetSingletonPtr();
    
    return 0;
}

/*

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main( int argc, char* args[] )
{
    

    //The window we'll be rendering to
    SDL_Window* window = NULL;
    
    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;
    
    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    else
    {
        //Create window
        window = SDL_CreateWindow( "Tankgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get window surface
            screenSurface = SDL_GetWindowSurface( window );
            
            //Fill the surface white
            SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
            
            //Update the surface
            SDL_UpdateWindowSurface( window );
            
            //Wait two seconds
            SDL_Delay( 2000 );
        }
    }
    
    //Destroy window
    SDL_DestroyWindow( window );
    
    //Quit SDL subsystems
    SDL_Quit();
    
    
    return 0;
}
 */