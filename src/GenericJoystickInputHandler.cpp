#include "GenericJoystickInputHandler.h"
#include "Tank.h"
#include "InputTask.h"
#include "App.h"
#include "GlobalTimer.h"
#include <SDL2/SDL.h>

void GenericJoystickInputHandler::HandleInput(Tank& tank)
{
    if (!InputTask::HasJoystick(tank.jid)) return;
    const auto& rules = tank.GetGameplayDefinitions().joystick;
    // Fire controls
    if ((tank.GetPrimaryType() == TankType::TYPE_PURPLE || tank.GetSecondaryType() == TankType::TYPE_PURPLE) &&
        (InputTask::GetButton(tank.jid, 0) || InputTask::GetButton(tank.jid, 3) || InputTask::GetButton(tank.jid, 7)))
    {
        tank.Fire((float)InputTask::GetAxis(tank.jid, 3) / rules.chargeDivisor);
    }
    else if (InputTask::GetButton(tank.jid, 3) || InputTask::GetButton(tank.jid, 0) || InputTask::GetButton(tank.jid, 7))
    {
        tank.Fire(1.0f);
    }

    // Special controls
    if ((tank.GetPrimaryType() == TankType::TYPE_PURPLE || tank.GetSecondaryType() == TankType::TYPE_PURPLE) &&
        (InputTask::GetButton(tank.jid, 5) || InputTask::GetButton(tank.jid, 11)))
    {
        tank.Special((float)InputTask::GetAxis(tank.jid, 3) / rules.specialChargeDivisor);
    }
    else if (InputTask::GetButton(tank.jid, 5) || InputTask::GetButton(tank.jid, 11))
    {
        tank.Special(1.0f);
    }

    // Movement controls
    if (InputTask::GetAxis(tank.jid, 1) < -rules.forwardDeadzone)
    {
        tank.Move(-1.0f * (float)InputTask::GetAxis(tank.jid, 1) / rules.forwardDivisor);
        if (tank.turbo && tank.energy > 0)
        {
            tank.Move(true);
            tank.energy -= tank.GetTurboEnergyPerSecond() * GlobalTimer::dT;
        }
    }

    if (InputTask::GetAxis(tank.jid, 0) < -rules.bodyDeadzone)
    {
        tank.RotBody((float)InputTask::GetAxis(tank.jid, 0) / rules.bodyDivisor);
    }
    if (InputTask::GetAxis(tank.jid, 0) > rules.bodyDeadzone)
    {
        tank.RotBody((float)InputTask::GetAxis(tank.jid, 0) / rules.bodyDivisor);
    }

    if (InputTask::GetAxis(tank.jid, 1) > rules.reverseDeadzone)
    {
        tank.Move(false);
        if (tank.turbo && tank.energy > 0)
        {
            tank.Move(false);
            tank.energy -= tank.GetTurboEnergyPerSecond() * GlobalTimer::dT;
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
#ifdef __EMSCRIPTEN__
    if (true) // Standard browser mapping: right stick X is axis 2.
#else
    if (tank.inputMode == InputMode::MODE_JOYSTICK_GENERIC)
#endif
    {
        if (InputTask::GetAxis(tank.jid, 2) < -rules.turretDeadzone)
        {
            tank.RotTurret((float)InputTask::GetAxis(tank.jid, 2) / rules.turretDivisor);
        }
        if (InputTask::GetAxis(tank.jid, 2) > rules.turretDeadzone)
        {
            tank.RotTurret((float)InputTask::GetAxis(tank.jid, 2) / rules.turretDivisor);
        }
    }
    else // EXTREME_3D or OTHER
    {
        if (InputTask::GetAxis(tank.jid, 3) < -rules.alternateTurretDeadzone)
        {
            tank.RotTurret((float)InputTask::GetAxis(tank.jid, 3) / rules.turretDivisor);
        }
        if (InputTask::GetAxis(tank.jid, 3) > rules.alternateTurretDeadzone)
        {
            tank.RotTurret((float)InputTask::GetAxis(tank.jid, 3) / rules.turretDivisor);
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
    if (InputTask::GetHat(tank.jid) == SDL_HAT_UP)
    {
        cam.ydist = 0.8;
        cam.xzdist = 1.0;
    }
    else if (InputTask::GetHat(tank.jid) == SDL_HAT_DOWN)
    {
        cam.ydist = 1.2;
        cam.xzdist = 2;
    }
    else if (InputTask::GetHat(tank.jid) == SDL_HAT_LEFT)
    {
        cam.ydist = 3.2;
        cam.xzdist = 2.2;
    }
    else if (InputTask::GetHat(tank.jid) == SDL_HAT_RIGHT)
    {
        cam.ydist = 20.2;
        cam.xzdist = 0.2;
    }
}
