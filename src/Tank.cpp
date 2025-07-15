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

#include "Tank.h"

#include "App.h"
#include "LevelHandler.h"
#include "FXHandler.h"
#include "TankHandler.h"
#include "TankTypeManager.h"
#include "TankRenderer.h"
#include "InputHandler.h"
#include "TankCollisionHelper.h"
#include "InputHandlerFactory.h"
#include "BulletHandler.h"

void Tank::SetType(TankType t1, TankType t2)
{
    type1 = t1;
    type2 = t2;

    // Common initialization
    movRate = 5;
    jumpCost = 250;
    jumpRate = 18.0f;
    fallRate = 16.0f;

    // Apply type-specific configuration
    auto config = TankTypeManager::GetConfig(t1, t2);
    fireRate = config.fireRate;
    attack = config.attack;
    maxCharge = config.maxCharge;
    chargeRegen = config.chargeRegen;
    moveCost = config.moveCost;
    fireCost = config.maxCharge / 10.0f;
    chargeCost = config.chargeCost;
    bounces = config.bounces;

    // Set colors
    SetTankColors(t1, t2);
}

void Tank::SetTankColors(TankType primary, TankType secondary)
{
    auto setColor = [](TankType type, float &r, float &g, float &b)
    {
        switch (type)
        {
        case TankType::TYPE_GREY:
            r = 0.5f;
            g = 0.5f;
            b = 0.5f;
            break;
        case TankType::TYPE_RED:
            r = 1.0f;
            g = 0.0f;
            b = 0.0f;
            break;
        case TankType::TYPE_BLUE:
            r = 0.0f;
            g = 0.0f;
            b = 1.0f;
            break;
        case TankType::TYPE_YELLOW:
            r = 1.0f;
            g = 1.0f;
            b = 0.0f;
            break;
        case TankType::TYPE_PURPLE:
            r = 1.0f;
            g = 0.0f;
            b = 1.0f;
            break;
        case TankType::TANK_TYPE_COUNT:
            r = 0.0f;
            g = 1.0f;
            b = 0.0f;
            break;
        default:
            r = g = b = 1.0f;
            break;
        }
    };

    setColor(primary, r, g, b);
    setColor(secondary, r2, g2, b2);
}

void Tank::SetInputMode(InputMode mode)
{
    inputMode = mode;
    inputHandler = InputHandlerFactory::CreateInputHandler(mode);
}

void Tank::Die()
{

    if (!alive && deadtime < 0.01)
    {
        if (id < 0)
        {
            App::GetSingleton().soundTask->PlayChannel(7);
        }
        else
        {
            App::GetSingleton().soundTask->PlayChannel(6);
        }
    }

    TankHandler::GetSingleton().numAttackingTanks--;

    if (TankHandler::GetSingleton().tanks.size() != 1 && id >= 0)
    {
        LevelHandler::GetSingleton().AddItem(x, y + .2, z, type1);
    }

    if (deadtime < 0.01)
    {
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_DEATH, x, y, z + .5, 0, .05, 0, 0, ry, 0, r, g, b, 1);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_DEATH, x, y, z + .5, 0, .05, 0, 0, ry, 0, r, g, b, .15);
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_DEATH, x, y, z + .5, 0, .05, 2, 0, ry, 0, r, g, b, .15);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_DEATH, x + .5, y, z + .5, 0, .05, -3, 0, ry, 0, r, g, b, .15);
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_DEATH, x + .5, y, z + .5, 0, .05, 2, 0, ry, 0, r, g, b, .15);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_DEATH, x + .5, y, z - .5, 3, .05, 0, 2, ry, 0, r2, g2, b2, .15);
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_DEATH, x + .5, y, z - .5, 2, .05, 0, 0, ry, 0, r2, g2, b2, .15);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_DEATH, x + .5, y, z - .5, 2, .01, 0, 0, ry, 0, r2, g2, b2, .15);
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_DEATH, x + .5, y, z - .5, 2, .05, 0, -2, ry, 0, r2, g2, b2, .15);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z + .5, 0, .01, 0, 0, ry, 90, r, g, b, 1);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z + .5, 0, .01, 0, 0, ry, 90, r, g, b, 1);
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x, y, z + .5, 0, .01, 2, 0, ry, 90, r, g, b, 1);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x + .5, y, z + .5, 0, .01, -3, 0, ry, 90, r, g, b, 1);
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x + .5, y, z + .5, 0, .01, 2, 0, ry, 90, r, g, b, 1);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x + .5, y, z - .5, 3, .01, 0, 2, ry, 90, r2, g2, b2, 1);
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x + .5, y, z - .5, 2, .01, 0, 0, ry, 90, r2, g2, b2, 1);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x + .5, y, z - .5, 2, .01, 0, 0, ry, 90, r2, g2, b2, 1);
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_SQUARE, x + .5, y, z - .5, 2, .01, 0, -2, ry, 90, r2, g2, b2, 1);
    }
    if (TankHandler::GetSingleton().tanks.size() == 1 && id >= 0)
    {
        LevelHandler::GetSingleton().SetTerrainHeight(static_cast<int>(x), static_cast<int>(z), -20);
    }
}

