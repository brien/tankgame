#pragma once

#include "Entity.h"
#include "Color.h"

/**
 * FX type enumeration for visual effects.
 * Defines the different types of visual effects that can be created in the game.
 */
enum class FxType
{
    TYPE_ZERO = 0,
    TYPE_JUMP = 1,
    TYPE_SMOKE = 2,
    TYPE_THREE = 3,
    TYPE_SMALL_SQUARE = 4,
    TYPE_STAR = 5,
    TYPE_DEATH = 6,
    TYPE_SMALL_RECTANGLE = 7,
    FX_TYPE_COUNT = 8
};

/**
 * Visual effect entity.
 * Represents temporary visual effects like explosions, smoke, particle trails, etc.
 * Effects are managed by GameWorld's EntityManager and have automatic lifecycle.
 */
class FX : public Entity
{
public:
    FX();
    FX(FxType type, float _x, float _y, float _z, float _rx, float _ry, float _rz, const Color& color);
    FX(FxType type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, const Color& color);
    ~FX() = default;
    
    // Entity interface implementation
    void Update() override;
    bool IsAlive() const override { return alive; }
    void OnDestroy() override {}
    void Kill() override { alive = false; }
    
    // Helper to set effect duration based on type
    void SetMaxTime();

    bool alive;

    FxType type;

    Color color;
    float x, y, z;
    float rx, ry, rz;
    float dx, dy, dz;

    float time;
    float maxTime;
};
