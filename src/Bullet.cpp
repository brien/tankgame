//
//  Bullet.cpp
//  tankgame
//
//

#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#else
// If building on Linux:
#include <GL/gl.h>
#endif

#include "Bullet.h"

#include "SoundTask.h"
#include "GlobalTimer.h"
#include "math.h"
#include "LevelHandler.h"
#include "TankHandler.h"
#include "Tank.h"
#include "FXHandler.h"
#include "App.h"

Bullet::Bullet()
    : x(0.0f), y(0.0f), z(0.0f),
      vx(0.0f), vy(0.0f), vz(0.0f),
      rx(0.0f), ry(0.0f), rz(0.0f),
      dty(0.0f),
      r(0.5f), g(0.5f), b(0.5f),
      r2(0.5f), g2(0.5f), b2(0.5f),
      moveRate(33.0f), power(0.0f),
      isSpecial(false),
      id(0), tankId(0),
      type1(), type2(),
      dT(0.0f), maxdT(100.0f),
      numbounces(0), maxbounces(0),
      alive(true)
{
}

Bullet::Bullet(int tid, float power,
               TankType type1, TankType type2,
               int maxbounces,
               float dTpressed,
               float r, float g, float b,
               float r2, float g2, float b2,
               float x, float y, float z,
               float rx, float ry, float rz)
    : x(x), y(y), z(z),
      vx(0.0f), vy(0.0f), vz(0.0f),
      rx(rx), ry(ry), rz(rz),
      dty(0.0f),
      r(r), g(g), b(b),
      r2(r2), g2(g2), b2(b2),
      moveRate(33.0f), power(power),
      isSpecial(false),
      id(1), tankId(tid),
      type1(type1), type2(type2),
      dT(0.0f), maxdT(100.0f),
      numbounces(0), maxbounces(maxbounces),
      alive(true)
{
    if (type1 == TankType::TYPE_PURPLE)
    {
        dty = 320 * dTpressed;
        if (dTpressed < 0)
            dTpressed *= -1;
        this->power *= dTpressed;
        if (this->power > 1000)
            this->power = 1000;
        if (this->power < 100)
            this->power = 100;
    }
    else if (type2 == TankType::TYPE_PURPLE)
    {
        this->power *= 0.5f;
        dty = 160 * dTpressed;
        if (dTpressed < 0)
            dTpressed *= -1;
        this->power += this->power * dTpressed / 2;
        if (this->power > 1000)
            this->power = 1000;
        if (this->power < 100)
            this->power = 100;
    }
}

void Bullet::NextFrame()
{
    dT += GlobalTimer::dT;

    if (type1 == TankType::TYPE_PURPLE && isSpecial)
    {
        if (dty < 0)
        {
            dty -= 1000 * GlobalTimer::dT;
        }
        else
        {
            dty += 1000 * GlobalTimer::dT;
        }
    }

    ry += GlobalTimer::dT * dty;

    if (ry > 360)
    {
        ry -= 360;
    }
    if (rx > 360)
    {
        rz -= 360;
    }
    if (rz > 360)
    {
        rz -= 360;
    }

    float xpp = (GlobalTimer::dT * moveRate) * static_cast<float>(cos(ry * DTR));
    float zpp = (GlobalTimer::dT * moveRate) * static_cast<float>(sin(ry * DTR));

    x += xpp;
    z += zpp;

    float ory = ry;

    if (LevelHandler::GetSingleton().PointCollision(x, y, z))
    {
        HandleLevelCollision(xpp, zpp, ory);
    }

    for (int i = 0; i < TankHandler::GetSingleton().numPlayers; i++)
    {
        if (TankHandler::GetSingleton().players[i].PointCollision(x, y, z))
        {
            HandlePlayerCollision(TankHandler::GetSingleton().players[i]);
        }
    }

    for (vector<Tank>::iterator it = TankHandler::GetSingleton().tanks.begin(); it != TankHandler::GetSingleton().tanks.end(); ++it)
    {
        if ((*it).PointCollision(x, y, z) || (*it).PointCollision(x - xpp / 2, y, z - zpp / 2))
        {
            HandleTankCollision(*it);
        }
    }

    if (x >= LevelHandler::GetSingleton().sizeX || x <= 0 || z >= LevelHandler::GetSingleton().sizeZ || z <= 0)
    {
        alive = false;
        if (tankId < 0)
        {
            TankHandler::GetSingleton().hitCombo[(-1 * tankId) - 1] = 0;
        }
    }
    if (dT >= maxdT)
    {
        alive = false;
        if (tankId < 0)
        {
            TankHandler::GetSingleton().hitCombo[(-1 * tankId) - 1] = 0;
        }
    }
}

