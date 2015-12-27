//
//  GlobalTimer.cpp
//  tankgame
//
//  Created by Brien Smith Martínez on 25/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#include <stdio.h>

#include "GlobalTimer.h"
#include <SDL2/SDL.h>

float GlobalTimer::dT=0;
unsigned long GlobalTimer::lastFrameIndex=0;
unsigned long GlobalTimer::thisFrameIndex=0;


bool GlobalTimer::Start()
{
    Reset();
    return true;
}

void GlobalTimer::Update()
{
    lastFrameIndex=thisFrameIndex;
    thisFrameIndex=SDL_GetTicks();
    dT=((float)(thisFrameIndex-lastFrameIndex))/1000.0f;
    dT*=1.0f;
}

void GlobalTimer::Reset()
{
    thisFrameIndex=SDL_GetTicks();
    lastFrameIndex=thisFrameIndex;
    dT=0;
}

void GlobalTimer::Stop()
{
    
}