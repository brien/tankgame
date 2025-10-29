#include "GameWorld.h"
#include "Tank.h"
#include "Bullet.h"
#include "FXHandler.h"
#include "Item.h"
#include "Logger.h"
#include "events/Events.h"
#include "events/CollisionEvents.h"

GameWorld::GameWorld() = default;

void GameWorld::Initialize() {
    // Initialize systems
    collisionSystem.Initialize();
    combatSystem.Initialize();
    
    // Set up event handlers
    SetupEventHandlers();
    
    Logger::Get().Write("GameWorld initialized with collision and combat systems\n");
}

void GameWorld::Shutdown() {
    combatSystem.Shutdown();
    collisionSystem.Shutdown();
}

void GameWorld::Update() {
    // Update collision system first
    collisionSystem.Update();
    
    // Update all entity types with collision system cleanup
    UpdateEntitiesWithCleanup(tanks);
    UpdateEntitiesWithCleanup(bullets);
    UpdateEntitiesWithCleanup(effects);
    UpdateEntitiesWithCleanup(items);

    // Handle interactions
    HandleCollisions();
    HandleItemCollection();
}

void GameWorld::Clear() {
    tanks.Clear();
    bullets.Clear(); 
    effects.Clear();
    items.Clear();
}

Bullet* GameWorld::CreateBullet(int id, float attack, TankType type1, TankType type2, int bounces, float dTpressed, 
                               float r, float g, float b, float r2, float g2, float b2,
                               float x, float y, float z, float rx, float ry, float rz) {
    Bullet* bullet = bullets.Create(id, attack, type1, type2, bounces, dTpressed, r, g, b, r2, g2, b2, x, y, z, rx, ry, rz);
    
    // Register bullet with collision system
    if (bullet) {
        collisionSystem.RegisterEntity(bullet, CollisionShape3D(CollisionShape3D::SPHERE, 0.3f), CollisionLayer::BULLETS);
    }
    
    return bullet;
}

Tank* GameWorld::CreateTank() {
    Tank* tank = tanks.Create();
    
    // Register tank with collision system 
    if (tank) {
        CollisionLayer layer = tank->isPlayer ? CollisionLayer::PLAYER_TANKS : CollisionLayer::ENEMY_TANKS;
        // Use collisionRadius * 2 to match original collision detection
        collisionSystem.RegisterEntity(tank, CollisionShape3D(CollisionShape3D::CUSTOM, 0.4f), layer);
    }
    
    return tank;
}

FX* GameWorld::CreateFX(FxType type, float x, float y, float z, float rx, float ry, float rz, float r, float g, float b, float a) {
    return effects.Create(type, x, y, z, rx, ry, rz, r, g, b, a);
}

FX* GameWorld::CreateFX(FxType type, float x, float y, float z, float dx, float dy, float dz, float rx, float ry, float rz, float r, float g, float b, float a) {
    return effects.Create(type, x, y, z, dx, dy, dz, rx, ry, rz, r, g, b, a);
}

Item* GameWorld::CreateItem(float x, float y, float z, TankType type) {
    return items.Create(x, y, z, type);
}

void GameWorld::HandleCollisions() {
    // Centralized collision handling
    // Move collision logic here from handlers
}

void GameWorld::HandleItemCollection() {
    // Move item collection logic here from LevelHandler
}

void GameWorld::SetupEventHandlers() {
    // Handle FX creation events from combat system
    Events::GetBus().Subscribe<CreateFXEvent>([this](const CreateFXEvent& event) {
        OnCreateFXEvent(event);
    });
}

template<typename T>
void GameWorld::UpdateEntitiesWithCleanup(EntityManager<T>& manager) {
    auto& entities = const_cast<std::vector<std::unique_ptr<T>>&>(manager.GetEntities());
    
    // Update living entities
    for (auto& entity : entities) {
        if (entity->IsAlive()) {
            entity->Update();
        }
    }
    
    // Remove dead entities and unregister from collision system
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [this](const auto& entity) {
                if (!entity->IsAlive()) {
                    // Unregister from collision system before destroying
                    collisionSystem.UnregisterEntity(entity.get());
                    entity->OnDestroy();  // Call cleanup
                    return true;
                }
                return false;
            }),
        entities.end()
    );
}

void GameWorld::OnCreateFXEvent(const CreateFXEvent& event) {
    if (event.hasVelocity) {
        CreateFX(static_cast<FxType>(event.fxType), 
                event.x, event.y, event.z, 
                event.dx, event.dy, event.dz,
                event.rx, event.ry, event.rz, 
                event.r, event.g, event.b, event.a);
    } else {
        CreateFX(static_cast<FxType>(event.fxType), 
                event.x, event.y, event.z, 
                event.rx, event.ry, event.rz, 
                event.r, event.g, event.b, event.a);
    }
}
