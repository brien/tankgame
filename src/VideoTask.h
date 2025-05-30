#ifndef VIDEO_TASK_H
#define VIDEO_TASK_H

#include <SDL2/SDL.h>
#include "ITask.h"

class VideoTask : public ITask
{
public:
    VideoTask();
    ~VideoTask() = default;

    static int scrWidth, scrHeight, scrBPP;

    bool Start();
    void Update();
    void Stop();

protected:
    int flags;
    SDL_Window *window;
    SDL_Renderer *sdlRenderer;
    SDL_GLContext glContext;
};

#endif // VIDEO_H