void Tank::Fire(float dTpressed)
{

    if (charge >= fireCost && fireTimer > fireRate)
    {
        float ratio = (z - TankHandler::GetSingleton().players[0].z) / (x - TankHandler::GetSingleton().players[0].x);
        float ryp = toDegrees(atan(ratio));

        if (TankHandler::GetSingleton().players[0].x < x)
        {
            ryp += 180;
        }

        ryp -= (TankHandler::GetSingleton().players[0].ry + TankHandler::GetSingleton().players[0].rty);

        float dist = sqrt((x - TankHandler::GetSingleton().players[0].x) * (x - TankHandler::GetSingleton().players[0].x) + (z - TankHandler::GetSingleton().players[0].z) * (z - TankHandler::GetSingleton().players[0].z));

        Mix_SetPosition(2, ryp, 10 * static_cast<int>(dist));

        App::GetSingleton().soundTask->PlayChannel(2);

        float bulletMovRate = 33.0f;

        Bullet temp(id, attack, type1, type2, bounces,
                    dTpressed,
                    r, g, b,
                    r2, g2, b2,
                    x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                    y + .25,
                    z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                    rtx + rx, rty + ry, rtz + rz);

        BulletHandler::GetSingleton().AddBullet(temp);

        fireTimer = 0;

        charge -= fireCost;
    }
}

void Tank::Special(float dTpressed)
{

    if (TankHandler::GetSingleton().special[(-1 * id) - 1] >= fireCost / 5 && fireTimer > fireRate)
    {
        float ratio = (z - TankHandler::GetSingleton().players[0].z) / (x - TankHandler::GetSingleton().players[0].x);
        float ryp = toDegrees(atan(ratio));

        if (TankHandler::GetSingleton().players[0].x < x)
        {
            ryp += 180;
        }

        ryp -= (TankHandler::GetSingleton().players[0].ry + TankHandler::GetSingleton().players[0].rty);

        float dist = sqrt((x - TankHandler::GetSingleton().players[0].x) * (x - TankHandler::GetSingleton().players[0].x) + (z - TankHandler::GetSingleton().players[0].z) * (z - TankHandler::GetSingleton().players[0].z));

        Mix_SetPosition(2, ryp, 10 * static_cast<int>(dist));

        App::GetSingleton().soundTask->PlayChannel(2);

        float bulletMovRate = 33.0f;

        Bullet temp(id, attack, type1, type2, bounces,
                    dTpressed,
                    r, g, b,
                    r2, g2, b2,
                    x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                    y + .25,
                    z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                    rtx + rx, rty + ry, rtz + rz);

        if (type1 == TankType::TYPE_RED)
        {
            for (int i = 0; i < 10; i++)
            {
                Bullet temp2(id, attack, type1, type2, 0,
                             dTpressed,
                             1.0, 0.0, 0.0,
                             0.5, 0.5, 0.5,
                             x,
                             y + .25,
                             z,
                             rtx + rx, rty + ry + i * (270 / 10) + 52, rtz + rz);

                BulletHandler::GetSingleton().AddBullet(temp2);
            }

            BulletHandler::GetSingleton().AddBullet(temp);
            Bullet temp(id, attack, type1, type2, bounces,
                        dTpressed,
                        r, g, b,
                        r2, g2, b2,
                        x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                        y + .25,
                        z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                        rtx + rx, rty + ry - 10, rtz + rz);

            BulletHandler::GetSingleton().AddBullet(temp);

            Bullet temp2(id, attack, type1, type2, bounces,
                         dTpressed,
                         r, g, b,
                         r2, g2, b2,
                         x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                         y + .25,
                         z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                         rtx + rx, rty + ry + 20, rtz + rz);

            BulletHandler::GetSingleton().AddBullet(temp2);
        }
        if (type1 == TankType::TYPE_BLUE)
        {

            if (type2 != TankType::TYPE_RED)
            {
                Bullet temp(id, attack, type1, type2, bounces,
                            dTpressed,
                            r, g, b,
                            r2, g2, b2,
                            x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                            y + .50,
                            z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                            rtx + rx, rty + ry, rtz + rz);

                BulletHandler::GetSingleton().AddBullet(temp);
            }

            Bullet temp(id, attack, type1, type2, bounces,
                        dTpressed,
                        r, g, b,
                        r2, g2, b2,
                        x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR) + .2 * std::cosf((rty + ry + 90) * DTR),
                        y + .25,
                        z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR) + .2 * std::sinf((rty + ry + 90) * DTR),
                        rtx + rx, rty + ry, rtz + rz);

            BulletHandler::GetSingleton().AddBullet(temp);
        }

        if (type1 == TankType::TYPE_YELLOW)
        {
            if (type2 != TankType::TYPE_YELLOW)
            {
                Bullet temp(id, attack, type1, type2, 4,
                            dTpressed,
                            r, g, b,
                            r2, g2, b2,
                            x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                            y + .25,
                            z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                            rtx + rx, rty + ry, rtz + rz);

                BulletHandler::GetSingleton().AddBullet(temp);
            }

            BulletHandler::GetSingleton().AddBullet(temp);
        }

        if (type1 == TankType::TYPE_PURPLE)
        {
            Bullet temp(id, attack, type1, type2, bounces,
                        dTpressed,
                        r, g, b,
                        r2, g2, b2,
                        x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                        y + .25,
                        z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                        rtx + rx, rty + ry, rtz + rz);

            BulletHandler::GetSingleton().AddBullet(temp);

            Bullet temp1(id, attack, type1, type2, bounces,
                         dTpressed,
                         r, g, b,
                         r2, g2, b2,
                         x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                         y + .25,
                         z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                         rtx + rx, rty + ry - 90, rtz + rz);

            BulletHandler::GetSingleton().AddBullet(temp1);

            Bullet temp2(id, attack, type1, type2, bounces,
                         dTpressed,
                         r, g, b,
                         r2, g2, b2,
                         x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                         y + .25,
                         z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                         rtx + rx, rty + ry + 180, rtz + rz);

            BulletHandler::GetSingleton().AddBullet(temp2);

            Bullet temp3(id, attack, type1, type2, bounces,
                         dTpressed,
                         r, g, b,
                         r2, g2, b2,
                         x + (GlobalTimer::dT * bulletMovRate) * std::cosf((rty + ry) * DTR),
                         y + .25,
                         z + (GlobalTimer::dT * bulletMovRate) * std::sinf((rty + ry) * DTR),
                         rtx + rx, rty + ry + 90, rtz + rz);

            BulletHandler::GetSingleton().AddBullet(temp3);
        }

        fireTimer = 0;

        TankHandler::GetSingleton().special[(-1 * id) - 1] -= fireCost / 5;
    }
}

