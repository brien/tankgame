#pragma once

#include "igtl_qmesh.h"

#include <vector>
#include <queue>
#include <memory>
using namespace std;
#include "Bullet.h"

// Forward declarations
class TankRenderer;
class InputHandler;

enum class EnemyState
{
    STATE_WANDER,
    STATE_FEAR,
    STATE_TURN,
    STATE_BLANK,
    STATE_OTHER,
    STATE_HUNT,
    ENEMY_STATE_COUNT
};

enum class TankType
{
    TYPE_GREY,
    TYPE_RED,
    TYPE_BLUE,
    TYPE_YELLOW,
    TYPE_PURPLE,
    TANK_TYPE_COUNT
};

enum class InputMode
{
    MODE_KEYBOARD_MOUSE,
    MODE_JOYSTICK_GENERIC,
    MODE_EXTREME_3D,
    MODE_OTHER,
    MODE_NINTENDO_GC,
    INPUT_MODE_COUNT
};

// Forward declaration for helper class
class TankCollisionHelper;

class Tank
{
    // Allow the collision helper to access private members
    friend class TankCollisionHelper;
    
public:
public:
    Tank();
    ~Tank();
    
    // Delete copy constructor and copy assignment operator
    Tank(const Tank&) = delete;
    Tank& operator=(const Tank&) = delete;
    
    // Move constructor and move assignment operator
    Tank(Tank&&) noexcept;
    Tank& operator=(Tank&&) noexcept;

    vector<Bullet> bullets;
    queue<Bullet> bulletq;

    bool isPlayer;

    bool turbo;
    bool recharge;
    float charge;
    float maxCharge;
    float chargeRegen;

    float fireCost;
    float jumpCost;
    float moveCost;
    float chargeCost;

    float fireTimer;
    float fireRate;
    int bounces;
    int attack;

    bool alive;

    float energy;
    float maxEnergy;
    float energyRegen;

    int id;

    void Init();

    float x, y, z;
    float vx, vy, vz;
    void SetPosition(float _x, float _y, float _z);

    float collisionPoints[21];
    float collisionRadius;

    int control;
    InputMode inputMode;
    unsigned int jid;
    std::unique_ptr<InputHandler> inputHandler;

    float rx, ry, rz, rr, rrl;
    float rtx, rty, rtz;
    float rotRate, movRate, jumpRate;
    float fallRate;
    void Fall();
    void Jump();
    void HandleInput();
    void AI();

    void Hunt(Tank &player);
    void Fear();
    void Wander();

    void RotBody(bool forb);
    void RotBody(float rate);
    void RotTurret(float rate);
    void RotBarrel(bool forb);
    bool Move(float rate);
    bool Move(bool forb);
    void Fire(float dTpressed);
    void Special(float dTpressed);
    void Die();

    void SetType(TankType t1, TankType t2);
    void SetTankColors(TankType t1, TankType t2);
    void SetInputMode(InputMode mode);

    TankType type1;
    TankType type2;

    float r;
    float g;
    float b;

    float r2;
    float g2;
    float b2;

    float dist;

    bool isJumping;
    bool grounded;
    float jumpTime;

    bool PointCollision(float cx, float cy, float cz);

    void NextFrame();

    void Draw() const;
    void Draw2();

    int bonus;
    float bonusTime;
    float deadtime;
    float hitAlpha;
    int hitNum;
};
