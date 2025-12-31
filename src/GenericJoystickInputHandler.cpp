#include "GenericJoystickInputHandler.h"
#include "Tank.h"
#include "InputTask.h"
#include "App.h"
#include "GlobalTimer.h"
#include <SDL2/SDL.h>

void GenericJoystickInputHandler::HandleInput(Tank& tank)
{
    // Fire controls
    if ((tank.type1 == TankType::TYPE_PURPLE || tank.type2 == TankType::TYPE_PURPLE) && 
        (InputTask::GetButton(tank.jid, 0) || InputTask::GetButton(tank.jid, 3) || InputTask::GetButton(tank.jid, 7)))
    {
        tank.Fire((float)InputTask::GetAxis(tank.jid, 3) / (float)3200);
    }
    else if (InputTask::GetButton(tank.jid, 3) || InputTask::GetButton(tank.jid, 0) || InputTask::GetButton(tank.jid, 7))
    {
        tank.Fire(1.0f);
    }

    // Special controls
    if ((tank.type1 == TankType::TYPE_PURPLE || tank.type2 == TankType::TYPE_PURPLE) && 
        (InputTask::GetButton(tank.jid, 5) || InputTask::GetButton(tank.jid, 11)))
    {
        tank.Special((float)InputTask::GetAxis(tank.jid, 3) / (float)3200);
    }
    else if (InputTask::GetButton(tank.jid, 5) || InputTask::GetButton(tank.jid, 11))
    {
        tank.Special(1.0f);
    }

    // Movement controls
    if (InputTask::GetAxis(tank.jid, 1) < -5000)
    {
        tank.Move(-1.0f * (float)InputTask::GetAxis(tank.jid, 1) / (float)20000);
        if (tank.turbo && tank.energy > 0)
        {
            tank.Move(true);
            tank.energy -= tank.jumpCost * GlobalTimer::dT;
        }
    }
    
    if (InputTask::GetAxis(tank.jid, 0) < -8000)
    {
        tank.RotBody((float)InputTask::GetAxis(tank.jid, 0) / (float)32000);
    }
    if (InputTask::GetAxis(tank.jid, 0) > 8000)
    {
        tank.RotBody((float)InputTask::GetAxis(tank.jid, 0) / (float)32000);
    }
    
    if (InputTask::GetAxis(tank.jid, 1) > 8000)
    {
        tank.Move(false);
        if (tank.turbo && tank.energy > 0)
        {
            tank.Move(false);
            tank.energy -= tank.jumpCost * GlobalTimer::dT;
        }
    }

    // Jump controls
    if (InputTask::GetButton(tank.jid, 1) || InputTask::GetButton(tank.jid, 2) || InputTask::GetButton(tank.jid, 6))
    {
        tank.Jump();
        if (tank.turbo && tank.energy > 0)
        {
            tank.Jump();
        }
    }
    
    // Turret rotation - different for JOYSTICK_GENERIC vs EXTREME_3D/OTHER
    if (tank.inputMode == InputMode::MODE_JOYSTICK_GENERIC)
    {
        if (InputTask::GetAxis(tank.jid, 2) < -6000)
        {
            tank.RotTurret((float)InputTask::GetAxis(tank.jid, 2) / (float)3200);
        }
        if (InputTask::GetAxis(tank.jid, 2) > 6000)
        {
            tank.RotTurret((float)InputTask::GetAxis(tank.jid, 2) / (float)3200);
        }
    }
    else // EXTREME_3D or OTHER
    {
        if (InputTask::GetAxis(tank.jid, 3) < -5000)
        {
            tank.RotTurret((float)InputTask::GetAxis(tank.jid, 3) / (float)3200);
        }
        if (InputTask::GetAxis(tank.jid, 3) > 5000)
        {
            tank.RotTurret((float)InputTask::GetAxis(tank.jid, 3) / (float)3200);
        }
    }
    
    // Turbo controls
    if (tank.energy > 0 && (InputTask::GetButton(tank.jid, 4) || InputTask::GetButton(tank.jid, 10)))
    {
        tank.turbo = true;
    }
    else
    {
        tank.turbo = false;
    }

    // Camera controls via hat switch
    auto& cam = App::GetSingleton().graphicsTask->cams[tank.identity.GetPlayerIndex()];
    if (SDL_JoystickGetHat(InputTask::joysticks[tank.jid], 0) == SDL_HAT_UP)
    {
        cam.ydist = 0.8;
        cam.xzdist = 1.0;
    }
    else if (SDL_JoystickGetHat(InputTask::joysticks[tank.jid], 0) == SDL_HAT_DOWN)
    {
        cam.ydist = 1.2;
        cam.xzdist = 2;
    }
    else if (SDL_JoystickGetHat(InputTask::joysticks[tank.jid], 0) == SDL_HAT_LEFT)
    {
        cam.ydist = 3.2;
        cam.xzdist = 2.2;
    }
    else if (SDL_JoystickGetHat(InputTask::joysticks[tank.jid], 0) == SDL_HAT_RIGHT)
    {
        cam.ydist = 20.2;
        cam.xzdist = 0.2;
    }
}
