#include "GameWorld.h"
#include "Tank.h"
#include "Bullet.h"
#include "FXHandler.h"
#include "Item.h"
#include "Logger.h"

GameWorld::GameWorld() = default;

void GameWorld::Update() {
    // Update all entity types using unified EntityManager
    tanks.Update();
    bullets.Update();
    effects.Update();
    items.Update();

    // Handle interactions
    HandleCollisions();
    HandleItemCollection();
}

void GameWorld::Clear() {
    tanks.Clear();
    bullets.Clear(); 
    effects.Clear();
    items.Clear();
}

Tank* GameWorld::CreateTank() {
    return tanks.Create();
}

Bullet* GameWorld::CreateBullet(int id, float attack, TankType type1, TankType type2, int bounces, float dTpressed, 
                               float r, float g, float b, float r2, float g2, float b2,
                               float x, float y, float z, float rx, float ry, float rz) {
    return bullets.Create(id, attack, type1, type2, bounces, dTpressed, r, g, b, r2, g2, b2, x, y, z, rx, ry, rz);
}

FX* GameWorld::CreateFX(FxType type, float x, float y, float z, float rx, float ry, float rz, float r, float g, float b, float a) {
    return effects.Create(type, x, y, z, rx, ry, rz, r, g, b, a);
}

FX* GameWorld::CreateFX(FxType type, float x, float y, float z, float dx, float dy, float dz, float rx, float ry, float rz, float r, float g, float b, float a) {
    return effects.Create(type, x, y, z, dx, dy, dz, rx, ry, rz, r, g, b, a);
}

Item* GameWorld::CreateItem(float x, float y, float z, TankType type) {
    return items.Create(x, y, z, type);
}

void GameWorld::HandleCollisions() {
    // Centralized collision handling
    // Move collision logic here from handlers
}

void GameWorld::HandleItemCollection() {
    // Move item collection logic here from LevelHandler
}
