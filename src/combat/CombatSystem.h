#pragma once

#include "../events/CollisionEvents.h"
#include "../events/Event.h"
#include "../Tank.h"
#include "../Bullet.h"
#include "../Item.h"

/**
 * Handles combat-related collision responses.
 * Processes collision events and applies appropriate game logic.
 */
class CombatSystem {
public:
    CombatSystem() = default;
    ~CombatSystem() = default;
    
    void Initialize();
    void Shutdown();
    
private:
    // Event handlers for different collision types
    void OnBulletCollision(const BulletCollisionEvent& event);
    void OnBulletLevelCollision(const BulletLevelCollisionEvent& event);
    void OnBulletOutOfBounds(const BulletOutOfBoundsEvent& event);
    void OnBulletTimeout(const BulletTimeoutEvent& event);
    
    // Combat logic helpers
    void HandleBulletTankCollision(class Bullet* bullet, class Tank* tank);
    void HandleBulletPlayerCollision(class Bullet* bullet, class Tank* player);
    void ApplyTankDamage(class Tank* tank, float damage, class Bullet* source);
    void UpdatePlayerCombos(int playerIndex, class Tank* target, class Bullet* bullet);
    void CreateCollisionEffects(float x, float y, float z, float r, float g, float b, float angle);
    void ResetPlayerCombo(int playerIndex);
};
