#include "CollisionSystem.h"
#include "../events/Events.h"
#include "../LevelHandler.h"
#include "../TankHandler.h"
#include "../Item.h"
#include "../Bullet.h"
#include "../Tank.h"
#include "../Logger.h"
#include <cmath>

CollisionSystem::CollisionSystem() {
}

void CollisionSystem::Initialize() {
    Logger::Get().Write("CollisionSystem::Initialize() - Subscribing to collision query events\n");
    
    // Subscribe to collision query events
    Events::GetBus().Subscribe<PointCollisionQuery>([this](const PointCollisionQuery& query) {
        OnPointCollisionQuery(query);
    });
    
    Events::GetBus().Subscribe<SphereCollisionQuery>([this](const SphereCollisionQuery& query) {
        OnSphereCollisionQuery(query);
    });
    
    Events::GetBus().Subscribe<GetLevelBoundsQuery>([this](const GetLevelBoundsQuery& query) {
        OnGetLevelBoundsQuery(query);
    });
    
    Logger::Get().Write("CollisionSystem::Initialize() - Subscriptions complete\n");
}

void CollisionSystem::Update() {
    // Update cached positions and spatial structures if needed
    // For now, we'll do simple linear checks
    
    // Invalidate bounds cache periodically or when level changes
    // For simplicity, we'll keep it valid for now
}

void CollisionSystem::Shutdown() {
    registeredEntities.clear();
}

void CollisionSystem::RegisterEntity(Entity* entity, const CollisionShape3D& shape, CollisionLayer layer) {
    if (!entity) return;
    
    CollisionEntry entry;
    entry.entity = entity;
    entry.shape = shape;
    entry.layer = layer;
    entry.lastX = 0;  // Will be updated in queries
    entry.lastY = 0;
    entry.lastZ = 0;
    
    registeredEntities[entity] = entry;
}

void CollisionSystem::UnregisterEntity(Entity* entity) {
    if (!entity) return;
    registeredEntities.erase(entity);
}

void CollisionSystem::UpdateEntityShape(Entity* entity, const CollisionShape3D& newShape) {
    if (!entity) return;
    
    auto it = registeredEntities.find(entity);
    if (it != registeredEntities.end()) {
        it->second.shape = newShape;
    }
}

bool CollisionSystem::CheckPointCollision(float x, float y, float z, CollisionLayer layerMask, Entity* exclude) const {
    // First check level collision if requested
    if ((layerMask & CollisionLayer::LEVEL) != CollisionLayer::NONE) {
        if (CheckLevelCollision(x, y, z)) {
            return true;
        }
    }
    
    // Check registered entities
    for (const auto& pair : registeredEntities) {
        Entity* entity = pair.first;
        const CollisionEntry& entry = pair.second;
        
        if (entity == exclude) continue;
        if ((entry.layer & layerMask) == CollisionLayer::NONE) continue;
        if (!entity->IsAlive()) continue;
        
        // Get entity position (assuming Tank-like interface for now)
        Tank* tank = dynamic_cast<Tank*>(entity);
        if (tank) {
            if (tank->PointCollision(x, y, z)) {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<Entity*> CollisionSystem::CheckSphereCollision(float x, float y, float z, float radius, CollisionLayer layerMask, Entity* exclude) const {
    std::vector<Entity*> results;
    
    // Check registered entities
    for (const auto& pair : registeredEntities) {
        Entity* entity = pair.first;
        const CollisionEntry& entry = pair.second;
        
        if (entity == exclude) continue;
        if ((entry.layer & layerMask) == CollisionLayer::NONE) continue;
        if (!entity->IsAlive()) continue;
        
        // Get entity position and check collision
        Tank* tank = dynamic_cast<Tank*>(entity);
        if (tank) {
            // Use sphere-to-sphere collision detection
            float dx = x - tank->x;
            float dy = y - tank->y;
            float dz = z - tank->z;
            float distanceSquared = dx*dx + dy*dy + dz*dz;
            float combinedRadius = radius + entry.shape.radius;
            
            if (distanceSquared <= combinedRadius * combinedRadius) {
                results.push_back(entity);
            }
        }
    }
    
    return results;
}

void CollisionSystem::OnPointCollisionQuery(const PointCollisionQuery& query) {
    Logger::Get().Write("CollisionSystem::OnPointCollisionQuery - pos=(%.2f, %.2f, %.2f)\n", query.x, query.y, query.z);
    
    query.result = CheckPointCollision(query.x, query.y, query.z, query.layerMask, query.excludeEntity);
    
    Logger::Get().Write("CollisionSystem::OnPointCollisionQuery - result=%d\n", query.result);
    
    if (query.result) {
        // Find which entity was hit (for more detailed queries)
        for (const auto& pair : registeredEntities) {
            Entity* entity = pair.first;
            const CollisionEntry& entry = pair.second;
            
            if (entity == query.excludeEntity) continue;
            if ((entry.layer & query.layerMask) == CollisionLayer::NONE) continue;
            if (!entity->IsAlive()) continue;
            
            Tank* tank = dynamic_cast<Tank*>(entity);
            if (tank && tank->PointCollision(query.x, query.y, query.z)) {
                query.hitEntity = entity;
                break;
            }
        }
    }
}

void CollisionSystem::OnSphereCollisionQuery(const SphereCollisionQuery& query) {
    Logger::Get().Write("CollisionSystem::OnSphereCollisionQuery - pos=(%.2f, %.2f, %.2f) radius=%.2f\n", 
                       query.x, query.y, query.z, query.radius);
    
    query.results = CheckSphereCollision(query.x, query.y, query.z, query.radius, query.layerMask, query.excludeEntity);
    
    Logger::Get().Write("CollisionSystem::OnSphereCollisionQuery - found %zu entities\n", query.results.size());
}

void CollisionSystem::OnGetLevelBoundsQuery(const GetLevelBoundsQuery& query) {
    if (!boundsValid) {
        cachedSizeX = LevelHandler::GetSingleton().sizeX;
        cachedSizeZ = LevelHandler::GetSingleton().sizeZ;
        boundsValid = true;
    }
    
    query.sizeX = cachedSizeX;
    query.sizeZ = cachedSizeZ;
}

bool CollisionSystem::CheckLevelCollision(float x, float y, float z) const {
    return LevelHandler::GetSingleton().PointCollision(x, y, z);
}

bool CollisionSystem::PointVsPoint(float x1, float y1, float z1, float x2, float y2, float z2, float threshold) const {
    float dx = x1 - x2;
    float dy = y1 - y2;
    float dz = z1 - z2;
    return (dx * dx + dy * dy + dz * dz) <= (threshold * threshold);
}

bool CollisionSystem::PointVsSphere(float px, float py, float pz, float sx, float sy, float sz, float radius) const {
    return PointVsPoint(px, py, pz, sx, sy, sz, radius);
}

bool CollisionSystem::SphereVsSphere(float x1, float y1, float z1, float r1, float x2, float y2, float z2, float r2) const {
    return PointVsPoint(x1, y1, z1, x2, y2, z2, r1 + r2);
}
