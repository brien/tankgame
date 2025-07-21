#pragma once

#include <vector>
using namespace std;
#include "Singleton.h"

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

class FX
{
public:
    FX();
    FX(FxType type, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);
    FX(FxType type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);
    ~FX() = default;
    void SetMaxTime();
    void Update();

    bool alive;

    FxType type;

    float r, g, b, a;
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

    void CreateFX(FxType type, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);
    void CreateFX(FxType type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);

    void ClearFX();
};
