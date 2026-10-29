//
//  InputTask.cpp
//  tankgame
//
//

#include "InputTask.h"

#include <SDL2/SDL.h>
#include <algorithm>
#include <cstdlib>

SDL_Joystick *InputTask::joysticks[MAX_JOYSTICKS] = {NULL};
string InputTask::joynames[2] = {"NULL"};
unsigned char *InputTask::keys = 0;
unsigned char *InputTask::oldKeys = 0;
int InputTask::keyCount = 0;
int InputTask::dX = 0;
int InputTask::dY = 0;

int InputTask::jdA[MAX_JOYSTICKS][MAX_JOYSTICK_AXES] = {0};

unsigned int InputTask::buttons = 0;
unsigned int InputTask::oldButtons = 0;
std::vector<Uint8> InputTask::suppressedKeys;
unsigned int InputTask::suppressedButtons = 0;

bool InputTask::suppressedJoysticks[MAX_JOYSTICKS] = {};

InputTask::InputTask()
{
}

bool InputTask::Start()
{
    RefreshJoysticks();
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

    RefreshJoysticks();
    for (int i = 0; i < MAX_JOYSTICKS; ++i)
    {
        if (!joysticks[i]) continue;
        if (suppressedJoysticks[i])
        {
            bool neutral = true;
            for (int j = 0; j < SDL_JoystickNumAxes(joysticks[i]); ++j)
                neutral &= std::abs(static_cast<int>(SDL_JoystickGetAxis(joysticks[i], j))) < 4000;
            for (int j = 0; j < SDL_JoystickNumButtons(joysticks[i]); ++j)
                neutral &= !SDL_JoystickGetButton(joysticks[i], j);
            for (int j = 0; j < SDL_JoystickNumHats(joysticks[i]); ++j)
                neutral &= SDL_JoystickGetHat(joysticks[i], j) == SDL_HAT_CENTERED;
            suppressedJoysticks[i] = !neutral;
        }
        for (int j = 0; j < std::min(static_cast<int>(MAX_JOYSTICK_AXES), SDL_JoystickNumAxes(joysticks[i])); ++j)
            jdA[i][j] = GetAxis(i, j);
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
#ifdef __EMSCRIPTEN__
    std::fill_n(suppressedJoysticks, MAX_JOYSTICKS, true);
#endif
}

int InputTask::GetAxis(int joystickId, int axis)
{
    int ret;
    if (!HasJoystick(joystickId) || suppressedJoysticks[joystickId])
    {
        ret = 0;
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
    if (!HasJoystick(joystickId) || suppressedJoysticks[joystickId])
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
    for (int i = 0; i < MAX_JOYSTICKS; ++i)
    {
        if (joysticks[i]) SDL_JoystickClose(joysticks[i]);
        joysticks[i] = nullptr;
        suppressedJoysticks[i] = false;
    }
    delete[] keys;
    delete[] oldKeys;
    keys = 0;
    oldKeys = 0;
    keyCount = 0;
    suppressedKeys.clear();
}

bool InputTask::HasJoystick(int id)
{
    return id >= 0 && id < MAX_JOYSTICKS && joysticks[id] && SDL_JoystickGetAttached(joysticks[id]);
}

Uint8 InputTask::GetHat(int id)
{
    if (!HasJoystick(id) || suppressedJoysticks[id]) return SDL_HAT_CENTERED;
#ifdef __EMSCRIPTEN__
    if (GetButton(id, 12)) return SDL_HAT_UP;
    if (GetButton(id, 13)) return SDL_HAT_DOWN;
    if (GetButton(id, 14)) return SDL_HAT_LEFT;
    if (GetButton(id, 15)) return SDL_HAT_RIGHT;
#endif
    return SDL_JoystickGetHat(joysticks[id], 0);
}

void InputTask::RefreshJoysticks()
{
    for (int slot = 0; slot < MAX_JOYSTICKS; ++slot)
    {
        if (joysticks[slot] && !SDL_JoystickGetAttached(joysticks[slot]))
        {
            SDL_JoystickClose(joysticks[slot]);
            joysticks[slot] = nullptr;
            std::fill_n(jdA[slot], MAX_JOYSTICK_AXES, 0);
        }
    }
    // Device indices shift on unplug; retained instance IDs preserve player slots.
    for (int device = 0; device < SDL_NumJoysticks(); ++device)
    {
        const SDL_JoystickID id = SDL_JoystickGetDeviceInstanceID(device);
        bool known = false;
        for (auto* joystick : joysticks)
            known |= joystick && SDL_JoystickInstanceID(joystick) == id;
        if (known) continue;
        for (int slot = 0; slot < MAX_JOYSTICKS; ++slot)
        {
            if (joysticks[slot]) continue;
            joysticks[slot] = SDL_JoystickOpen(device);
#ifdef __EMSCRIPTEN__
            suppressedJoysticks[slot] = true;
#endif
            break;
        }
    }
}