bool Tank::PointCollision(float cx, float cy, float cz)
{
    bool result = false;
    if ((cy - y) < 0.3f && (y - cy) < 0)
    {
        if (sqrt((cx - x) * (cx - x) + (cz - z) * (cz - z)) < collisionRadius * 2)
        {
            result = true;
        }
    }
    return result;
}

void Tank::Fall()
{

    vy -= (fallRate * GlobalTimer::dT);

    float dy = vy * GlobalTimer::dT;

    if (dy > 10.0f * GlobalTimer::dT)
    {
        dy = 10.0f * GlobalTimer::dT;
    }

    if (dy < -15.0f * GlobalTimer::dT)
    {
        dy = -15.0f * GlobalTimer::dT;
    }

    y += dy;

    if (y > 25)
    {
        y = 25;
        vy = 0;
    }

    // Check for float collision with upward offset
    if (TankCollisionHelper::CheckFourPointFloatCollision(*this, 0.2f))
    {
        y = static_cast<int>(y) + .8;
        vy = 0;
    }

    int highest = -1;

    // Check for float collision at current position
    if (TankCollisionHelper::CheckFourPointFloatCollision(*this))
    {
        highest = TankCollisionHelper::FindHighestFloatHeight(*this);
        y = static_cast<float>(highest);

        if (!grounded)
        {
            if (charge < 2 * maxCharge / 3)
            {
                charge += maxCharge / 3;
            }
            if (id < 0)
            {
                App::GetSingleton().soundTask->PlayChannel(5);
            }
        }
        isJumping = false;
        grounded = true;
        jumpTime = 0.0f;

        vy = 0;
    }
    // Check for fall collision (terrain collision)
    else if (TankCollisionHelper::CheckFourPointFallCollision(*this))
    {
        // Find the height of the highest collision point
        highest = TankCollisionHelper::FindHighestTerrainHeight(*this);
        y = static_cast<float>(highest);

        if (highest < -10 && (id == -1 || id == -2))
        {
            LevelHandler::GetSingleton().NextLevel(true);
            x = LevelHandler::GetSingleton().start[0];
            z = LevelHandler::GetSingleton().start[1];
            y = 24;
        }
        else
        {
            vy = 0;
            if (!grounded)
            {
                if (id < 0)
                    App::GetSingleton().soundTask->PlayChannel(5);
            }
        }
        if (!grounded)
        {
            if (charge < 2 * maxCharge / 3)
            {
                charge += maxCharge / 3;
            }
        }
        isJumping = false;
        grounded = true;
        jumpTime = 0.0f;
    }
    else
    {
        grounded = false;
    }
}

Tank::Tank()
{
    x = y = z = 0.0f;
    vx = vy = vz = 0.0f;
    rx = ry = rz = 0.0f;
    rtx = rty = rtz = 0.0f;
    rrl = 0.0f;
    rr = 0.0f;
    r = 0.0f;
    r2 = 0.0f;
    g = 0.0f;
    g2 = 0.0f;

    jid = 0;
    inputMode = InputMode::MODE_KEYBOARD_MOUSE;
    id = 0;
    hitNum = 0;
    hitAlpha = 0.0f;

    type1 = TankType::TYPE_RED;
    type2 = TankType::TYPE_BLUE;
    bonus = 0;
    bonusTime = 0.0f;
    deadtime = 0.0f;
    jumpTime = 0.0f;
    turbo = false;
    recharge = false;
    isJumping = false;
    grounded = false;

    collisionRadius = 0.0f;
    rotRate = 0.0f;
    movRate = 0.0f;
    moveCost = 0.0f;
    maxEnergy = 0.0f;
    maxCharge = 0.0f;
    jumpRate = 0.0f;
    jumpCost = 0.0f;
    fireTimer = 0.0f;
    fireRate = 0.0f;
    fireCost = 0.0f;
    fallRate = 0.0f;
    energyRegen = 0.0f;
    energy = 0.0f;
    dist = 0.0f;
    control = 0;
    chargeRegen = 0.0f;
    chargeCost = 0.0f;
    charge = 0.0f;
    bounces = 0.0f;
    b = 0.0f;
    b2 = 0.0f;
    attack = 0.0f;
    alive = true;
    isPlayer = false;

    collisionPoints[21] = {0};
    
    // Initialize input handler after setting inputMode
    inputHandler = InputHandlerFactory::CreateInputHandler(inputMode);
}

Tank::~Tank()
{
    // Destructor implementation - required for proper destruction of unique_ptr<InputHandler>
    // The InputHandler's destructor will be called automatically
}

