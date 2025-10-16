#pragma once

#include "Entity.h"
#include "EntityManager.h"

// Forward declarations for existing classes
class Tank;
class Bullet;
class Item;
class FX;

/**
 * Central game world manager.
 * Phase 1: Runs alongside existing handlers for gradual migration.
 * Eventually will replace scattered entity management.
 */
class GameWorld {
public:
    GameWorld();
    ~GameWorld() = default;
    
    /**
     * Update all entities in the world.
     * Phase 1: This is mostly empty, just demonstrates the structure.
     */
    void Update();
    
    /**
     * Reset the world state (for level changes, game restart, etc.).
     */
    void Reset();
    
    /**
     * Query methods for GameTask to check game state.
     */
    bool AreAllEnemiesDead() const;
    bool AreAllPlayersDead() const;
    int GetRemainingPlayers() const;
    
    /**
     * Get entity counts for debugging/UI.
     */
    size_t GetEnemyCount() const;
    size_t GetBulletCount() const;
    size_t GetItemCount() const;
    size_t GetEffectCount() const;
    
private:
    // Phase 1: Start with empty entity managers
    // These will be populated as we migrate systems
    // EntityManager<Tank> enemies;     // Will replace TankHandler::tanks
    // EntityManager<Bullet> bullets;   // Will replace BulletHandler
    // EntityManager<Item> items;       // Will replace LevelHandler::items  
    // EntityManager<FX> effects;       // Will replace FXHandler::fx
    
    // Phase 1: Just track if we're initialized
    bool initialized = false;
};
