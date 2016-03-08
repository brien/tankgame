//
//  InputTask.cpp
//  tankgame
//
//  Created by Brien Smith Martínez on 27/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#include "InputTask.h"

#include <SDL2/SDL.h>

SDL_Joystick *InputTask::joysticks[4]={NULL};
string InputTask::joynames[2]={"NULL"};
unsigned char *InputTask::keys=0;
unsigned char *InputTask::oldKeys=0;
int InputTask::keyCount=0;
int InputTask::dX=0;
int InputTask::dY=0;

int InputTask::jdA[4][4]={0};

unsigned int InputTask::buttons=0;
unsigned int InputTask::oldButtons=0;

InputTask::InputTask()
{
    
    
}

InputTask::~InputTask()
{
    
}

bool InputTask::Start()
{
    for(int i=0; i<SDL_NumJoysticks(); i++)
    {
        if(i<4)
        {
            joysticks[i] = SDL_JoystickOpen(i);
            for(int j=0; j<4; j++)
            {
                jdA[i][j] = 0;
                //joynames[i]=SDL_JoystickInstanceID(i);
            }
        }
    }
    const Uint8 *tempKeys=SDL_GetKeyboardState(&keyCount);
    keys=new Uint8 [keyCount];
    memcpy(keys,tempKeys,sizeof(Uint8)*keyCount);
    oldKeys=new Uint8 [keyCount];
    dX=dY=0;
    SDL_PumpEvents(); SDL_PumpEvents();
    return true;
}

void InputTask::Update()
{
    SDL_PumpEvents();
    oldButtons=buttons;
    buttons=SDL_GetRelativeMouseState(&dX,&dY);
    memcpy(oldKeys,keys,sizeof(unsigned char)*keyCount);
    const Uint8 *tempKeys=SDL_GetKeyboardState(&keyCount);
    memcpy(keys,tempKeys,sizeof(unsigned char)*keyCount);
    
    for(int i=0; i<SDL_NumJoysticks(); i++)
    {
        if(i<4)
        {
            for(int j=0; j<SDL_JoystickNumAxes(joysticks[i]); j++)
            {
                jdA[i][j] = SDL_JoystickGetAxis(joysticks[i], j) - jdA[i][j];
            }
        }
    }
    
    
    //number_of_buttons = SDL_JoystickNumButtons(joystick);
}

int InputTask::GetAxis(int jid, int axis)
{
    int ret;
    if(jid<0 || jid>3 || joysticks[jid]==NULL)
        ret=-1;
    else
        ret = SDL_JoystickGetAxis(joysticks[jid], axis);
    
    return ret;
}

unsigned char InputTask::GetButton(int jid, int bid)
{
    unsigned char ret;
    if(jid<0 || jid>3 || joysticks[jid]==NULL)
    {
        ret=0;
    }
    else
    {
        ret = SDL_JoystickGetButton(joysticks[jid], bid);
    }
    
    return ret;
}

void InputTask::Stop()
{
    for(int i=0; i<SDL_NumJoysticks(); i++)
    {
        SDL_JoystickClose(joysticks[i]);
    }
    delete [] oldKeys;
    keys=0;
    oldKeys=0;
}