Tank::Tank(Tank&& other) noexcept
    : isPlayer(other.isPlayer),
      turbo(other.turbo),
      recharge(other.recharge),
      charge(other.charge),
      maxCharge(other.maxCharge),
      chargeRegen(other.chargeRegen),
      fireCost(other.fireCost),
      jumpCost(other.jumpCost),
      moveCost(other.moveCost),
      chargeCost(other.chargeCost),
      fireTimer(other.fireTimer),
      fireRate(other.fireRate),
      bounces(other.bounces),
      attack(other.attack),
      alive(other.alive),
      energy(other.energy),
      maxEnergy(other.maxEnergy),
      energyRegen(other.energyRegen),
      id(other.id),
      x(other.x), y(other.y), z(other.z),
      vx(other.vx), vy(other.vy), vz(other.vz),
      collisionRadius(other.collisionRadius),
      control(other.control),
      inputMode(other.inputMode),
      jid(other.jid),
      inputHandler(std::move(other.inputHandler)),
      rx(other.rx), ry(other.ry), rz(other.rz), rr(other.rr), rrl(other.rrl),
      rtx(other.rtx), rty(other.rty), rtz(other.rtz),
      rotRate(other.rotRate), movRate(other.movRate), jumpRate(other.jumpRate),
      fallRate(other.fallRate),
      type1(other.type1),
      type2(other.type2),
      r(other.r), g(other.g), b(other.b),
      r2(other.r2), g2(other.g2), b2(other.b2),
      dist(other.dist),
      isJumping(other.isJumping),
      grounded(other.grounded),
      jumpTime(other.jumpTime),
      bonus(other.bonus),
      bonusTime(other.bonusTime),
      deadtime(other.deadtime),
      hitAlpha(other.hitAlpha),
      hitNum(other.hitNum)
{
    // Copy collision points array
    for (int i = 0; i < 21; ++i) {
        collisionPoints[i] = other.collisionPoints[i];
    }
}

Tank& Tank::operator=(Tank&& other) noexcept
{
    if (this != &other) {
        isPlayer = other.isPlayer;
        turbo = other.turbo;
        recharge = other.recharge;
        charge = other.charge;
        maxCharge = other.maxCharge;
        chargeRegen = other.chargeRegen;
        fireCost = other.fireCost;
        jumpCost = other.jumpCost;
        moveCost = other.moveCost;
        chargeCost = other.chargeCost;
        fireTimer = other.fireTimer;
        fireRate = other.fireRate;
        bounces = other.bounces;
        attack = other.attack;
        alive = other.alive;
        energy = other.energy;
        maxEnergy = other.maxEnergy;
        energyRegen = other.energyRegen;
        id = other.id;
        x = other.x; y = other.y; z = other.z;
        vx = other.vx; vy = other.vy; vz = other.vz;
        collisionRadius = other.collisionRadius;
        control = other.control;
        inputMode = other.inputMode;
        jid = other.jid;
        inputHandler = std::move(other.inputHandler);
        rx = other.rx; ry = other.ry; rz = other.rz; rr = other.rr; rrl = other.rrl;
        rtx = other.rtx; rty = other.rty; rtz = other.rtz;
        rotRate = other.rotRate; movRate = other.movRate; jumpRate = other.jumpRate;
        fallRate = other.fallRate;
        type1 = other.type1;
        type2 = other.type2;
        r = other.r; g = other.g; b = other.b;
        r2 = other.r2; g2 = other.g2; b2 = other.b2;
        dist = other.dist;
        isJumping = other.isJumping;
        grounded = other.grounded;
        jumpTime = other.jumpTime;
        bonus = other.bonus;
        bonusTime = other.bonusTime;
        deadtime = other.deadtime;
        hitAlpha = other.hitAlpha;
        hitNum = other.hitNum;
        
        // Copy collision points array
        for (int i = 0; i < 21; ++i) {
            collisionPoints[i] = other.collisionPoints[i];
        }
    }
    return *this;
}

void Tank::Init()
{
    SetInputMode(InputMode::MODE_KEYBOARD_MOUSE);

    deadtime = 0.0f;

    isJumping = false;
    grounded = false;

    fireTimer = 0;
    fireRate = .5;

    recharge = true;
    turbo = false;
    energy = 1000;
    maxEnergy = 1000;
    alive = true;
    charge = 100;
    maxCharge = 100;

    chargeRegen = 50;
    energyRegen = 100;

    moveCost = 0;
    jumpCost = 150;
    fireCost = maxCharge / 2;
    chargeCost = 200;

    bounces = 2;

    collisionRadius = 0.2;

    collisionPoints[0] = collisionRadius;
    collisionPoints[1] = 0;
    collisionPoints[2] = collisionRadius;

    collisionPoints[3] = -1 * collisionRadius;
    collisionPoints[4] = 0;
    collisionPoints[5] = -1 * collisionRadius;

    collisionPoints[6] = -1 * collisionRadius;
    collisionPoints[7] = 0;
    collisionPoints[8] = collisionRadius;

    collisionPoints[9] = collisionRadius;
    collisionPoints[10] = 0;
    collisionPoints[11] = -1 * collisionRadius;

    collisionPoints[12] = 0;
    collisionPoints[13] = 0;
    collisionPoints[14] = 0;

    collisionPoints[15] = 0;
    collisionPoints[16] = 0;
    collisionPoints[17] = 0;

    collisionPoints[18] = 0;
    collisionPoints[19] = 0;
    collisionPoints[20] = 0;

    x = LevelHandler::GetSingleton().start[0];
    y = 24;
    z = LevelHandler::GetSingleton().start[1];
    ;
    rx = 0;
    ry = 0;
    rz = 0;
    rtx = 0;
    rty = 0;
    rtz = 0;

    rotRate = 75;
    movRate = 5;
    fallRate = 2;
    jumpRate = .3;
}

