#include "BulletHandler.h"
#include "BulletRenderer.h"

BulletHandler& BulletHandler::GetSingleton() {
    static BulletHandler instance;
    return instance;
}

void BulletHandler::AddBullet(const Bullet& bullet) {
    bullets.push_back(bullet);
}

void BulletHandler::NextFrame() {
    for (auto it = bullets.begin(); it != bullets.end(); ) {
        if (it->IsAlive()) {
            it->NextFrame();
            ++it;
        } else {
            it = bullets.erase(it);
        }
    }
}

void BulletHandler::DrawBullets() const {
    BulletRenderer::Draw(bullets);
}

void BulletHandler::Clear() {
    bullets.clear();
}

std::vector<Bullet>& BulletHandler::GetBullets() {
    return bullets;
}

const std::vector<Bullet>& BulletHandler::GetBullets() const {
    return bullets;
}