void Bullet::HandleTankCollision(Tank &tank)
{
    if (tank.id != tankId)
    {
        if (tank.energy > 0)
        {
            tank.energy -= power;
            TankHandler::GetSingleton().hitCombo[(-1 * tankId) - 1]++;
            TankHandler::GetSingleton().combo[(-1 * tankId) - 1] += static_cast<float>(TankHandler::GetSingleton().hitCombo[(-1 * tankId) - 1]) / 10;

            if (tankId < 0 && tank.energy <= 0)
            {
                float distx = x - TankHandler::GetSingleton().players[(-1 * tankId) - 1].x;
                float distz = z - TankHandler::GetSingleton().players[(-1 * tankId) - 1].z;

                float dist = sqrt(distx * distx + distz * distz);

                if (TankHandler::GetSingleton().hitCombo[(-1 * tankId) - 1] > 9)
                {
                    TankHandler::GetSingleton().combo[(-1 * tankId) - 1] += TankHandler::GetSingleton().comboNum[(-1 * tankId) - 1] / 2;
                    TankHandler::GetSingleton().players[(-1 * tankId) - 1].bonus = 23;
                    TankHandler::GetSingleton().players[(-1 * tankId) - 1].bonusTime = 0;
                    FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, TankHandler::GetSingleton().players[(-1 * tankId) - 1].x, TankHandler::GetSingleton().players[(-1 * tankId) - 1].y, TankHandler::GetSingleton().players[(-1 * tankId) - 1].z, 0, .01, 0, 0, 0, 90, 0.5, 0.5, 0, 1);
                }

                if (dist > 20)
                {
                    TankHandler::GetSingleton().combo[(-1 * tankId) - 1] += dist / 10;
                    TankHandler::GetSingleton().players[(-1 * tankId) - 1].bonus = 21;
                    TankHandler::GetSingleton().players[(-1 * tankId) - 1].bonusTime = 0;
                    FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, TankHandler::GetSingleton().players[(-1 * tankId) - 1].x, TankHandler::GetSingleton().players[(-1 * tankId) - 1].y, TankHandler::GetSingleton().players[(-1 * tankId) - 1].z, 0, .01, 0, 0, 0, 90, 0, 1, 1, 1);
                }

                if (numbounces > 0 && dT < 10.0f)
                {
                    TankHandler::GetSingleton().combo[(-1 * tankId) - 1] += (1 + dT / 2);
                    TankHandler::GetSingleton().players[(-1 * tankId) - 1].bonus = 22;
                    TankHandler::GetSingleton().players[(-1 * tankId) - 1].bonusTime = 0;
                    FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, TankHandler::GetSingleton().players[(-1 * tankId) - 1].x, TankHandler::GetSingleton().players[(-1 * tankId) - 1].y, TankHandler::GetSingleton().players[(-1 * tankId) - 1].z, 0, .01, 0, 0, 0, 90, 0.5, 0.5, 0, 1);
                }

                TankHandler::GetSingleton().combo[(-1 * tankId) - 1] += (10 / (TankHandler::GetSingleton().combo[(-1 * tankId) - 1] / 10 + 1));

                if (TankHandler::GetSingleton().combo[(-1 * tankId) - 1] > TankHandler::GetSingleton().special[(-1 * tankId) - 1])
                {
                    TankHandler::GetSingleton().special[(-1 * tankId) - 1] = TankHandler::GetSingleton().combo[(-1 * tankId) - 1];
                }

                TankHandler::GetSingleton().comboNum[(-1 * tankId) - 1]++;
            }

            if (tankId < 0 && isSpecial && type1 == TankType::TYPE_BLUE)
            {
                alive = true;
                power += 100;
            }
            else
                alive = false;

            FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, .01, 0, 0, tank.ry, 90, r, g, b, 1);
            FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, .01, 2, 0, ry, 90, r, g, b, 1);
        }
    }
    else
    {
        if (dT > .5)
        {
            if (tank.energy < tank.maxEnergy * 2)
            {
                tank.energy += power / 2;
                alive = false;
            }
        }
    }
}