void Tank::SetPosition(float _x, float _y, float _z)
{
    x = _x;
    y = _y;
    z = _z;
}

void Tank::RotBody(float rate)
{
    if (rate > 1)
    {
        rate = 1;
    }
    if (rate < -1)
    {
        rate = -1;
    }

    ry += rate * rotRate * GlobalTimer::dT;
}

void Tank::RotBody(bool forb)
{
    if (forb)
    {
        ry += rotRate * GlobalTimer::dT;
    }
    else
    {
        ry -= rotRate * GlobalTimer::dT;
    }
}

void Tank::RotTurret(float rate)
{
    float rtplus = rate * 10 * GlobalTimer::dT;

    if (rtplus > 2.5)
    {
        rtplus = 2.5;
    }
    if (rtplus < -2.5)
    {
        rtplus = -2.5;
    }

    rty += rtplus;
}

void Tank::RotBarrel(bool forb)
{
}

void Tank::Jump()
{

    if (charge > 0)
    {
        jumpTime += GlobalTimer::dT;

        charge -= jumpCost * GlobalTimer::dT;

        if (charge > 5)
        {
            vy = (jumpRate * jumpTime);
        }

        // y += vy*GlobalTimer::dT;

        // Check for float collision with upward offset during jump
        if (TankCollisionHelper::CheckFourPointFloatCollision(*this, 0.2f))
        {
            y = static_cast<int>(y) + .8;
            vy = 0;
        }

        // Jump damn it

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_JUMP, x, y - .2, z, 0, .5 * vy * GlobalTimer::dT, 0, rx, ry, rz, r, g, b, 1);
        if (!isJumping && id < 0)
        {
            App::GetSingleton().soundTask->PlayChannel(4);
        }
        isJumping = true;
    }
}

void Tank::NextFrame()
{

    if (!isJumping)
    {
        jumpTime = 0.0f;
    }

    bonusTime += GlobalTimer::dT;
    if (bonusTime > 1)
    {
        bonus = 0;
    }

    if (hitAlpha > 0)
    {
        hitAlpha -= 1 * GlobalTimer::dT;
    }

    if (TankHandler::GetSingleton().hitCombo[(-1 * id) - 1] != hitNum)
    {
        hitAlpha = 1.0;
    }

    hitNum = TankHandler::GetSingleton().hitCombo[(-1 * id) - 1];

    if (energy > maxEnergy && !App::GetSingleton().gameTask->debug)
    {
        energy = maxEnergy;
    }

    if (energy < maxEnergy / 2)
    {
        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x, y + .1, z, 0, .01, 0, 0, ry + rty, 90, .2, .2, .2, 1);
    }

    if (x >= 128 || x <= 0 || z >= 128 || z <= 0)
    {
        x = 64;
        z = 64;
    }
    if (y >= 27)
    {
        y--;
        x = 64;
        z = 64;
    }

    if (energy < 0 && alive)
    {
        alive = false;
        if (id < 0 && App::GetSingleton().gameTask->versus)
        {
            if (id == -1)
            {
                TankHandler::GetSingleton().wins[1]++;
            }
            else
            {
                TankHandler::GetSingleton().wins[0]++;
            }
        }
    }
    else
    {
        fireTimer += GlobalTimer::dT;
        Fall();
        if (charge < maxCharge && energy > maxEnergy / 3 && recharge)
        {
            charge += chargeRegen / 2 * GlobalTimer::dT;
            energy -= chargeCost * GlobalTimer::dT;
        }
        else if (energy >= maxEnergy && charge < maxCharge)
        {
            charge += chargeRegen / 2 * GlobalTimer::dT;
            energy -= chargeCost * GlobalTimer::dT;
        }
        if (energy < maxEnergy)
        {
            energy += energyRegen * GlobalTimer::dT;
        }
        if (charge < maxCharge)
        {
            charge += chargeRegen / 2 * GlobalTimer::dT;
        }
    }
}

