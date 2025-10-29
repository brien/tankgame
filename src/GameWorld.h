#pragma once

#include "Entity.h"
#include "EntityManager.h"
#include "collision/CollisionSystem.h"
#include "combat/CombatSystem.h"

// Forward declarations for existing classes
class Tank;
class Bullet;
class Item;
class FX;
enum class FxType;
enum class TankType;

/**
 * Central game world manager.
 * Owns EntityManager instances for all entity types and coordinates their lifecycle.
 * Handlers now serve as interfaces to GameWorld rather than managing entities directly.
 */
class GameWorld {
public:
    GameWorld();
    ~GameWorld() = default;
    
    void Update();
    void Clear();

    // Entity creation interfaces (for handlers to use)
    Tank* CreateTank();
    Bullet* CreateBullet(int id, float attack, TankType type1, TankType type2, int bounces, float dTpressed, 
                        float r, float g, float b, float r2, float g2, float b2,
                        float x, float y, float z, float rx, float ry, float rz);
    FX* CreateFX(FxType type, float x, float y, float z, float rx, float ry, float rz, float r, float g, float b, float a);
    FX* CreateFX(FxType type, float x, float y, float z, float dx, float dy, float dz, float rx, float ry, float rz, float r, float g, float b, float a);
    Item* CreateItem(float x, float y, float z, TankType type);

    // Access for rendering and other systems
    const auto& GetTanks() const { return tanks.GetEntities(); }
    const auto& GetBullets() const { return bullets.GetEntities(); }
    const auto& GetFX() const { return effects.GetEntities(); }
    const auto& GetItems() const { return items.GetEntities(); }

    // Initialize/shutdown systems
    void Initialize();
    void Shutdown();
    
    // System accessors
    CollisionSystem& GetCollisionSystem() { return collisionSystem; }

private:
    EntityManager<Tank> tanks;
    EntityManager<Bullet> bullets;
    EntityManager<FX> effects;
    EntityManager<Item> items;
    
    // Game systems
    CollisionSystem collisionSystem;
    CombatSystem combatSystem;

    void HandleCollisions();
    void HandleItemCollection();
    
    // Event handlers
    void SetupEventHandlers();
    void OnCreateFXEvent(const struct CreateFXEvent& event);
    
    // Helper for updating entities with collision cleanup
    template<typename T>
    void UpdateEntitiesWithCleanup(EntityManager<T>& manager);
};
