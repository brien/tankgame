//
//  main.cpp
//  tankgame
//
//
#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#include <mach-o/dyld.h>
#else
// If building on Linux:
#include <GL/gl.h>
#endif

#include <iostream>
#include <algorithm>
#include <SDL2/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "App.h"
#include "Singleton.h"
#include "InputTask.h"
#include "GlobalTimer.h"

#include "TankHandler.h"
#include "LevelHandler.h"

bool App::Initialize(int argc, char *argv[])
{
    if (!Logger::Get().Init())
        return false;

    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    Logger::Get().Write("We compiled against SDL version %d.%d.%d ...\n",
           compiled.major, compiled.minor, compiled.patch);
    Logger::Get().Write("But we are linking against SDL version %d.%d.%d.\n",
           linked.major, linked.minor, linked.patch);

    videoTask = new VideoTask;
    graphicsTask = new GraphicsTask;
    soundTask = new SoundTask;
#ifdef __EMSCRIPTEN__
    // Browser audio requires a user gesture. It is not a startup dependency for
    // the bootstrap renderer, so leave it disabled until a later audio milestone.
    soundTask->disable = true;
#endif
    gameTask = new GameTask;
    globalTimer = new GlobalTimer;
    inputTask = new InputTask;

    new TankHandler();
    new LevelHandler();

    new TaskHandler();

    videoTask->priority = 100;
    if (!TaskHandler::GetSingleton().AddTask(videoTask))
    {
        Logger::Get().Write("FATAL: VideoTask failed to initialize.\n");
        return false;
    }

    inputTask->priority = 90;
    if (!TaskHandler::GetSingleton().AddTask(inputTask))
    {
        Logger::Get().Write("FATAL: InputTask failed to initialize.\n");
        return false;
    }

    graphicsTask->priority = 80;
    if (!TaskHandler::GetSingleton().AddTask(graphicsTask))
    {
        Logger::Get().Write("FATAL: GraphicsTask failed to initialize.\n");
        return false;
    }

    soundTask->priority = 70;
    if (!TaskHandler::GetSingleton().AddTask(soundTask))
    {
        Logger::Get().Write("FATAL: SoundTask failed to initialize.\n");
        return false;
    }

    gameTask->priority = 60;
    if (!TaskHandler::GetSingleton().AddTask(gameTask))
    {
        Logger::Get().Write("FATAL: GameTask failed to initialize.\n");
        return false;
    }

    globalTimer->priority = 10;
    if (!TaskHandler::GetSingleton().AddTask(globalTimer))
    {
        Logger::Get().Write("FATAL: GlobalTimer failed to initialize.\n");
        return false;
    }

    gameTask->OnResume();

    quit = false;

#ifndef __EMSCRIPTEN__
    soundTask->PlayMusic(0);
#endif

    Logger::Get().Write("Initialization complete.\n");
    return true;
}

bool App::Tick()
{
    return TaskHandler::GetSingleton().Tick();
}

void App::Shutdown()
{
    TaskHandler::GetSingleton().Execute();

    delete TankHandler::GetSingletonPtr();
    delete LevelHandler::GetSingletonPtr();

    delete TaskHandler::GetSingletonPtr();
}

void App::Run(int argc, char *argv[])
{
    if (!Initialize(argc, argv))
        return;

    Logger::Get().Write("Entering native TaskHandler execute loop.\n");
    TaskHandler::GetSingleton().Execute();
    Shutdown();
}

#ifdef __EMSCRIPTEN__
namespace
{
void BrowserFrame(void *appPointer)
{
    // Callback spacing includes browser scheduling/presentation; Tick time is
    // CPU submission time, not GPU execution time. Report only once a second.
    static double previousStart = 0.0;
    static double elapsed = 0.0;
    static double tickTime = 0.0;
    static double maxFrame = 0.0;
    static unsigned frames = 0;
    const double start = emscripten_get_now();
    const double frameTime = previousStart > 0.0 ? start - previousStart : 0.0;
    previousStart = start;
    App *app = static_cast<App *>(appPointer);
    static bool firstFrame = true;
    if (!app->Tick())
    {
        Logger::Get().Write("Browser frame loop stopped.\n");
        emscripten_cancel_main_loop();
        app->Shutdown();
        return;
    }

    if (frameTime > 0.0)
    {
        elapsed += frameTime;
        tickTime += emscripten_get_now() - start;
        maxFrame = std::max(maxFrame, frameTime);
        ++frames;
        if (elapsed >= 1000.0)
        {
            Logger::Get().Write("Browser perf: fps=%.2f avg_ms=%.3f recent_ms=%.3f max_ms=%.3f tick_ms=%.3f frames=%u interval_ms=%.3f\n",
                frames * 1000.0 / elapsed, elapsed / frames, frameTime,
                maxFrame, tickTime / frames, frames, elapsed);
            elapsed = tickTime = maxFrame = 0.0;
            frames = 0;
        }
    }

    if (firstFrame)
    {
        firstFrame = false;
        Logger::Get().Write("Browser first frame completed: WebGL clear/render/swap path executed.\n");
    }
}
}
#endif

int main(int argc, char *argv[])
{
    new App();
#ifdef __EMSCRIPTEN__
    if (!App::GetSingleton().Initialize(argc, argv))
    {
        Logger::Get().Write("FATAL: Browser application initialization failed.\n");
        return 1;
    }
    Logger::Get().Write("Registering non-blocking Emscripten browser frame loop.\n");
    emscripten_set_main_loop_arg(BrowserFrame, App::GetSingletonPtr(), 0, true);
#else
    App::GetSingleton().Run(argc, argv);
    delete App::GetSingletonPtr();
#endif

    return 0;
}
