#pragma once

#include "Entity.h"
#include "igtl_qmesh.h"
#include "BulletHandler.h"

#include <vector>
#include <queue>
#include <memory>
using namespace std;
#include "Bullet.h"

// Forward declarations
class TankRenderer;
class InputHandler;
class GameWorld;

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

class Tank : public Entity
{
    // Allow the collision helper to access private members
    friend class TankCollisionHelper;
    
public:
    Tank();
    ~Tank();
    
    // Delete copy constructor and copy assignment operator
    Tank(const Tank&) = delete;
    Tank& operator=(const Tank&) = delete;
    
    // Move constructor and move assignment operator
    Tank(Tank&&) noexcept;
    Tank& operator=(Tank&&) noexcept;

    bool isPlayer;

    // === REFACTORED: Clear two-concept system ===
    // 1. HEALTH - Tank's structural integrity (survival)
    float health;           // Was: energy (but actually represented health)
    float maxHealth;        // Was: maxEnergy
    float healthRegen;      // Health regeneration rate (slow)
    
    // 2. ENERGY - Used for actions (firing, jumping, special abilities)
    float energy;           // Was: charge (now clearly for actions)
    float maxEnergy;        // Was: maxCharge
    float energyRegen;      // Was: chargeRegen (energy regeneration rate)
    
    // Action costs (how much energy each action consumes)
    float fireCost;         // Energy cost to fire
    float jumpCost;         // Energy cost to jump
    float moveCost;         // Energy cost for movement (if applicable)
    float specialCost;      // Was: chargeCost (for special abilities)

    float fireTimer;
    float fireRate;
    int bounces;
    int attack;

    bool alive;

    int id;
    
    // Entity interface implementation
    void Update() override { 
        if (!isPlayer && id >= 0) {
            // Enemy tanks (id >= 0, isPlayer = false) need AI updates
            // Debug: Uncomment this to verify enemy tanks are updating
            // printf("Enemy tank %d updating (isPlayer=%d, id=%d)\n", id, isPlayer, id);
            AI(); 
        }
        NextFrame(); 
    }
    bool IsAlive() const override { return alive; }
    void OnDestroy() override { Die(); }
    void Kill() override { alive = false; }

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
    bool isGrounded;
    float jumpTime;
    bool turbo;         // Turbo mode flag (consumes extra energy)

    bool PointCollision(float cx, float cy, float cz) const;

    void NextFrame();

    void Draw() const;
    void Draw2();
    
    // GameWorld access for bullet creation
    void SetGameWorld(GameWorld* world) { gameWorld = world; }

private:
    // Helper method to create bullets through GameWorld
    void CreateBullet(int id, float attack, TankType type1, TankType type2, int bounces, float dTpressed,
                     float r, float g, float b, float r2, float g2, float b2,
                     float x, float y, float z, float rx, float ry, float rz);

public:
    // === COMPATIBILITY LAYER (temporary) ===
    // These provide backward compatibility during migration
    // Old "energy" field was actually health
    [[deprecated("Use health instead")]] float& GetOldEnergyRef() { return health; }
    [[deprecated("Use maxHealth instead")]] float& GetOldMaxEnergyRef() { return maxHealth; }
    // Old "charge" field is now energy  
    [[deprecated("Use energy instead")]] float& GetOldChargeRef() { return energy; }
    [[deprecated("Use maxEnergy instead")]] float& GetOldMaxChargeRef() { return maxEnergy; }

    int bonus;
    float bonusTime;
    float deadtime;
    float hitAlpha;
    int hitNum;

private:
    GameWorld* gameWorld = nullptr;
};
