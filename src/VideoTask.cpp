#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include "VideoTask.h"
#include "TankHandler.h"
#include "App.h"
#include "rendering/GLFunctions.h"
#include "rendering/GraphicsCapabilities.h"
#include "rendering/RendererMode.h"

#include <SDL2/SDL.h>
#include <iostream>

int VideoTask::scrWidth = 800;
int VideoTask::scrHeight = 600;
int VideoTask::scrBPP = 32;
int VideoTask::difficultySetting = 1; // Default to normal difficulty

VideoTask::VideoTask()
{
}

bool VideoTask::Start()
{

    if (-1 == SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK))
    {
        Logger::Get().Write("VideoTask::Start Failed:\n", SDL_GetError());
        Logger::Get().Write("VideoTask::Start: SDL_InitSubSystem failed.\n");
        return false;
    }
    GraphicsPlatform platform = GraphicsPlatform::Linux;
#ifdef __EMSCRIPTEN__
    platform = GraphicsPlatform::Web;
#elif defined(__APPLE__)
    platform = GraphicsPlatform::MacOS;
#elif defined(_WIN32)
    platform = GraphicsPlatform::Windows;
#endif
    const ContextRequest context = ChooseContextRequest(platform, RendererMode::IsModern());
    if (!context.supported)
    {
        Logger::Get().Write("VideoTask::Start: compatibility rendering is unsupported on macOS; use TANKGAME_RENDERER=modern.\n");
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, context.major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, context.minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        context.profile == GraphicsProfile::ES2 ? SDL_GL_CONTEXT_PROFILE_ES :
        context.profile == GraphicsProfile::Core32 ? SDL_GL_CONTEXT_PROFILE_CORE :
        SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 1);

    char line[64];
    char const *settingsFilename = "settings.txt";

    FILE *filein;

    filein = fopen(settingsFilename, "rt");

    if (!filein)
    {
        Logger::Get().Write("VideoTask::Start: Failed to open settings.txt\n");
        Logger::Get().Write("VideoTask::Start: Using default settings\n");

        flags = SDL_WINDOW_OPENGL;

        scrWidth = 1280;
        scrHeight = 720;

        VideoTask::difficultySetting = 0;
#ifndef __EMSCRIPTEN__
        App::GetSingleton().soundTask->disable = false;
#endif
    }
    else
    {
        fgets(line, 64, filein);
        fgets(line, 64, filein);

        if (line[0] == '0')
        {
            flags = SDL_WINDOW_OPENGL;
        }
        else
        {
            flags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;
        }

#ifndef __EMSCRIPTEN__
        SDL_ShowCursor(SDL_DISABLE);
        SDL_SetRelativeMouseMode(SDL_TRUE);
#endif

        fgets(line, 64, filein);
        fgets(line, 64, filein);

        scrWidth = (line[0] - 48) * 100;
        scrWidth += (line[1] - 48) * 10;
        scrWidth += (line[2] - 48);

        if (line[3] >= 48 && line[3] <= 57)
        {
            scrWidth *= 10;
            scrWidth += (line[3] - 48);
        }

        fgets(line, 64, filein);
        fgets(line, 64, filein);

        scrHeight = (line[0] - 48) * 100;
        scrHeight += (line[1] - 48) * 10;
        scrHeight += (line[2] - 48);

        if (line[3] >= 48 && line[3] <= 57)
        {
            scrHeight *= 10;
            scrHeight += (line[3] - 48);
        }

        fgets(line, 64, filein);
        fgets(line, 64, filein);

        VideoTask::difficultySetting = (line[0] - 48);

        fgets(line, 64, filein);
        fgets(line, 64, filein);

#ifndef __EMSCRIPTEN__
        App::GetSingleton().soundTask->disable = (bool)(line[0] - 48);
#endif
        fclose(filein);
    }

    window = SDL_CreateWindow("tankgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, scrWidth, scrHeight, flags);
    if (!window)
    {
        Logger::Get().Write("VideoTask::Start: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        Logger::Get().Write("VideoTask::Start: WebGL/OpenGL context creation failed: %s\n", SDL_GetError());
        return false;
    }

    if (SDL_GL_MakeCurrent(window, glContext) != 0)
    {
        Logger::Get().Write("VideoTask::Start: context activation failed: %s\n", SDL_GetError());
        return false;
    }

    if (RendererMode::IsModern())
    {
        std::string loaderError;
        if (!GLFunctions::Initialize(loaderError))
        {
            Logger::Get().Write("VideoTask::Start: %s\n", loaderError.c_str());
            SDL_GL_DeleteContext(glContext);
            glContext = nullptr;
            SDL_DestroyWindow(window);
            window = nullptr;
            return false;
        }
    }

    SDL_GL_SetSwapInterval(1);
    Logger::Get().Write("VideoTask::Start: graphics context ready at %dx%d.\n", scrWidth, scrHeight);

    return true;
}

void VideoTask::Update()
{
    SDL_GL_SwapWindow(window);
}

void VideoTask::Stop()
{
    GLFunctions::Shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
