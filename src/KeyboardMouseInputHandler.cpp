#include "KeyboardMouseInputHandler.h"
#include "Tank.h"
#include "InputTask.h"
#include "App.h"
#include "LevelHandler.h"
#include "GlobalTimer.h"
#include <SDL2/SDL.h>

void KeyboardMouseInputHandler::HandleInput(Tank& tank)
{
    tank.RotTurret(InputTask::dX * 2);

    if ((tank.type1 == TankType::TYPE_PURPLE || tank.type2 == TankType::TYPE_PURPLE) && InputTask::MouseStillDown(1))
    {
        tank.Fire(InputTask::dX * GlobalTimer::dT);
    }
    else if (InputTask::MouseStillDown(1))
    {
        tank.Fire(1);
    }

    if ((tank.type1 == TankType::TYPE_PURPLE || tank.type2 == TankType::TYPE_PURPLE) && InputTask::MouseStillDown(3))
    {
        tank.Special(InputTask::dX * GlobalTimer::dT);
    }
    else if (InputTask::MouseStillDown(3))
    {
        tank.Special(1);
    }
    
    if (InputTask::KeyDown(SDL_SCANCODE_T))
    {
        float oldy = tank.y;
        LevelHandler::GetSingleton().NextLevel(true);
        tank.x = LevelHandler::GetSingleton().start[0];
        tank.z = LevelHandler::GetSingleton().start[1];
        tank.y = oldy;
    }

    if (InputTask::KeyStillDown(SDL_SCANCODE_W))
    {
        tank.Move(true);
        if (tank.turbo)
        {
            tank.Move(true);
            tank.energy -= tank.jumpCost * GlobalTimer::dT;
        }
    }
    if (InputTask::KeyStillDown(SDL_SCANCODE_A))
    {
        tank.RotBody(false);
        if (tank.turbo)
        {
            tank.RotBody(false);
            tank.energy -= tank.jumpCost * GlobalTimer::dT;
        }
    }
    if (InputTask::KeyStillDown(SDL_SCANCODE_D))
    {
        tank.RotBody(true);
        if (tank.turbo)
        {
            tank.RotBody(true);
            tank.energy -= tank.jumpCost * GlobalTimer::dT;
        }
    }
    if (InputTask::KeyStillDown(SDL_SCANCODE_S))
    {
        tank.Move(false);
        if (tank.turbo)
        {
            tank.Move(false);
            tank.energy -= tank.jumpCost * GlobalTimer::dT;
        }
    }

    if (InputTask::KeyStillDown(SDL_SCANCODE_SPACE))
    {
        tank.Jump();
        if (tank.turbo)
        {
            tank.Jump();
        }
    }
    else
    {
        tank.isJumping = false;
    }
    
    if (InputTask::KeyStillDown(SDL_SCANCODE_LEFT))
    {
        tank.RotTurret(-200.0f * GlobalTimer::dT);
        if (tank.turbo)
        {
            tank.RotTurret(-300.0f * GlobalTimer::dT);
            tank.energy -= tank.jumpCost * GlobalTimer::dT;
        }
    }
    if (InputTask::KeyStillDown(SDL_SCANCODE_RIGHT))
    {
        tank.RotTurret(200.0f * GlobalTimer::dT);
        if (tank.turbo)
        {
            tank.RotTurret(300.0f * GlobalTimer::dT);
            tank.energy -= tank.jumpCost * GlobalTimer::dT;
        }
    }

    if (InputTask::CurKey(SDL_SCANCODE_LSHIFT) && tank.energy > 0)
    {
        tank.turbo = true;
    }
    else
        tank.turbo = false;

    // Camera controls
    if (InputTask::KeyStillDown(SDL_SCANCODE_UP))
    {
        App::GetSingleton().graphicsTask->cams[tank.identity.GetPlayerIndex()].ydist += 10 * GlobalTimer::dT;
    }
    if (InputTask::KeyStillDown(SDL_SCANCODE_DOWN))
    {
        App::GetSingleton().graphicsTask->cams[tank.identity.GetPlayerIndex()].ydist -= 10 * GlobalTimer::dT;
    }

    if (InputTask::KeyDown(SDL_SCANCODE_C))
    {
        auto& cam = App::GetSingleton().graphicsTask->cams[tank.identity.GetPlayerIndex()];
        if (cam.xzdist > 20)
        {
            cam.xzdist = 0.1;
        }
        else if (cam.xzdist <= 0.1)
        {
            cam.xzdist = 1.5;
        }
        else if (cam.xzdist <= 1.5)
        {
            cam.xzdist = 2.0;
        }
        else if (cam.xzdist <= 2.0)
        {
            cam.xzdist = 4.0;
        }
        else if (cam.xzdist <= 4.0)
        {
            cam.xzdist = 8.0;
        }
        else if (cam.xzdist <= 8.0)
        {
            cam.xzdist = 20.2;
        }
    }
}
