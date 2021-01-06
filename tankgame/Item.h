#ifndef ITEM_H_INCLUDED
#define ITEM_H_INCLUDED

#include "Tank.h"

class Item
{
public:
    
    Item();
    Item(float nx, float ny, float nz, TankType ntype);
    
    bool alive;
    float x, y, z;
    float rx, ry, rz;
    
    float r, g, b;
    
    TankType type;

    void Draw();
};


#endif