//
//  GlobalTimer.cpp
//  tankgame
//
//

#include "GlobalTimer.h"
#include <SDL2/SDL.h>

float GlobalTimer::dT = 0;
unsigned long GlobalTimer::lastFrameIndex = 0;
unsigned long GlobalTimer::thisFrameIndex = 0;

GlobalTimer::GlobalTimer()
{
}

GlobalTimer::~GlobalTimer()
{
}

bool GlobalTimer::Start()
{
    Reset();
    return true;
}

void GlobalTimer::Update()
{
    lastFrameIndex = thisFrameIndex;
    thisFrameIndex = SDL_GetTicks();
    dT = ((float)(thisFrameIndex - lastFrameIndex)) / 1000.0f;
    dT *= 1.0f;
}

void GlobalTimer::Reset()
{
    thisFrameIndex = SDL_GetTicks();
    lastFrameIndex = thisFrameIndex;
    dT = 0;
}

void GlobalTimer::Stop()
{
}
