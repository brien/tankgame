#include "GameCubeInputHandler.h"
#include "Tank.h"
#include "InputTask.h"
#include "App.h"
#include "GlobalTimer.h"
#include <SDL2/SDL.h>

void GameCubeInputHandler::HandleInput(Tank& tank)
{
    // Fire controls
    if (InputTask::GetButton(tank.jid, 0) || InputTask::GetButton(tank.jid, 7) || InputTask::GetAxis(tank.jid, 3) > -5000)
    {
        if (tank.type1 == TankType::TYPE_PURPLE || tank.type2 == TankType::TYPE_PURPLE)
        {
            tank.Fire((float)InputTask::GetAxis(tank.jid, 2) / (float)6400);
        }
        else
        {
            tank.Fire(1);
        }
    }

    // Special controls
    if (InputTask::GetButton(tank.jid, 5) || InputTask::GetButton(tank.jid, 6))
    {
        if (tank.type1 == TankType::TYPE_PURPLE || tank.type2 == TankType::TYPE_PURPLE)
        {
            tank.Special((float)InputTask::GetAxis(tank.jid, 2) / (float)6000);
        }
        else
        {
            tank.Special(1);
        }
    }

    // Movement controls
    if (InputTask::GetAxis(tank.jid, 1) < -4000)
    {
        tank.Move(-1.0f * (float)InputTask::GetAxis(tank.jid, 1) / (float)18000);
        if (tank.turbo)
        {
            tank.Move(true);
            tank.charge -= tank.jumpCost * GlobalTimer::dT;
        }
    }
    
    if (InputTask::GetAxis(tank.jid, 0) < -8000)
    {
        tank.RotBody((float)InputTask::GetAxis(tank.jid, 0) / (float)28000);
    }
    if (InputTask::GetAxis(tank.jid, 0) > 8000)
    {
        tank.RotBody((float)InputTask::GetAxis(tank.jid, 0) / (float)28000);
    }
    
    if (InputTask::GetAxis(tank.jid, 1) > 8000)
    {
        tank.Move(false);
        if (tank.turbo)
        {
            tank.Move(false);
            tank.charge -= tank.jumpCost * GlobalTimer::dT;
        }
    }

    // Jump controls
    if (InputTask::GetButton(tank.jid, 1))
    {
        tank.Jump();
        if (tank.turbo)
        {
            tank.Jump();
        }
    }

    // Turret rotation
    if (InputTask::GetAxis(tank.jid, 2) < -6000)
    {
        tank.RotTurret((float)InputTask::GetAxis(tank.jid, 2) / (float)2500);
    }
    if (InputTask::GetAxis(tank.jid, 2) > 6000)
    {
        tank.RotTurret((float)InputTask::GetAxis(tank.jid, 2) / (float)2500);
    }

    // Additional jump control via axis 4
    if (InputTask::GetAxis(tank.jid, 4) > 5000)
    {
        tank.Jump();
        if (tank.turbo)
        {
            tank.Jump();
        }
    }

    // Turbo controls
    if (tank.charge > 0 && (InputTask::GetButton(tank.jid, 3) || InputTask::GetButton(tank.jid, 2)))
    {
        tank.turbo = true;
    }
    else
    {
        tank.turbo = false;
    }

    // Camera controls via hat switch
    auto& cam = App::GetSingleton().graphicsTask->cams[-1 * (tank.id + 1)];
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
