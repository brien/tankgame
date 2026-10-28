//
//  InputTask.cpp
//  tankgame
//
//

#include "InputTask.h"

#include <SDL2/SDL.h>

SDL_Joystick *InputTask::joysticks[MAX_JOYSTICKS] = {NULL};
string InputTask::joynames[2] = {"NULL"};
unsigned char *InputTask::keys = 0;
unsigned char *InputTask::oldKeys = 0;
int InputTask::keyCount = 0;
int InputTask::dX = 0;
int InputTask::dY = 0;

int InputTask::jdA[MAX_JOYSTICKS][MAX_JOYSTICKS] = {0};

unsigned int InputTask::buttons = 0;
unsigned int InputTask::oldButtons = 0;
std::vector<Uint8> InputTask::suppressedKeys;
unsigned int InputTask::suppressedButtons = 0;

InputTask::InputTask()
{
}

bool InputTask::Start()
{
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (i < MAX_JOYSTICKS)
        {
            joysticks[i] = SDL_JoystickOpen(i);
            for (int j = 0; j < 4; j++)
            {
                jdA[i][j] = 0;
            }
        }
    }
    const Uint8 *tempKeys = SDL_GetKeyboardState(&keyCount);
    keys = new Uint8[keyCount];
    memcpy(keys, tempKeys, sizeof(Uint8) * keyCount);
    oldKeys = new Uint8[keyCount]();
    suppressedKeys.assign(keyCount, 0);
    buttons = oldButtons = suppressedButtons = 0;
    dX = dY = 0;
#ifndef __EMSCRIPTEN__
    SDL_SetRelativeMouseMode(SDL_TRUE);
#endif
    SDL_PumpEvents();
    SDL_PumpEvents();
    return true;
}

void InputTask::Update()
{
    SDL_PumpEvents();
    oldButtons = buttons;
    buttons = SDL_GetRelativeMouseState(&dX, &dY);
    suppressedButtons &= buttons;
    buttons &= ~suppressedButtons;
    memcpy(oldKeys, keys, sizeof(unsigned char) * keyCount);
    const Uint8 *tempKeys = SDL_GetKeyboardState(&keyCount);
    memcpy(keys, tempKeys, sizeof(unsigned char) * keyCount);
    for (int i = 0; i < keyCount; ++i)
    {
        suppressedKeys[i] &= keys[i] != 0;
        if (suppressedKeys[i]) keys[i] = 0;
    }

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (i < MAX_JOYSTICKS)
        {
            for (int j = 0; j < SDL_JoystickNumAxes(joysticks[i]); j++)
            {
                jdA[i][j] = SDL_JoystickGetAxis(joysticks[i], j) - jdA[i][j];
            }
        }
    }
}

void InputTask::ResetForFocusChange()
{
    if (keys) memset(keys, 0, keyCount);
    if (oldKeys) memset(oldKeys, 0, keyCount);
    suppressedKeys.assign(keyCount, 1);
    suppressedButtons = ~0u;
    buttons = oldButtons = 0;
    SDL_GetRelativeMouseState(nullptr, nullptr);
    dX = dY = 0;
    memset(jdA, 0, sizeof(jdA));
}

int InputTask::GetAxis(int joystickId, int axis)
{
    int ret;
    if (joystickId < 0 || joystickId >= MAX_JOYSTICKS || joysticks[joystickId] == NULL)
    {
        ret = -1;
    }
    else
    {
        ret = SDL_JoystickGetAxis(joysticks[joystickId], axis);
    }

    return ret;
}

unsigned char InputTask::GetButton(int joystickId, int bid)
{
    unsigned char ret;
    if (joystickId < 0 || joystickId >= MAX_JOYSTICKS || joysticks[joystickId] == NULL)
    {
        ret = 0;
    }
    else
    {
        ret = SDL_JoystickGetButton(joysticks[joystickId], bid);
    }

    return ret;
}

void InputTask::Stop()
{
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        SDL_JoystickClose(joysticks[i]);
    }
    delete[] keys;
    delete[] oldKeys;
    keys = 0;
    oldKeys = 0;
    keyCount = 0;
    suppressedKeys.clear();
}