void Bullet::HandlePlayerCollision(Tank &playerTank)
{
    if (tankId != playerTank.id)
    {
        if (tankId < 0 && !App::GetSingleton().gameTask->IsVersusMode())
        {
            App::GetSingleton().soundTask->PlayChannel(3);
            if (playerTank.energy < playerTank.maxEnergy * 2)
            {
                playerTank.energy += power / 2;
            }
            else if (playerTank.charge < playerTank.maxCharge * 2)
            {
                playerTank.charge += power / 8;
            }
            alive = false;
        }
        else
        {
            playerTank.energy -= power;
            alive = false;
            FXHandler::GetSingleton().CreateFX(FxType::TYPE_STAR, x, y, z, 0, .01, 0, 0, playerTank.ry, 90, r, g, b, 1);
            FXHandler::GetSingleton().CreateFX(FxType::TYPE_STAR, x, y, z, 0, .01, 2, 0, ry, 90, r, g, b, 1);
            App::GetSingleton().soundTask->PlayChannel(8);

            TankHandler::GetSingleton().hitCombo[(-1 * tankId) - 1]++;
            TankHandler::GetSingleton().combo[(-1 * tankId) - 1] += static_cast<float>(TankHandler::GetSingleton().hitCombo[(-1 * tankId) - 1]) / 10;
        }
    }
    else
    {
        if (dT > 0.5)
        {
            App::GetSingleton().soundTask->PlayChannel(3);
            if (playerTank.energy < playerTank.maxEnergy)
            {
                playerTank.energy += power / 2;
            }
            else if (playerTank.charge < playerTank.maxCharge)
            {
                playerTank.charge += power / 8;
            }
            alive = false;
        }
    }  
}