bool Tank::Move(float rate)
{
    if (rate > 1.25)
    {
        rate = 1.25;
    }

    bool forb = true;

    if (rate < 0)
    {
        forb = false;
    }

    bool moved = true;

    vx = rate * (GlobalTimer::dT * movRate) * std::cosf(ry * DTR);
    vz = rate * (GlobalTimer::dT * movRate) * std::sinf(ry * DTR);

    x += vx;
    z += vz;

    // Check for collision using helper
    if (TankCollisionHelper::CheckFourPointCollision(*this))
    {
        if (!forb)
        {
            vx = -1 * vx;
            vz = -1 * vz;
        }

        int kx = 0;
        int kz = 2;

        bool done = false;

        // Find which specific collision point is colliding and create appropriate FX
        int which = TankCollisionHelper::FindCollidingPoint(*this);
        if (which >= 0 && which <= 3)
        {
            int xIndex = which * 3;
            int zIndex = xIndex + 2;
            FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[xIndex], y, z - vz + collisionPoints[zIndex], 0, 90, 90, r, g, b, 1);
        }
        else
        {
            // Fallback to manual checking if helper doesn't find the point
            if (LevelHandler::GetSingleton().PointCollision(x + collisionPoints[0], y, z + collisionPoints[2]))
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[0], y, z - vz + collisionPoints[2], 0, 90, 90, r, g, b, 1);
                which = 0;
            }
            else if (LevelHandler::GetSingleton().PointCollision(x + collisionPoints[3], y, z + collisionPoints[5]))
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[3], y, z - vz + collisionPoints[5], 0, 90, 90, r, g, b, 1);
                which = 1;
            }
            else if (LevelHandler::GetSingleton().PointCollision(x + collisionPoints[6], y, z + collisionPoints[8]))
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[6], y, z - vz + collisionPoints[8], 0, 90, 90, r, g, b, 1);
                which = 2;
            }
            else if (LevelHandler::GetSingleton().PointCollision(x + collisionPoints[9], y, z + collisionPoints[11]))
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[9], y, z - vz + collisionPoints[11], 0, 90, 90, r, g, b, 1);
                which = 3;
            }
        }

        kx = which * 3;
        kz = 2 + kx;

        x -= vx;
        z -= vz;

        if (static_cast<int>(x + collisionPoints[kx] + vx) != static_cast<int>(x + collisionPoints[kx]) && static_cast<int>(z + collisionPoints[kz] + vz) == static_cast<int>(z + collisionPoints[kz]))
        {
            z += vz / 2;
            done = true;
        }
        // else
        if (static_cast<int>(x + collisionPoints[kx] + vx) == static_cast<int>(x + collisionPoints[kx]) && static_cast<int>(z + collisionPoints[kz] + vz) != static_cast<int>(z + collisionPoints[kz]))
        {
            x += vx / 2;
            done = true;
        }
        // else
        if (static_cast<int>(x + collisionPoints[kx] + vx) != static_cast<int>(x + collisionPoints[kx]) && static_cast<int>(z + collisionPoints[kz] + vz) != static_cast<int>(z + collisionPoints[kz]))
        {
            if (LevelHandler::GetSingleton().PointCollision((x + vx + collisionPoints[kx]), y, z + collisionPoints[kz]) && !LevelHandler::GetSingleton().PointCollision(x + collisionPoints[kx], y, z + vz + collisionPoints[kz]))
            {
                z += vz / 2;
                done = true;
            }
            else
            {
                x += vx / 2;
                done = true;
            }
        }

        // Final collision check using helper
        if (TankCollisionHelper::CheckFourPointCollision(*this))
        {
            x -= vx;
            z -= vz;
        }

        moved = false;
    }

    charge -= rate * moveCost * GlobalTimer::dT;

    if (x >= 128 || x <= 0 || z >= 128 || z <= 0)
    {
        x = 64;
        z = 64;
    }

    return moved;
}

bool Tank::Move(bool forb)
{
    bool moved;

    vx = (GlobalTimer::dT * movRate) * std::cosf(ry * DTR);
    vz = (GlobalTimer::dT * movRate) * std::sinf(ry * DTR);

    if (forb)
    {
        x += vx;
        z += vz;
    }
    else
    {
        x -= vx;
        z -= vz;
    }
    moved = true;

    // TODO: Use rotated points (calculated here for the points at which treadmark FX should be spawned) for collision instead of world-aligned points.
    //  These are four points .25 units away from the center of the tank, at 90 degrees
    //  Point one:
    // float treadPointX = 0.25 * std::cosf((ry + 45) * DTR);
    // float treadPointZ = 0.25 * std::sinf((ry + 45) * DTR);
    //  Point two:
    // treadPointX = 0.25 * std::cosf((ry + 135) * DTR);
    // treadPointZ = 0.25 *  std::sinf((ry + 135) * DTR);
    //  Point three:
    // treadPointX = 0.25 * std::cosf((ry + 225) * DTR);
    // treadPointZ = 0.25 *  std::sinf((ry + 225) * DTR);
    //  Point four:
    // treadPointX = 0.25 * std::cosf((ry + 315) * DTR);
    // treadPointZ = 0.25 *  std::sinf((ry + 315) * DTR);

    if (isPlayer && grounded)
    {
        float treadPointX = 0.25f * std::cosf((ry + 45) * DTR);
        float treadPointZ = 0.25f * std::sinf((ry + 45) * DTR);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_RECTANGLE, x - vx + treadPointX, y - 0.18, z - vz + treadPointZ, 0, ry, 0, r, g, b, 1);

        treadPointX = 0.25f * std::cosf((ry + 315) * DTR);
        treadPointZ = 0.25f * std::sinf((ry + 315) * DTR);

        FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMALL_RECTANGLE, x - vx + treadPointX, y - 0.18, z - vz + treadPointZ, 0, ry, 0, r, g, b, 1);
    }

    // Check for collision using helper (includes center point + four corner points)
    if (LevelHandler::GetSingleton().PointCollision(x, y, z) || TankCollisionHelper::CheckFourPointCollision(*this))
    {
        if (!forb)
        {
            vx = -1 * vx;
            vz = -1 * vz;
        }

        int kx = 0;
        int kz = 2;

        // Find which specific collision point is colliding and create appropriate FX
        int which = TankCollisionHelper::FindCollidingPoint(*this);
        if (which >= 0 && which <= 3)
        {
            int xIndex = which * 3;
            int zIndex = xIndex + 2;
            FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[xIndex], y, z - vz + collisionPoints[zIndex], 0, 90, 90, r, g, b, 1);
        }
        else
        {
            // Fallback to manual checking if helper doesn't find the point
            if (LevelHandler::GetSingleton().PointCollision(x + collisionPoints[0], y, z + collisionPoints[2]))
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[0], y, z - vz + collisionPoints[2], 0, 90, 90, r, g, b, 1);
                which = 0;
            }
            else if (LevelHandler::GetSingleton().PointCollision(x + collisionPoints[3], y, z + collisionPoints[5]))
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[3], y, z - vz + collisionPoints[5], 0, 90, 90, r, g, b, 1);
                which = 1;
            }
            else if (LevelHandler::GetSingleton().PointCollision(x + collisionPoints[6], y, z + collisionPoints[8]))
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[6], y, z - vz + collisionPoints[8], 0, 90, 90, r, g, b, 1);
                which = 2;
            }
            else if (LevelHandler::GetSingleton().PointCollision(x + collisionPoints[9], y, z + collisionPoints[11]))
            {
                FXHandler::GetSingleton().CreateFX(FxType::TYPE_SMOKE, x - vx + collisionPoints[9], y, z - vz + collisionPoints[11], 0, 90, 90, r, g, b, 1);
                which = 3;
            }
        }

        kx = which * 3;
        kz = 2 + kx;

        x -= vx;
        z -= vz;

        if (static_cast<int>(x + collisionPoints[kx] + vx) != static_cast<int>(x + collisionPoints[kx]) && static_cast<int>(z + collisionPoints[kz] + vz) == static_cast<int>(z + collisionPoints[kz]))
        {
            z += vz / 2;
        }
        if (static_cast<int>(x + collisionPoints[kx] + vx) == static_cast<int>(x + collisionPoints[kx]) && static_cast<int>(z + collisionPoints[kz] + vz) != static_cast<int>(z + collisionPoints[kz]))
        {
            x += vx / 2;
        }
        if (static_cast<int>(x + collisionPoints[kx] + vx) != static_cast<int>(x + collisionPoints[kx]) && static_cast<int>(z + collisionPoints[kz] + vz) != static_cast<int>(z + collisionPoints[kz]))
        {
            if (LevelHandler::GetSingleton().PointCollision((x + vx + collisionPoints[kx]), y, z + collisionPoints[kz]) && !LevelHandler::GetSingleton().PointCollision(x + collisionPoints[kx], y, z + vz + collisionPoints[kz]))
            {
                z += vz / 2;
            }
            else
            {
                x += vx / 2;
            }
        }

        // Final collision check using helper
        if (TankCollisionHelper::CheckFourPointCollision(*this))
        {
            x -= vx;
            z -= vz;
        }

        moved = false;
    }

    if (x >= 128 || x <= 0 || z >= 128 || z <= 0)
    {
        x = 64;
        z = 64;
    }

    charge -= moveCost * GlobalTimer::dT;

    return moved;
}

