#pragma once

#include <SDL2/SDL.h>
#include "ITask.h"

class VideoTask : public ITask
{
public:
    VideoTask();
    ~VideoTask() = default;

    // Render dimensions in drawable pixels (settings supply the initial window size).
    static int scrWidth, scrHeight, scrBPP;
    static int difficultySetting; // 0=easy, 1=normal, 2=hard (read from settings file)

    bool Start();
    void Update();
    void Stop();

    // Query immediately before rendering, including display/DPI and canvas changes.
    // False means the drawable has no area and this frame should not be rendered.
    bool RefreshDrawableSize();

protected:
    int flags;
    SDL_Window *window = nullptr;
    SDL_Renderer *sdlRenderer;
    SDL_GLContext glContext;
};
