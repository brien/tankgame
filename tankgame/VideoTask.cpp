#include "VideoTask.h"
#include "TankHandler.h"
#include "App.h"

#include <SDL2/SDL.h>
#include <iostream>


int VideoTask::scrWidth=800;
int VideoTask::scrHeight=600;
int VideoTask::scrBPP=32;

VideoTask::VideoTask()
{
    
}

VideoTask::~VideoTask()
{
    
}


bool VideoTask::Start()
{
    
    if(-1==SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK))
    {
        std::cerr << "VideoTask::Start: SDL_InitSubSystem failed." << std::endl;
        return false;
    }
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1 );
    SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 1 );
    
    char line[64];
    FILE *filein;
    filein=fopen("settings.txt", "rt");
    if(!filein)
    {
        std::cerr << "VideoTask::Start: Failed to open settings.txt" << std::endl;
        std::cerr << "VideoTask::Start: Using default settings" << std::endl;
        
        flags = SDL_WINDOW_OPENGL;
        
        scrWidth=1280;
        scrHeight=720;
        
        TankHandler::GetSingleton().diff=0;
        App::GetSingleton().soundTask->disable=false;
    }
    else
    {
        fgets(line, 64, filein);
        fgets(line, 64, filein);
        
        if(line[0]=='0')
        {
            flags = SDL_WINDOW_OPENGL;
            //hide the mouse cursor
            //SDL_ShowCursor(SDL_DISABLE);
        }
        else
        {
            flags = SDL_WINDOW_OPENGL|SDL_WINDOW_FULLSCREEN;
            //hide the mouse cursor
            SDL_ShowCursor(SDL_DISABLE);
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        
        fgets(line, 64, filein);
        fgets(line, 64, filein);
        
        scrWidth=(line[0]-48)*100;
        scrWidth+=(line[1]-48)*10;
        scrWidth+=(line[2]-48);
        
        if(line[3]>=48 && line[3]<=57)
        {
            scrWidth*=10;
            scrWidth+=(line[3]-48);
        }
        
        fgets(line, 64, filein);
        fgets(line, 64, filein);
        
        scrHeight=(line[0]-48)*100;
        scrHeight+=(line[1]-48)*10;
        scrHeight+=(line[2]-48);
        
        if(line[3]>=48 && line[3]<=57)
        {
            scrHeight*=10;
            scrHeight+=(line[3]-48);
        }
        
        fgets(line, 64, filein);
        fgets(line, 64, filein);
        
        TankHandler::GetSingleton().diff=(line[0]-48);
        
        fgets(line, 64, filein);
        fgets(line, 64, filein);
        
        App::GetSingleton().soundTask->disable=(bool)(line[0]-48);
    }
    
    fclose(filein);
    
    window = SDL_CreateWindow("tankgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, scrWidth, scrHeight, flags);
    //sdlRenderer = SDL_CreateRenderer(window, -1, 0);
    glContext = SDL_GL_CreateContext(window);
    
    SDL_GL_SetSwapInterval( 1 );
    
    if(!window)
    {
        std::cerr << "VideoTask::Start: SDL_CreateWindow failed." << std::endl;
        return false;
    }
    
    
    return true;
}

void VideoTask::Update()
{
    
    SDL_GL_SwapWindow(window);
}

void VideoTask::Stop()
{
    
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow( window );
}

