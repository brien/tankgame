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
#include "PlayerManager.h"
#include "Tank.h"
#include "App.h"
#include "events/Events.h"
#include "events/CollisionEvents.h"
#include "GameWorld.h"

void Bullet::CreateFX(FxType type, float x, float y, float z, float rx, float ry, float rz, float r, float g, float b, float a)
{
    gameWorld->CreateFX(type, x, y, z, rx, ry, rz, r, g, b, a);
}

void Bullet::CreateFX(FxType type, float x, float y, float z, float dx, float dy, float dz, float rx, float ry, float rz, float r, float g, float b, float a)
{
    gameWorld->CreateFX(type, x, y, z, dx, dy, dz, rx, ry, rz, r, g, b, a);
}

void Bullet::CreateWallImpactFX(float xpp, float zpp)
{
    // Determine impact orientation based on which axis changed
    if (static_cast<int>(x + xpp) != static_cast<int>(x) && static_cast<int>(z + zpp) == static_cast<int>(z))
    {
        // Horizontal wall impact (X-axis)
        CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, 0, 0, 0, 0, 90, primaryColor.r, primaryColor.g, primaryColor.b, 1);
    }
    else if (static_cast<int>(z + zpp) != static_cast<int>(z) && static_cast<int>(x + xpp) == static_cast<int>(x))
    {
        // Vertical wall impact (Z-axis)
        CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, 0, 0, 0, 90, 90, primaryColor.r, primaryColor.g, primaryColor.b, 1);
    }
    else if (static_cast<int>(x + xpp) != static_cast<int>(x) && static_cast<int>(z + zpp) != static_cast<int>(z))
    {
        // Corner impact - determine primary collision axis
        if (LevelHandler::GetSingleton().PointCollision((x + xpp), y, z) && !LevelHandler::GetSingleton().PointCollision(x, y, z + zpp))
        {
            // X-axis primary
            CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, 0, 0, 0, 0, 90, primaryColor.r, primaryColor.g, primaryColor.b, 1);
        }
        else
        {
            // Z-axis primary
            CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z, 0, 0, 0, 0, 90, 90, primaryColor.r, primaryColor.g, primaryColor.b, 1);
        }
    }
}

Bullet::Bullet()
    : x(0.0f), y(0.0f), z(0.0f),
      vx(0.0f), vy(0.0f), vz(0.0f),
      rx(0.0f), ry(0.0f), rz(0.0f),
      dty(0.0f),
      primaryColor(0.5f, 0.5f, 0.5f, 1.0f),
      secondaryColor(0.5f, 0.5f, 0.5f, 1.0f),
      moveRate(33.0f), power(0.0f),
      isSpecial(false),
      id(0), ownerIdentity(TankIdentity::Enemy(0)),
      type1(), type2(),
      dT(0.0f), maxdT(100.0f),
      numbounces(0), maxbounces(0),
      alive(true)
{
}

Bullet::Bullet(const TankIdentity& ownerIdentity, float power,
               TankType type1, TankType type2,
               int maxbounces,
               float dTpressed,
               const Color& primaryColor,
               const Color& secondaryColor,
               float x, float y, float z,
               float rx, float ry, float rz)
    : x(x), y(y), z(z),
      vx(0.0f), vy(0.0f), vz(0.0f),
      rx(rx), ry(ry), rz(rz),
      dty(0.0f),
      primaryColor(primaryColor),
      secondaryColor(secondaryColor),
      moveRate(33.0f), power(power),
      isSpecial(false),
      id(1), ownerIdentity(ownerIdentity),
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

    // NEW: Event-based collision detection
    
    // Check level collision first
    PointCollisionQuery levelQuery(x, y, z, CollisionLayer::LEVEL, this);
    Logger::Get().Write("Bullet::CheckForCollisions - Publishing PointCollisionQuery for level at (%.2f, %.2f, %.2f)\n", x, y, z);
    Events::GetBus().Publish(levelQuery);
    Logger::Get().Write("Bullet::CheckForCollisions - PointCollisionQuery result=%d\n", levelQuery.result);
    
    if (levelQuery.result) {
        // Post level collision event for CombatSystem to handle
        Events::GetBus().Post(BulletLevelCollisionEvent(this, x, y, z, xpp, zpp, ory));
        return; // Level collision handling will determine if bullet survives
    }

    // Check tank collisions (both players and enemies)
    // Use smaller radius to match original collision detection
    SphereCollisionQuery tankQuery(x, y, z, 0.1f, CollisionLayer::ALL_TANKS, this);
    Events::GetBus().Publish(tankQuery);
    
    if (!tankQuery.results.empty()) {
        // Filter out the tank that fired this bullet
        for (Entity* entity : tankQuery.results) {
            Tank* tank = dynamic_cast<Tank*>(entity);
            if (tank && tank->identity != ownerIdentity) {
                // Found a valid target (not the firing tank)
                Events::GetBus().Post(BulletCollisionEvent(this, tank, x, y, z));
                return; // CombatSystem will handle the collision response
            } else if (tank && tank->identity == ownerIdentity && dT > 0.5f) {
                // Allow collision with firing tank only after 0.5 seconds (for healing/self-damage)
                Events::GetBus().Post(BulletCollisionEvent(this, tank, x, y, z));
                return;
            }
        }
    }
    
    // Also check the previous position for fast-moving bullets
    if (xpp != 0 || zpp != 0) {
        SphereCollisionQuery prevQuery(x - xpp / 2, y, z - zpp / 2, 0.1f, CollisionLayer::ALL_TANKS, this);
        Events::GetBus().Publish(prevQuery);
        
        if (!prevQuery.results.empty()) {
            // Filter out the tank that fired this bullet
            for (Entity* entity : prevQuery.results) {
                Tank* tank = dynamic_cast<Tank*>(entity);
                if (tank && tank->identity != ownerIdentity) {
                    // Found a valid target (not the firing tank)
                    Events::GetBus().Post(BulletCollisionEvent(this, tank, x - xpp / 2, y, z - zpp / 2));
                    return;
                } else if (tank && tank->identity == ownerIdentity && dT > 0.5f) {
                    // Allow collision with firing tank only after 0.5 seconds (for healing/self-damage)
                    Events::GetBus().Post(BulletCollisionEvent(this, tank, x - xpp / 2, y, z - zpp / 2));
                    return;
                }
            }
        }
    }

    // Check bounds
    GetLevelBoundsQuery boundsQuery;
    Events::GetBus().Publish(boundsQuery);
    
    if (x >= boundsQuery.sizeX || x <= 0 || z >= boundsQuery.sizeZ || z <= 0) {
        Events::GetBus().Post(BulletOutOfBoundsEvent(this, x, y, z));
        return;
    }
    
    // Check timeout
    if (dT >= maxdT) {
        Events::GetBus().Post(BulletTimeoutEvent(this, dT));
        return;
    }
}

