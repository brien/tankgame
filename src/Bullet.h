//
//  Bullet.hpp
//  tankgame
//
//

#pragma once

#include "Entity.h"
#include "Color.h"
#include "TankIdentity.h"

class Tank;
enum class TankType;
enum class FxType;

class Bullet : public Entity
{

public:
    Bullet();
    Bullet(const TankIdentity& ownerIdentity, float power,
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
    
    // GameWorld access for FX creation
    void SetGameWorld(class GameWorld* world) { gameWorld = world; }
    
    // Descriptive FX helper methods (encapsulate effect creation logic)
    void CreateWallImpactFX(float xpp, float zpp);
    
    // Low-level FX creation (used by helper methods)
    void CreateFX(FxType type, float x, float y, float z, float rx, float ry, float rz, float r, float g, float b, float a);
    void CreateFX(FxType type, float x, float y, float z, float dx, float dy, float dz, float rx, float ry, float rz, float r, float g, float b, float a);
    
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
    const TankIdentity& GetOwnerIdentity() const { return ownerIdentity; }
    int GetTankId() const { return ownerIdentity.GetLegacyId(); }  // Backward compatibility
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
    TankIdentity ownerIdentity = TankIdentity::Enemy(0);  // Tank that fired this bullet

    TankType type1;
    TankType type2;

    float dT = 0.0f;
    float maxdT = 100.0f;

    int numbounces = 0;
    int maxbounces = 0;

    bool alive = true;
    
    class GameWorld* gameWorld = nullptr;
};
