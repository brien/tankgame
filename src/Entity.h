#pragma once

/**
 * Base class for all game entities.
 * Provides common interface for lifecycle management.
 */
class Entity {
public:
    virtual ~Entity() = default;
    
    /**
     * Update entity state for one frame.
     * Called once per frame for living entities.
     */
    virtual void Update() = 0;
    
    /**
     * Check if entity is still alive.
     * Dead entities will be removed from the world.
     */
    virtual bool IsAlive() const { return alive; }
    
    /**
     * Called when entity is about to be destroyed.
     * Override to handle cleanup logic (effects, sounds, etc.).
     */
    virtual void OnDestroy() {}
    
    /**
     * Kill this entity.
     * Will be removed from world on next update.
     */
    virtual void Kill() { alive = false; }
    
    // Common position data
    float x = 0.0f, y = 0.0f, z = 0.0f;
    
protected:
    bool alive = true;
};
