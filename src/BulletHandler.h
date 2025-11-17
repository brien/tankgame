#pragma once
#include <vector>
#include "Bullet.h"

class BulletHandler {
public:
    static BulletHandler& GetSingleton();

    void AddBullet(const Bullet& bullet);
    void NextFrame();
    void Clear();
    std::vector<Bullet>& GetBullets();
    const std::vector<Bullet>& GetBullets() const;
    
    // Interface to GameWorld
    void SetGameWorld(class GameWorld* world) { gameWorld = world; }
    class GameWorld* GetGameWorld() const { return gameWorld; }
    
    // Get all bullets for rendering (combines old and new systems)
    const std::vector<Bullet>& GetAllBullets() const;

private:
    BulletHandler() = default;
    std::vector<Bullet> bullets;
    class GameWorld* gameWorld = nullptr;
    mutable std::vector<Bullet> unifiedBulletView; // Mutable for const GetAllBullets()
};