void Bullet::HandleLevelCollision(float xpp, float zpp, float ory)
{
    x -= xpp;
    z -= zpp;
    if (numbounces < maxbounces)
    {
        if (static_cast<int>(x + xpp) != static_cast<int>(x) && static_cast<int>(z + zpp) == static_cast<int>(z))
        {
            ry = -ry + 180;
        }

        if (static_cast<int>(z + zpp) != static_cast<int>(z) && static_cast<int>(x + xpp) == static_cast<int>(x))
        {
            ry = -ry;
        }

        if (static_cast<int>(x + xpp) != static_cast<int>(x) && static_cast<int>(z + zpp) != static_cast<int>(z))
        {
            if (LevelHandler::GetSingleton().PointCollision((x + xpp), y, z) && !LevelHandler::GetSingleton().PointCollision(x, y, z + zpp))
            {
                ry = -ry + 180;
            }
            else
            {
                ry = -ry;
            }
        }

        int dist = static_cast<int>(sqrt((x - TankHandler::GetSingleton().players[0].x) * (x - TankHandler::GetSingleton().players[0].x) + (z - TankHandler::GetSingleton().players[0].z) * (z - TankHandler::GetSingleton().players[0].z)));

        if (dist > 128)
        {
            dist = 128;
        }
        Mix_Volume(9, 128 - dist);

        App::GetSingleton().soundTask->PlayChannel(9);

        if (numbounces < 1)
        {
            if (type1 == TankType::TYPE_YELLOW && type2 == TankType::TYPE_YELLOW)
            {
                power *= 2.0;
            }
            else
            {
                power *= 1.5;
            }
        }

        if (tankId < 0 && isSpecial && type1 == TankType::TYPE_YELLOW)
        {
            if (type2 == TankType::TYPE_RED)
            {
                for (int i = 0; i < 20; i++)
                {

                    Bullet temp;

                    temp.tankId = tankId;

                    temp.x = x;
                    temp.y = y;
                    temp.z = z;

                    temp.rx = rx;
                    temp.ry = i * (360 / 20);
                    temp.rz = rz;

                    temp.power = power;

                    temp.r = 1;
                    temp.b = 0;
                    temp.g = 0;

                    temp.r2 = 1;
                    temp.b2 = 0;
                    temp.g2 = 0;

                    temp.maxbounces = 0;
                    temp.numbounces = 0;

                    temp.type1 = type2;
                    temp.type2 = type2;

                    temp.id = 1;
                    temp.alive = true;
                    //TODO: Special handling needs to be reimplemented after the bullet refactor
                    //TankHandler::GetSingleton().players[(-1 * tankId) - 1].bulletq.push(temp);
                }
            }
            else if (type2 == TankType::TYPE_BLUE)
            {
                for (int i = 0; i < 2; i++)
                {

                    Bullet temp;

                    temp.tankId = tankId;

                    temp.rx = rx;
                    temp.ry = ry;
                    temp.rz = rz;

                    temp.x = x + (.2 + i * .2 + numbounces * .2) * static_cast<float>(cos((ry + 90) * DTR));
                    temp.y = y;
                    temp.z = z + (.2 + i * .2 + numbounces * .2) * static_cast<float>(sin((ry + 90) * DTR));

                    temp.power = power;

                    temp.r = r;
                    temp.b = b;
                    temp.g = g;

                    temp.r2 = r2;
                    temp.b2 = b2;
                    temp.g2 = g2;

                    temp.maxbounces = 4;

                    temp.type1 = type1;
                    temp.type2 = type2;

                    temp.id = 1;
                    temp.alive = true;

                    //TODO: Special handling needs to be reimplemented after the bullet refactor
                    //TankHandler::GetSingleton().players[(-1 * tankId) - 1].bulletq.push(temp);
                }
            }
            else if (type2 == TankType::TYPE_YELLOW || type2 == TankType::TYPE_GREY)
            {

                Bullet temp;

                temp.tankId = tankId;

                temp.x = x;
                temp.y = y;
                temp.z = z;

                temp.rx = rx;
                temp.ry = -ory;
                temp.rz = rz;

                temp.power = power * 2;

                temp.r = r;
                temp.b = b;
                temp.g = g;

                temp.r2 = r2;
                temp.b2 = b2;
                temp.g2 = g2;

                temp.maxbounces = 4;

                temp.type1 = type1;
                temp.type2 = type2;

                temp.id = 1;
                temp.alive = true;

                //TODO: Special handling needs to be reimplemented after the bullet refactor
                //TankHandler::GetSingleton().players[(-1 * tankId) - 1].bulletq.push(temp);
            }
            else if (type2 == TankType::TYPE_PURPLE)
            {

                Bullet temp;

                temp.tankId = tankId;

                temp.x = x;
                temp.y = y;
                temp.z = z;

                temp.rx = rx;
                temp.ry = ry;
                temp.rz = rz;

                temp.power = power * 2;

                temp.dty = 10.0f;

                temp.r = r;
                temp.b = b;
                temp.g = g;

                temp.r2 = r2;
                temp.b2 = b2;
                temp.g2 = g2;

                temp.maxbounces = 16;

                temp.type1 = type1;
                temp.type2 = type2;

                temp.id = 1;
                temp.alive = true;
                //TODO: Special handling needs to be reimplemented after the bullet refactor
                //TankHandler::GetSingleton().players[(-1 * tankId) - 1].bulletq.push(temp);
            }
        }

        numbounces++;
    }
    else
    {
        alive = false;

        if (tankId < 0)
        {
            TankHandler::GetSingleton().hitCombo[(-1 * tankId) - 1] = 0;
        }

        if (static_cast<int>(x + xpp) != static_cast<int>(x) && static_cast<int>(z + zpp) == static_cast<int>(z))
        {
            FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, 0, 0, 0, 0, 90, r, g, b, 1);
        }

        if (static_cast<int>(z + zpp) != static_cast<int>(z) && static_cast<int>(x + xpp) == static_cast<int>(x))
        {
            FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, 0, 0, 0, 90, 90, r, g, b, 1);
        }

        if (static_cast<int>(x + xpp) != static_cast<int>(x) && static_cast<int>(z + zpp) != static_cast<int>(z))
        {
            if (LevelHandler::GetSingleton().PointCollision((x + xpp), y, z) && !LevelHandler::GetSingleton().PointCollision(x, y, z + zpp))
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, 0, 0, 0, 0, 90, r, g, b, 1);
            }
            else
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, 0, 0, 0, 90, 90, r, g, b, 1);
            }
        }
    }
}

// Legacy draw method removed - bullets now handled by NewBulletRenderer
