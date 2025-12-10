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

bool GlobalTimer::Start()
{
    Reset();
    return true;
}

void GlobalTimer::Update()
{
    lastFrameIndex = thisFrameIndex;
    thisFrameIndex = SDL_GetTicks();
    dT = static_cast<float>(thisFrameIndex - lastFrameIndex) / 1000.0f;
    dT *= 1.0f;
    
    // Update FPS calculation every frame
    GetFPS();
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

float GlobalTimer::GetFPS()
{
    static int frameCount = 0;
    static unsigned long lastFPSTime = 0;
    static float currentFPS = 60.0f; // Default FPS
    
    frameCount++;
    
    // Initialize on first call
    if (lastFPSTime == 0) {
        lastFPSTime = thisFrameIndex;
    }
    
    // Calculate FPS every second (1000ms)
    unsigned long elapsed = thisFrameIndex - lastFPSTime;
    if (elapsed >= 1000) {
        float elapsedSeconds = elapsed / 1000.0f;
        currentFPS = frameCount / elapsedSeconds;
        frameCount = 0;
        lastFPSTime = thisFrameIndex;
    }
    
    return currentFPS;
}
