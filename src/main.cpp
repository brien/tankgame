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
#include "BrowserFramePolicy.h"
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

#ifdef __EMSCRIPTEN__
    soundTask->SetActive(false);
#else
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
extern "C" EMSCRIPTEN_KEEPALIVE void ActivateBrowserAudio()
{
    App::GetSingleton().soundTask->SetActive(true);
    App::GetSingleton().soundTask->PlayMusic(0);
}

namespace
{
// Use independent listeners: replacing Emscripten's focus callbacks would
// interfere with SDL's own keyboard/window bookkeeping.
EM_JS(void, InstallBrowserActivity, (), {
    const state = {changed: true, disposed: false};
    const update = () => {
        state.changed = true;
        if (Module.SDL2 && Module.SDL2.audioContext) {
            const active = !document.hidden && document.hasFocus() && !state.contextLost;
            const request = active && state.audioEnabled ? Module.SDL2.audioContext.resume() : Module.SDL2.audioContext.suspend();
            if (request && request.catch) request.catch(() => {});
        }
    };
    document.addEventListener('visibilitychange', update);
    window.addEventListener('focus', update);
    window.addEventListener('blur', update);
    const status = document.createElement('div');
    status.setAttribute('role', 'status');
    status.style.cssText = 'position:fixed;top:12px;left:50%;transform:translateX(-50%);padding:12px 20px;background:#111;color:#fff;font:16px sans-serif;z-index:1000;pointer-events:none';
    status.textContent = 'Paused — return to this window to resume';
    status.hidden = true;
    document.body.appendChild(status);
    state.status = status;
    const canvas = Module.canvas;
    const stage = canvas.parentElement;
    // Fullscreen the game surface so pause and input controls stay accessible.
    stage.id = 'tankgame-surface';
    stage.appendChild(status);
    const style = document.createElement('style');
    style.textContent = '#tankgame-surface:fullscreen { border:0; background:#000; } #tankgame-surface:fullscreen canvas { margin:0; }';
    document.head.appendChild(style);
    const shellControls = document.getElementById('controls');
    const shellDisplay = shellControls ? shellControls.style.display : '';
    if (shellControls) shellControls.style.display = 'none';
    const fullscreen = document.createElement('button');
    fullscreen.type = 'button';
    fullscreen.id = 'tankgame-fullscreen';
    fullscreen.setAttribute('aria-live', 'polite');
    fullscreen.style.cssText = 'position:fixed;bottom:12px;right:12px;padding:12px 20px;z-index:1000;background:#111;color:#fff';
    const refreshFullscreen = () => {
        fullscreen.textContent = document.fullscreenElement === stage ? 'Exit fullscreen' : 'Enter fullscreen';
        fullscreen.hidden = document.hidden || !document.hasFocus();
    };
    const fullscreenFailed = () => {
        if (state.disposed) return;
        refreshFullscreen();
        fullscreen.textContent = 'Fullscreen unavailable — retry';
    };
    const fullscreenChanged = () => {
        state.changed = true;
        refreshFullscreen();
    };
    const toggleFullscreen = () => {
        canvas.focus();
        try {
            const request = document.fullscreenElement === stage ? document.exitFullscreen() : stage.requestFullscreen();
            if (request && request.catch) request.catch(fullscreenFailed);
        } catch (error) { fullscreenFailed(); }
    };
    // SDL listens on window and cancels Space/Tab defaults. Keep button keys
    // local so native keyboard activation and focus traversal remain available.
    const controlKey = event => event.stopPropagation();
    const canvasTab = event => {
        if (event.key === 'Tab') event.stopPropagation();
    };
    canvas.addEventListener('keydown', canvasTab);
    canvas.addEventListener('keyup', canvasTab);
    fullscreen.addEventListener('keydown', controlKey);
    fullscreen.addEventListener('keyup', controlKey);
    fullscreen.addEventListener('click', toggleFullscreen);
    document.addEventListener('fullscreenchange', fullscreenChanged);
    document.addEventListener('fullscreenerror', fullscreenFailed);
    document.addEventListener('visibilitychange', refreshFullscreen);
    window.addEventListener('focus', refreshFullscreen);
    window.addEventListener('blur', refreshFullscreen);
    stage.appendChild(fullscreen);
    refreshFullscreen();
    const capture = document.createElement('button');
    capture.type = 'button';
    capture.id = 'tankgame-mouse-capture';
    capture.setAttribute('aria-live', 'polite');
    capture.style.cssText = 'position:fixed;bottom:12px;left:50%;transform:translateX(-50%);padding:12px 20px;z-index:1000;background:#111;color:#fff;opacity:1';
    const refreshCapture = () => {
        const locked = document.pointerLockElement === canvas;
        capture.textContent = locked ? 'Mouse captured — Esc to release' : 'Capture mouse';
        capture.disabled = locked;
        capture.hidden = document.hidden || !document.hasFocus();
    };
    const failed = () => {
        if (state.disposed) return;
        refreshCapture();
        capture.textContent = 'Mouse capture unavailable — retry';
        capture.title = 'Keyboard controls remain available.';
    };
    const lockChanged = () => {
        state.changed = true;
        capture.title = "";
        refreshCapture();
    };
    const activate = () => {
        // Request synchronously in the button gesture. Never request on startup
        // or automatically after Escape/blur; denial leaves keyboard play usable.
        canvas.focus();
        try {
            const request = canvas.requestPointerLock();
            if (request && request.catch) request.catch(failed);
        } catch (error) { failed(); }
    };
    capture.addEventListener('keydown', controlKey);
    capture.addEventListener('keyup', controlKey);
    capture.addEventListener('click', activate);
    document.addEventListener('pointerlockchange', lockChanged);
    document.addEventListener('pointerlockerror', failed);
    document.addEventListener('visibilitychange', refreshCapture);
    window.addEventListener('focus', refreshCapture);
    window.addEventListener('blur', refreshCapture);
    stage.appendChild(capture);
    refreshCapture();
    const audio = document.createElement('button');
    audio.id = 'tankgame-audio';
    audio.type = 'button';
    audio.textContent = 'Enable sound';
    audio.style.cssText = 'position:fixed;bottom:12px;left:12px;padding:12px 20px;z-index:1000;background:#111;color:#fff';
    audio.setAttribute('aria-live', 'polite');
    state.audioEnabled = false;
    const enableAudio = async () => {
        try {
            if (!Module.SDL2.audioContext) throw new Error('Audio unavailable');
            await Module.SDL2.audioContext.resume();
            if (state.disposed) return;
            if (Module.SDL2.audioContext.state !== 'running') throw new Error('Audio suspended');
            state.audioEnabled = true;
            _ActivateBrowserAudio();
            audio.textContent = 'Sound enabled';
            audio.disabled = true;
            canvas.focus();
        } catch (error) {
            if (!state.disposed) audio.textContent = 'Sound unavailable — retry';
        }
    };
    audio.addEventListener('click', enableAudio);
    audio.addEventListener('keydown', controlKey);
    audio.addEventListener('keyup', controlKey);
    stage.appendChild(audio);
    const contextLost = event => {
        event.preventDefault();
        state.contextLost = true;
        state.changed = true;
        update();
        status.textContent = 'Graphics interrupted — waiting for recovery';
        status.hidden = false;
    };
    const contextRestored = () => {
        state.contextLost = false;
        state.restore = true;
        update();
    };
    canvas.addEventListener('webglcontextlost', contextLost);
    canvas.addEventListener('webglcontextrestored', contextRestored);
    state.remove = () => {
        state.disposed = true;
        canvas.removeEventListener('webglcontextlost', contextLost);
        canvas.removeEventListener('webglcontextrestored', contextRestored);
        audio.removeEventListener('click', enableAudio);
        audio.removeEventListener('keydown', controlKey);
        audio.removeEventListener('keyup', controlKey);
        audio.remove();
        canvas.removeEventListener('keydown', canvasTab);
        canvas.removeEventListener('keyup', canvasTab);
        fullscreen.removeEventListener('keydown', controlKey);
        fullscreen.removeEventListener('keyup', controlKey);
        fullscreen.removeEventListener('click', toggleFullscreen);
        document.removeEventListener('fullscreenchange', fullscreenChanged);
        document.removeEventListener('fullscreenerror', fullscreenFailed);
        document.removeEventListener('visibilitychange', refreshFullscreen);
        window.removeEventListener('focus', refreshFullscreen);
        window.removeEventListener('blur', refreshFullscreen);
        if (document.fullscreenElement === stage) {
            const request = document.exitFullscreen();
            if (request && request.catch) request.catch(() => {});
        }
        fullscreen.remove();
        style.remove();
        if (shellControls) shellControls.style.display = shellDisplay;
        capture.removeEventListener('keydown', controlKey);
        capture.removeEventListener('keyup', controlKey);
        capture.removeEventListener('click', activate);
        document.removeEventListener('pointerlockchange', lockChanged);
        document.removeEventListener('pointerlockerror', failed);
        document.removeEventListener('visibilitychange', refreshCapture);
        window.removeEventListener('focus', refreshCapture);
        window.removeEventListener('blur', refreshCapture);
        if (document.pointerLockElement === canvas) document.exitPointerLock();
        capture.remove();
        document.removeEventListener('visibilitychange', update);
        window.removeEventListener('focus', update);
        window.removeEventListener('blur', update);
        status.remove();
    };
    Module.tankgameActivity = state;
});

EM_JS(int, ReadBrowserActivity, (), {
    const state = Module.tankgameActivity;
    const hidden = document.hidden;
    const focused = document.hasFocus();
    state.status.hidden = !hidden && focused && !state.contextLost;
    if (!state.contextLost) state.status.textContent = 'Paused — return to this window to resume';
    const flags = (hidden ? 1 : 0) | (focused ? 2 : 0) | (state.changed ? 4 : 0);
    state.changed = false;
    return flags | (state.contextLost ? 8 : 0) | (state.restore ? 16 : 0) | (state.audioEnabled ? 32 : 0);
});

EM_JS(void, RemoveBrowserActivity, (), {
    Module.tankgameActivity.remove();
    delete Module.tankgameActivity;
});

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
    App *app = static_cast<App *>(appPointer);
    static BrowserFramePolicy policy;
    const int activity = ReadBrowserActivity();
    static bool resourcesLost = false;
    if ((activity & 8) && !resourcesLost)
    {
        app->graphicsTask->Stop();
        resourcesLost = true;
    }
    if ((activity & 16) && !(activity & 8))
    {
        // Loss and restoration may both arrive between animation callbacks.
        if (!resourcesLost) app->graphicsTask->Stop();
        bool recovered = false;
        try { recovered = app->graphicsTask->Start(); }
        catch (const std::exception& error) {
            Logger::Get().Write("Graphics recovery failed: %s\n", error.what());
        }
        if (!recovered)
        {
            EM_ASM({ Module.onAbort('Graphics recovery failed'); });
            emscripten_cancel_main_loop();
            RemoveBrowserActivity();
            TaskHandler::GetSingleton().KillAllTasks();
            app->Shutdown();
            return;
        }
        resourcesLost = false;
        EM_ASM({ Module.tankgameActivity.restore = false; });
    }
    const auto decision = policy.NextFrame(start, (activity & 1) || (activity & 8), activity & 2, activity & 4);
    app->soundTask->SetActive(decision.tick && (activity & 32));
    if (decision.reset)
    {
        app->globalTimer->Reset();
        InputTask::ResetForFocusChange();
        previousStart = elapsed = tickTime = maxFrame = 0.0;
        frames = 0;
    }
    if (!decision.tick) return;
    const double frameTime = previousStart > 0.0 ? start - previousStart : 0.0;
    previousStart = start;
    static bool firstFrame = true;
    if (!app->Tick())
    {
        Logger::Get().Write("Browser frame loop stopped.\n");
        emscripten_cancel_main_loop();
        RemoveBrowserActivity();
        app->Shutdown();
        return;
    }

    static const bool profile = EM_ASM_INT({ return new URLSearchParams(location.search).has('profile'); });
    if (profile && frameTime > 0.0)
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
        EM_ASM({ Module.onAbort('Game initialization failed'); });
        return 1;
    }
    Logger::Get().Write("Registering non-blocking Emscripten browser frame loop.\n");
    InstallBrowserActivity();
    emscripten_set_main_loop_arg(BrowserFrame, App::GetSingletonPtr(), 0, true);
#else
    App::GetSingleton().Run(argc, argv);
    delete App::GetSingletonPtr();
#endif

    return 0;
}
