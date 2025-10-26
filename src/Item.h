#pragma once

#include "Entity.h"
#include "Tank.h"

class Item : public Entity
{
public:
    Item();
    Item(float nx, float ny, float nz, TankType ntype);

    bool alive;
    float x, y, z;        // Position
    float rx, ry, rz;     // Rotation

    float r, g, b;        // Color

    TankType type;
    
    // Entity interface implementation
    void Update() override;
    bool IsAlive() const override { return alive; }
    void OnDestroy() override {}
    void Kill() override { alive = false; }
};