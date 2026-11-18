#pragma once

#include "Entity.h"
#include "igtl_qmesh.h"
#include "gameplay/GameplayDefinitions.h"
#include "Color.h"
#include "TankIdentity.h"

#include <vector>
#include <queue>
#include <memory>
using namespace std;
#include "Bullet.h"

// Forward declarations
class TankRenderer;
class InputHandler;
class GameWorld;
enum class FxType;

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
    explicit Tank(std::shared_ptr<const Gameplay::GameplayDefinitions> definitions = nullptr);
    ~Tank();
    
    // Delete copy constructor and copy assignment operator
    Tank(const Tank&) = delete;
    Tank& operator=(const Tank&) = delete;
    
    // Move constructor and move assignment operator
    Tank(Tank&&) noexcept;
    Tank& operator=(Tank&&) noexcept;

    bool isPlayer;

    // Mutable resources/cooldown are independent of resolved tuning.
    float health;
    float energy;
    float fireTimer;
    const Gameplay::ResolvedTankConfig& GetStats() const { return effectiveStats; }
    const Gameplay::GameplayDefinitions& GetGameplayDefinitions() const { return Definitions(); }

    bool alive;

    TankIdentity identity;
    
    // Backward-compatible accessor for gradual migration
    int GetId() const { return identity.GetLegacyId(); }
    
    // Entity interface implementation
    void Update() override { 
        if (!isPlayer && identity.IsEnemy()) {
            // Enemy tanks (identity.IsEnemy(), isPlayer = false) need AI updates
            // Debug: Uncomment this to verify enemy tanks are updating
            // printf("Enemy tank %d updating (isPlayer=%d)\n", identity.GetEnemyIndex(), isPlayer);
            AI(); 
        }
        NextFrame(); 
    }
    bool IsAlive() const override { return alive; }
    void OnDestroy() override { Die(); }
    void Kill() override { alive = false; }

    void Init();
    // Refresh tuning without healing or resetting movement/cooldown state.
    void ConfigureStats(const Gameplay::TankContext& context);
    float GetTurboEnergyPerSecond() const { return statContext.enemy ? 0 : Definitions().tank.turboEnergyPerSecond; }

    float x, y, z;
    float vx, vy, vz;
    void SetPosition(float _x, float _y, float _z);

    float collisionPoints[21] = {};

    int control;
    InputMode inputMode;
    unsigned int jid;
    std::unique_ptr<InputHandler> inputHandler;

    float rx, ry, rz, rr, rrl;
    float rtx, rty, rtz;
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
    void SetInputMode(InputMode mode);

    TankType GetPrimaryType() const { return type1; }
    TankType GetSecondaryType() const { return type2; }

    // Dynamic color methods - replace hardcoded r,g,b fields
    Color GetPrimaryColor() const;
    Color GetSecondaryColor() const;
    
    // Convenience methods for RGB access (for legacy compatibility)
    void GetPrimaryColorRGB(float& r, float& g, float& b) const { 
        const auto color = GetPrimaryColor();
        r = color.r; g = color.g; b = color.b;
    }
    void GetSecondaryColorRGB(float& r, float& g, float& b) const { 
        const auto color = GetSecondaryColor();
        r = color.r; g = color.g; b = color.b;
    }

    float dist;

    bool isJumping;
    bool isGrounded;
    float jumpTime;
    bool turbo;         // Turbo mode flag (consumes extra energy)

    bool PointCollision(float cx, float cy, float cz) const;

    void NextFrame();
    
    // GameWorld access for bullet and FX creation
    void SetGameWorld(GameWorld* world) { gameWorld = world; }
    
    // Descriptive FX helper methods (encapsulate effect creation logic)
    void CreateDeathExplosionFX();
    
    // Low-level FX creation (used by helper methods)
    void CreateFX(FxType type, float x, float y, float z, float rx, float ry, float rz, float r, float g, float b, float a);
    void CreateFX(FxType type, float x, float y, float z, float dx, float dy, float dz, float rx, float ry, float rz, float r, float g, float b, float a);

private:
    // Helper method to create bullets through GameWorld
    void CreateBullet(const TankIdentity& ownerIdentity, float attack, TankType type1, TankType type2, int bounces, float dTpressed,
                     const Color& primaryColor, const Color& secondaryColor,
                     float x, float y, float z, float rx, float ry, float rz, bool isSpecial = false);

public:
    int bonus;
    float bonusTime;
    float deadtime;
    float hitAlpha;
    int hitNum;

private:
    TankType type1;
    TankType type2;
    Gameplay::ResolvedTankConfig effectiveStats;
    GameWorld* gameWorld = nullptr;
    std::shared_ptr<const Gameplay::GameplayDefinitions> gameplayDefinitions;
    Gameplay::TankContext statContext;
    const Gameplay::GameplayDefinitions& Definitions() const;
    void ApplyStats(const Gameplay::ResolvedTankConfig& config, bool refill);
};
