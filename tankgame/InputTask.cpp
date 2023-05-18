//
//  InputTask.cpp
//  tankgame
//
//

#include "InputTask.h"

#include <SDL2/SDL.h>

SDL_Joystick *InputTask::joysticks[MAX_JOYSTICKS]={NULL};
string InputTask::joynames[2]={"NULL"};
unsigned char *InputTask::keys=0;
unsigned char *InputTask::oldKeys=0;
int InputTask::keyCount=0;
int InputTask::dX=0;
int InputTask::dY=0;

int InputTask::jdA[MAX_JOYSTICKS][MAX_JOYSTICKS]={0};

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
        if(i < MAX_JOYSTICKS)
        {
            joysticks[i] = SDL_JoystickOpen(i);
            for(int j=0; j<4; j++)
            {
                jdA[i][j] = 0;
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
        if(i < MAX_JOYSTICKS)
        {
            for(int j=0; j<SDL_JoystickNumAxes(joysticks[i]); j++)
            {
                jdA[i][j] = SDL_JoystickGetAxis(joysticks[i], j) - jdA[i][j];
            }
        }
    }
}

int InputTask::GetAxis(int jid, int axis)
{
    int ret;
    if(jid < 0 || jid >= MAX_JOYSTICKS || joysticks[jid]==NULL)
    {
        ret=-1;
    }
    else
    {
        ret = SDL_JoystickGetAxis(joysticks[jid], axis);
    }
    
    return ret;
}

unsigned char InputTask::GetButton(int jid, int bid)
{
    unsigned char ret;
    if(jid < 0 || jid >= MAX_JOYSTICKS || joysticks[jid]==NULL)
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
