#pragma once

#include "../events/CollisionEvents.h"
#include "../Entity.h"
#include <unordered_map>
#include <vector>

/**
 * Shape information for collision detection.
 */
struct CollisionShape3D {
    enum Type { POINT, SPHERE, CUSTOM } type;
    float radius;
    
    CollisionShape3D(Type t = POINT, float r = 0.5f) : type(t), radius(r) {}
};

/**
 * Centralized collision detection system.
 * Handles collision queries and maintains spatial information.
 */
class CollisionSystem {
public:
    CollisionSystem();
    ~CollisionSystem() = default;
    
    void Initialize();
    void Update();  // Update spatial grid and detect collisions
    void Shutdown();
    
    // Entity registration
    void RegisterEntity(Entity* entity, const CollisionShape3D& shape, CollisionLayer layer);
    void UnregisterEntity(Entity* entity);
    void UpdateEntityShape(Entity* entity, const CollisionShape3D& newShape);
    
    // Direct collision queries (for immediate results)
    bool CheckPointCollision(float x, float y, float z, CollisionLayer layerMask, Entity* exclude = nullptr) const;
    std::vector<Entity*> CheckSphereCollision(float x, float y, float z, float radius, CollisionLayer layerMask, Entity* exclude = nullptr) const;

private:
    struct CollisionEntry {
        Entity* entity;
        CollisionShape3D shape;
        CollisionLayer layer;
        float lastX, lastY, lastZ;  // Cached position
    };
    
    std::unordered_map<Entity*, CollisionEntry> registeredEntities;
    
    // Event handlers
    void OnPointCollisionQuery(const PointCollisionQuery& query);
    void OnSphereCollisionQuery(const SphereCollisionQuery& query);
    void OnGetLevelBoundsQuery(const GetLevelBoundsQuery& query);
    
    // Collision detection algorithms
    bool PointVsPoint(float x1, float y1, float z1, float x2, float y2, float z2, float threshold) const;
    bool PointVsSphere(float px, float py, float pz, float sx, float sy, float sz, float radius) const;
    bool SphereVsSphere(float x1, float y1, float z1, float r1, float x2, float y2, float z2, float r2) const;
    
    // Level collision (interfaces with LevelHandler for now)
    bool CheckLevelCollision(float x, float y, float z) const;
    
    // Cache level bounds
    mutable float cachedSizeX = 0;
    mutable float cachedSizeZ = 0;
    mutable bool boundsValid = false;
};