void Tank::HandleInput()
{
    // Use the appropriate input handler based on current input mode
    if (inputHandler)
    {
        inputHandler->HandleInput(*this);
    }

    // Common debug controls that apply to all input modes
    if (InputTask::KeyDown(SDL_SCANCODE_I) && App::GetSingleton().gameTask->debug)
    {
        TankHandler::GetSingleton().special[0] += 100;
        energy += maxEnergy * 20;
        charge += maxCharge * 20;
    }

    if (InputTask::KeyDown(SDL_SCANCODE_HOME) && App::GetSingleton().gameTask->debug)
    {
        TankHandler::GetSingleton().special[0] = 10;
        energy = maxEnergy;
        charge = maxCharge;
    }

    // Allow switching to keyboard/mouse mode from other input modes
    if (InputTask::KeyDown(SDL_SCANCODE_K) && inputMode != InputMode::MODE_KEYBOARD_MOUSE)
    {
        SetInputMode(InputMode::MODE_KEYBOARD_MOUSE);
    }
}

void Tank::AI()
{
    static EnemyState state = EnemyState::STATE_TURN;

    bool p2target = false;

    dist = sqrt((x - TankHandler::GetSingleton().players[0].x) * (x - TankHandler::GetSingleton().players[0].x) + (z - TankHandler::GetSingleton().players[0].z) * (z - TankHandler::GetSingleton().players[0].z));

    if (TankHandler::GetSingleton().numPlayers > 1 && TankHandler::GetSingleton().players[1].alive)
    {
        float dist2 = sqrt((x - TankHandler::GetSingleton().players[1].x) * (x - TankHandler::GetSingleton().players[1].x) + (z - TankHandler::GetSingleton().players[1].z) * (z - TankHandler::GetSingleton().players[1].z));

        if (dist2 < dist || !TankHandler::GetSingleton().players[1].alive)
        {
            p2target = true;
            dist = dist2;
        }
    }

    if (!TankHandler::GetSingleton().players[1].alive)
        p2target = false;

    if (dist > (15 + 3 * (LevelHandler::GetSingleton().levelNumber - 48)))
    {
        state = EnemyState::STATE_WANDER;
    }
    else
    {
        if (energy > (maxEnergy / 2) && !App::GetSingleton().gameTask->versus)
        {
            state = EnemyState::STATE_HUNT;
        }
        else
        {
            if ((LevelHandler::GetSingleton().levelNumber - 48) < 2)
            {
                state = EnemyState::STATE_FEAR;
            }
            else
            {
                state = EnemyState::STATE_HUNT;
            }
        }
    }

    if (TankHandler::GetSingleton().tanks.size() == 1 || (TankHandler::GetSingleton().numAttackingTanks < (LevelHandler::GetSingleton().levelNumber - 47) && !App::GetSingleton().gameTask->versus))
    {
        state = EnemyState::STATE_HUNT;
        TankHandler::GetSingleton().numAttackingTanks++;
    }

    switch (state)
    {
    case EnemyState::STATE_WANDER:
        Wander();
        break;
    case EnemyState::STATE_FEAR:
        Fear();
        break;
    case EnemyState::STATE_TURN:
        RotBody(true);
        break;
    case EnemyState::STATE_HUNT:
        if (p2target)
        {
            Hunt(TankHandler::GetSingleton().players[1]);
        }
        else
        {
            Hunt(TankHandler::GetSingleton().players[0]);
        }

    default:
        RotTurret(false);
    }
}

