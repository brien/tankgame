#pragma once

#include "Entity.h"
#include "EntityManager.h"
#include "FXHandler.h"  // For FxType enum
#include "FXEntity.h"   // Full include needed for EntityManager<FXEntity>

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
     * Phase 2A: Now includes FX management alongside existing systems.
     */
    void Update();
    
    /**
     * Create visual effects (replaces FXHandler::CreateFX calls).
     * Phase 2A: Start redirecting FX creation to GameWorld.
     */
    void CreateFX(FxType type, float x, float y, float z, 
                  float rx, float ry, float rz, 
                  float r, float g, float b, float a);
    
    void CreateFX(FxType type, float x, float y, float z, 
                  float dx, float dy, float dz,
                  float rx, float ry, float rz, 
                  float r, float g, float b, float a);
    
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
    
    /**
     * Access to FX entities for rendering system.
     * Phase 2A: Provide access to new FX entities.
     */
    const EntityManager<FXEntity>& GetEffects() const { return effects; }
    
private:
    // Phase 2A: Start with FX entity manager (actively used)
    EntityManager<FXEntity> effects;     // New FX management system
    
    // Phase 1: Other entity managers still commented out for future phases
    // EntityManager<Tank> enemies;     // Will replace TankHandler::tanks
    // EntityManager<Bullet> bullets;   // Will replace BulletHandler
    // EntityManager<Item> items;       // Will replace LevelHandler::items  
    
    // Phase 1: Just track if we're initialized
    bool initialized = false;
};
