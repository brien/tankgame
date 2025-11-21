//
//  Bullet.hpp
//  tankgame
//
//

#pragma once

#include "Entity.h"
#include "Color.h"

class Tank;
enum class TankType;

class Bullet : public Entity
{

public:
    Bullet();
    Bullet(int tid, float power,
           TankType type1, TankType type2,
           int maxbounces,
           float dTpressed,
           const Color& primaryColor,
           const Color& secondaryColor,
           float x, float y, float z,
           float rx, float ry, float rz);
    ~Bullet() = default;
    
    // Entity interface implementation
    void Update() override { NextFrame(); }
    bool IsAlive() const override { return alive; }
    void OnDestroy() override {}
    void Kill() override { alive = false; }
    
    // Legacy methods for compatibility
    void NextFrame();

    // Accessor methods for alive member
    void SetAlive(bool isAlive) { alive = isAlive; }

    // Accessor methods for rendering data extraction
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }
    float GetVX() const { return vx; }
    float GetVY() const { return vy; }
    float GetVZ() const { return vz; }
    float GetRX() const { return rx; }
    float GetRY() const { return ry; }
    float GetRZ() const { return rz; }
    float GetR() const { return primaryColor.r; }
    float GetG() const { return primaryColor.g; }
    float GetB() const { return primaryColor.b; }
    float GetR2() const { return secondaryColor.r; }
    float GetG2() const { return secondaryColor.g; }
    float GetB2() const { return secondaryColor.b; }
    const Color& GetPrimaryColor() const { return primaryColor; }
    const Color& GetSecondaryColor() const { return secondaryColor; }
    float GetPower() const { return power; }
    TankType GetType1() const { return type1; }
    TankType GetType2() const { return type2; }
    int GetTankId() const { return tankId; }
    float GetMoveRate() const { return moveRate; }
    int GetBounces() const { return maxbounces; }
    float GetDT() const { return dT; }
    bool GetIsSpecial() const { return isSpecial; }
    
    // Setters needed by CombatSystem
    void SetPower(float newPower) { power = newPower; }
    
    // Collision handling methods (level collision still needed for bullet physics)
    void HandleLevelCollision(float xpp, float zpp, float ory);

private:
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float vx = 0.0f, vy = 0.0f, vz = 0.0f;
    float rx = 0.0f, ry = 0.0f, rz = 0.0f;
    float dty = 0.0f;

    Color primaryColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
    Color secondaryColor = Color(0.5f, 0.5f, 0.5f, 1.0f);

    float moveRate = 33.0f;
    float power = 0.0f;

    bool isSpecial = false;

    int id = 0;
    int tankId = 0;

    TankType type1;
    TankType type2;

    float dT = 0.0f;
    float maxdT = 100.0f;

    int numbounces = 0;
    int maxbounces = 0;

    bool alive = true;
};
