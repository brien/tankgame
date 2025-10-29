#pragma once

#include "Event.h"
#include "../Entity.h"
#include <vector>

// Collision layer flags for filtering
enum class CollisionLayer : unsigned int {
    NONE = 0,
    LEVEL = 1 << 0,
    PLAYER_TANKS = 1 << 1,
    ENEMY_TANKS = 1 << 2,
    BULLETS = 1 << 3,
    ITEMS = 1 << 4,
    ALL_TANKS = PLAYER_TANKS | ENEMY_TANKS,
    ALL = 0xFFFFFFFF
};

// Operator overloads for CollisionLayer
inline CollisionLayer operator|(CollisionLayer a, CollisionLayer b) {
    return static_cast<CollisionLayer>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

inline CollisionLayer operator&(CollisionLayer a, CollisionLayer b) {
    return static_cast<CollisionLayer>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
}

inline bool operator!=(CollisionLayer a, CollisionLayer b) {
    return static_cast<unsigned int>(a) != static_cast<unsigned int>(b);
}

// === COLLISION QUERY EVENTS ===

/**
 * Query for point collision detection.
 * Used to check if a specific point intersects with any registered colliders.
 */
struct PointCollisionQuery : public EventBase<PointCollisionQuery> {
    float x, y, z;
    CollisionLayer layerMask;
    Entity* excludeEntity;  // Optional entity to exclude from check
    
    // Results (filled by collision system)
    mutable bool result = false;
    mutable Entity* hitEntity = nullptr;
    
    PointCollisionQuery(float x, float y, float z, CollisionLayer layers = CollisionLayer::LEVEL, Entity* exclude = nullptr)
        : x(x), y(y), z(z), layerMask(layers), excludeEntity(exclude) {}
};

/**
 * Query for sphere collision detection.
 * Returns all entities within a spherical area.
 */
struct SphereCollisionQuery : public EventBase<SphereCollisionQuery> {
    float x, y, z, radius;
    CollisionLayer layerMask;
    Entity* excludeEntity;
    
    // Results (filled by collision system)
    mutable std::vector<Entity*> results;
    
    SphereCollisionQuery(float x, float y, float z, float radius, CollisionLayer layers, Entity* exclude = nullptr)
        : x(x), y(y), z(z), radius(radius), layerMask(layers), excludeEntity(exclude) {}
};

/**
 * Query to get level boundaries.
 */
struct GetLevelBoundsQuery : public EventBase<GetLevelBoundsQuery> {
    mutable float sizeX = 0;
    mutable float sizeZ = 0;
};

// === COLLISION DETECTION EVENTS ===

/**
 * Event posted when a bullet collides with something.
 * Allows combat system to handle the response.
 */
struct BulletCollisionEvent : public EventBase<BulletCollisionEvent> {
    Entity* bullet;
    Entity* target;
    float contactX, contactY, contactZ;
    
    BulletCollisionEvent(Entity* bullet, Entity* target, float x, float y, float z)
        : bullet(bullet), target(target), contactX(x), contactY(y), contactZ(z) {}
};

/**
 * Event posted when a bullet goes out of bounds.
 */
struct BulletOutOfBoundsEvent : public EventBase<BulletOutOfBoundsEvent> {
    Entity* bullet;
    float x, y, z;
    
    BulletOutOfBoundsEvent(Entity* bullet, float x, float y, float z)
        : bullet(bullet), x(x), y(y), z(z) {}
};

/**
 * Event posted when a bullet times out.
 */
struct BulletTimeoutEvent : public EventBase<BulletTimeoutEvent> {
    Entity* bullet;
    float timeAlive;
    
    BulletTimeoutEvent(Entity* bullet, float time)
        : bullet(bullet), timeAlive(time) {}
};

/**
 * Event posted when a bullet collides with level geometry.
 */
struct BulletLevelCollisionEvent : public EventBase<BulletLevelCollisionEvent> {
    Entity* bullet;
    float x, y, z;
    float xMovement, zMovement;  // Movement delta that caused collision
    float originalAngle;
    
    BulletLevelCollisionEvent(Entity* bullet, float x, float y, float z, float xpp, float zpp, float angle)
        : bullet(bullet), x(x), y(y), z(z), xMovement(xpp), zMovement(zpp), originalAngle(angle) {}
};

// === GAME EVENTS FOR REACTIONS ===

/**
 * Event for creating visual effects.
 */
struct CreateFXEvent : public EventBase<CreateFXEvent> {
    int fxType;  // FxType enum value
    float x, y, z;
    float rx, ry, rz;
    float dx, dy, dz;  // Optional velocity
    float r, g, b, a;  // Color
    bool hasVelocity;
    
    // Constructor for simple FX
    CreateFXEvent(int type, float x, float y, float z, float rx, float ry, float rz, float r, float g, float b, float a)
        : fxType(type), x(x), y(y), z(z), rx(rx), ry(ry), rz(rz), 
          dx(0), dy(0), dz(0), r(r), g(g), b(b), a(a), hasVelocity(false) {}
    
    // Constructor for FX with velocity
    CreateFXEvent(int type, float x, float y, float z, float dx, float dy, float dz, float rx, float ry, float rz, float r, float g, float b, float a)
        : fxType(type), x(x), y(y), z(z), rx(rx), ry(ry), rz(rz), 
          dx(dx), dy(dy), dz(dz), r(r), g(g), b(b), a(a), hasVelocity(true) {}
};

/**
 * Event for tank damage application.
 */
struct TankDamagedEvent : public EventBase<TankDamagedEvent> {
    Entity* tank;
    Entity* source;  // What caused the damage
    float damage;
    float newHealth;
    bool isDead;
    
    TankDamagedEvent(Entity* tank, Entity* source, float damage, float newHealth, bool dead)
        : tank(tank), source(source), damage(damage), newHealth(newHealth), isDead(dead) {}
};