void Tank::Wander()
{
    float angle = 20;
    float frames = 15;
    float xpp = x + (GlobalTimer::dT * frames * movRate) * std::cosf((ry - angle) * DTR);
    float zpp = z + (GlobalTimer::dT * frames * movRate) * std::sinf((ry - angle) * DTR);

    float xpp2 = x + (GlobalTimer::dT * frames * movRate) * std::cosf((ry + angle) * DTR);
    float zpp2 = z + (GlobalTimer::dT * frames * movRate) * std::sinf((ry + angle) * DTR);

    Move(true);

    if (LevelHandler::GetSingleton().PointCollision(xpp, y, zpp) || LevelHandler::GetSingleton().PointCollision(xpp2, y, zpp2))
    {
        if (LevelHandler::GetSingleton().GetTerrainHeight((unsigned int)xpp, (unsigned int)zpp) < (y + vy + 3) && charge > (maxCharge / 4) && LevelHandler::GetSingleton().GetTerrainHeight((unsigned int)x, (unsigned int)z) > (int)(y - 7))
        {
            Jump();
        }
        else
        {
            ry += 90;
        }
    }
}

void Tank::Fear()
{
    recharge = false;

    double ratio;

    ratio = (double)(z - TankHandler::GetSingleton().players[0].z) / (double)(x - TankHandler::GetSingleton().players[0].x);
    float ryp = toDegrees(atan(ratio));

    if (TankHandler::GetSingleton().players[0].x < x)
    {
        ryp += 180;
    }

    ryp -= 180;

    if (ryp < (ry - 5))
    {
        RotBody(false);
    }
    else
    {
        if (ryp > (ry + 5))
        {
            RotBody(true);
        }
    }

    ratio = (double)(z - TankHandler::GetSingleton().players[0].z) / (double)(x - TankHandler::GetSingleton().players[0].x);

    float rtyp = toDegrees(atan(ratio)) - ry;

    if (TankHandler::GetSingleton().players[0].x < x)
    {
        rtyp += 180;
    }

    rty = rtyp;

    float angle = 30;
    float frames = 20;
    float xpp = x + (GlobalTimer::dT * frames * movRate) * std::cosf((ry - angle) * DTR);
    float zpp = z + (GlobalTimer::dT * frames * movRate) * std::sinf((ry - angle) * DTR);

    float xpp2 = x + (GlobalTimer::dT * frames * movRate) * std::cosf((ry + angle) * DTR);
    float zpp2 = z + (GlobalTimer::dT * frames * movRate) * std::sinf((ry + angle) * DTR);

    if (LevelHandler::GetSingleton().PointCollision(xpp, y, zpp) || LevelHandler::GetSingleton().PointCollision(xpp2, y, zpp2))
    {
        if (LevelHandler::GetSingleton().GetTerrainHeight((int)xpp, (int)zpp) < (y + 3) && charge > (maxCharge / 4))
        {
            Jump();
        }
        else
        {
            ry += 30;
        }
    }

    if (!Move(true))
    {
        ry += 30;
    }
}

void Tank::Hunt(Tank &player)
{

    recharge = true;

    double ratio;
    float xpp;
    float zpp;

    ratio = (double)(z - player.z) / (double)(x - player.x);
    float ryp = toDegrees(atan(ratio));

    if (player.x < x)
    {
        ryp += 180;
    }

    if (ryp < (ry - 5))
    {
        RotBody(false);
    }
    else
    {
        if (ryp > (ry + 5))
            RotBody(true);
    }

    ratio = (double)(z - player.z) / (double)(x - player.x);

    float rtyp = toDegrees(atan(ratio)) - ry;

    if (player.x < x)
    {
        rtyp += 180;
    }

    rty = rtyp;

    xpp = x + (GlobalTimer::dT * 10 * movRate) * std::cosf(ry * DTR);
    zpp = z + (GlobalTimer::dT * 10 * movRate) * std::sinf(ry * DTR);

    if (LevelHandler::GetSingleton().PointCollision(xpp, y, zpp))
    {
        if (LevelHandler::GetSingleton().GetTerrainHeight((int)xpp, (int)zpp) < (y + 5) && charge > (maxCharge / 4))
        {
            Jump();
        }
        else
        {
            if (ryp < (ry - 5))
            {
                ry -= 30;
            }
            else
            {
                if (ryp > (ry + 5))
                    ry += 30;
            }
        }
    }
    if (dist > 10)
    {
        Move(true);
    }
    else
    {
        if (charge >= (maxCharge / 2) && (int)player.y == (int)y)
        {
            Fire(1);
        }
    }
}

void Tank::Draw() const
{
    if (alive)
    {
        TankRenderer::Draw(*this);
    }
    else
    {
        // Tank::Draw2();
    }
}

void Tank::Draw2()
{
    TankRenderer::Draw2(*this);
}