// Legacy HandleTankCollision method removed - collision handling now done by CombatSystem

// Legacy HandlePlayerCollision method removed - collision handling now done by CombatSystem

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

        // Get player tank from PlayerManager for audio volume calculation
        auto playerTanks = App::GetSingleton().gameTask->GetPlayerManager()->GetPlayerTanks();
        Tank* player0 = playerTanks[0];
        int dist = 128; // Default max distance
        if (player0) {
            dist = static_cast<int>(sqrt((x - player0->x) * (x - player0->x) + (z - player0->z) * (z - player0->z)));
        }

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

        if (ownerIdentity.IsPlayer() && isSpecial && type1 == TankType::TYPE_YELLOW)
        {
            if (type2 == TankType::TYPE_RED)
            {
                for (int i = 0; i < 20; i++)
                {

                    Bullet temp;

                    temp.ownerIdentity = ownerIdentity;

                    temp.x = x;
                    temp.y = y;
                    temp.z = z;

                    temp.rx = rx;
                    temp.ry = i * (360 / 20);
                    temp.rz = rz;

                    temp.power = power;

                    temp.primaryColor = Color(1.0f, 0.0f, 0.0f, 1.0f);
                    temp.secondaryColor = Color(1.0f, 0.0f, 0.0f, 1.0f);

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

                    temp.ownerIdentity = ownerIdentity;

                    temp.rx = rx;
                    temp.ry = ry;
                    temp.rz = rz;

                    temp.x = x + (.2 + i * .2 + numbounces * .2) * static_cast<float>(cos((ry + 90) * DTR));
                    temp.y = y;
                    temp.z = z + (.2 + i * .2 + numbounces * .2) * static_cast<float>(sin((ry + 90) * DTR));

                    temp.power = power;

                    temp.primaryColor = primaryColor;
                    temp.secondaryColor = secondaryColor;

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

                temp.ownerIdentity = ownerIdentity;

                temp.x = x;
                temp.y = y;
                temp.z = z;

                temp.rx = rx;
                temp.ry = -ory;
                temp.rz = rz;

                temp.power = power;

                temp.primaryColor = primaryColor;
                temp.secondaryColor = secondaryColor;

                temp.maxbounces = 4;
                temp.alive = true;

                //TODO: Special handling needs to be reimplemented after the bullet refactor
                //TankHandler::GetSingleton().players[(-1 * tankId) - 1].bulletq.push(temp);
            }
            else if (type2 == TankType::TYPE_PURPLE)
            {

                Bullet temp;

                temp.ownerIdentity = ownerIdentity;

                temp.x = x;
                temp.y = y;
                temp.z = z;

                temp.rx = rx;
                temp.ry = ry;
                temp.rz = rz;

                temp.power = power * 2;

                temp.dty = 10.0f;

                temp.primaryColor = primaryColor;
                temp.secondaryColor = secondaryColor;

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

        if (ownerIdentity.IsPlayer())
        {
            App::GetSingleton().gameTask->GetPlayerManager()->ResetHitComboByTankId(ownerIdentity.GetLegacyId());
        }

        CreateWallImpactFX(xpp, zpp);
    }
}

// Legacy draw method removed - bullets now handled by NewBulletRenderer
