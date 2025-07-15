#pragma once
#include <vector>
#include "Bullet.h"

class BulletHandler {
public:
    static BulletHandler& GetSingleton();

    void AddBullet(const Bullet& bullet);
    void NextFrame();
    void DrawBullets() const;
    void Clear();
    std::vector<Bullet>& GetBullets();
    const std::vector<Bullet>& GetBullets() const;

private:
    BulletHandler() = default;
    std::vector<Bullet> bullets;
};
