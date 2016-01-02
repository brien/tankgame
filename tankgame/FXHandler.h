#ifndef FXHANDLER_H_INCLUDED
#define FXHANDLER_H_INCLUDED

#include <vector>
using namespace std;
#include "Singleton.h"


class FX
{
public:
    FX();
    FX(int type, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);
    FX(int type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);
    ~FX();
    void Update();
    void Draw();
    
    
    bool alive;
    
    int type;
    
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
    ~FXHandler();
    
    vector<FX> fx;
    
    void NextFrame();
    void Draw();
    
    void CreateFX(int type, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);
    void CreateFX(int type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a);
};

#endif