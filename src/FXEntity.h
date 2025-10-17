#pragma once

#include "Entity.h"
#include "FXHandler.h"

/**
 * Entity wrapper around the existing FX class.
 * Allows FX to be managed by the unified entity system while
 * preserving all existing FX functionality.
 */
class FXEntity : public Entity {
private:
    FX fx;  // Wrap existing FX class
    
public:
    /**
     * Constructor matching FXHandler::CreateFX signature.
     */
    FXEntity(FxType type, float x, float y, float z, 
             float rx, float ry, float rz, 
             float r, float g, float b, float a)
        : fx(type, x, y, z, rx, ry, rz, r, g, b, a) {
        // Sync Entity base class position with FX position
        this->x = x;
        this->y = y; 
        this->z = z;
        this->alive = fx.alive;
    }
    
    /**
     * Constructor with velocity - matching second FXHandler::CreateFX signature.
     */
    FXEntity(FxType type, float x, float y, float z, 
             float dx, float dy, float dz,
             float rx, float ry, float rz, 
             float r, float g, float b, float a)
        : fx(type, x, y, z, dx, dy, dz, rx, ry, rz, r, g, b, a) {
        // Sync Entity base class position with FX position
        this->x = x;
        this->y = y;
        this->z = z;
        this->alive = fx.alive;
    }
    
    /**
     * Update the wrapped FX and sync state.
     */
    void Update() override {
        fx.Update();
        
        // Sync position from FX to Entity base class
        x = fx.x;
        y = fx.y;
        z = fx.z;
        
        // Sync alive status - FX determines when it should die
        alive = fx.alive;
    }
    
    /**
     * No special cleanup needed - FX handles its own lifecycle.
     */
    void OnDestroy() override {
        // FX class handles its own cleanup in destructor
    }
    
    /**
     * Expose underlying FX for rendering system.
     * The rendering system expects FX objects, so we provide access.
     */
    const FX& GetFX() const { return fx; }
    FX& GetFX() { return fx; }
    
    /**
     * Get FX type for filtering/sorting.
     */
    FxType GetType() const { return fx.type; }
};
