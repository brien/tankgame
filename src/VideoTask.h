#pragma once

#include <SDL2/SDL.h>
#include "ITask.h"

class VideoTask : public ITask
{
public:
    VideoTask();
    ~VideoTask() = default;

    static int scrWidth, scrHeight, scrBPP;
    static int difficultySetting; // 0=easy, 1=normal, 2=hard (read from settings file)

    bool Start();
    void Update();
    void Stop();

protected:
    int flags;
    SDL_Window *window;
    SDL_Renderer *sdlRenderer;
    SDL_GLContext glContext;
};
