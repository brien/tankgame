#pragma once

#include <vector>
using namespace std;
#include "Singleton.h"
#include "Entity.h"
#include "Color.h"

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
    
    // Legacy methods for compatibility
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

class FXHandler : public Singleton<FXHandler>
{
public:
    FXHandler();
    ~FXHandler() = default;

    vector<FX> fx;

    void NextFrame();

    void CreateFX(FxType type, float _x, float _y, float _z, float _rx, float _ry, float _rz, const Color& color);
    void CreateFX(FxType type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, const Color& color);
    
    // Convenience overloads for backward compatibility
    void CreateFX(FxType type, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);
    void CreateFX(FxType type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);

    void ClearFX();
    
    // Interface to GameWorld
    void SetGameWorld(class GameWorld* world) { gameWorld = world; }
    
    // Get all FX for rendering (combines old and new systems)
    const std::vector<FX>& GetAllFX() const;

private:
    class GameWorld* gameWorld = nullptr;
    mutable std::vector<FX> unifiedFXView; // Mutable for const GetAllFX()
};
