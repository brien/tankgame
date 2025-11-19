#include "BulletHandler.h"
#include "GameWorld.h"

BulletHandler& BulletHandler::GetSingleton() {
    static BulletHandler instance;
    return instance;
}

void BulletHandler::AddBullet(const Bullet& bullet) {
    // Legacy method - no longer used, bullets created directly through GameWorld
    // This method exists only for interface compatibility during transition
}

void BulletHandler::NextFrame() {
    // No-op - GameWorld handles all bullet updates through EntityManager
    // Legacy bullets.clear() to ensure no accumulation during transition
    bullets.clear();
}

// Legacy DrawBullets method removed - bullets now handled by NewBulletRenderer

void BulletHandler::Clear() {
    bullets.clear();
}

std::vector<Bullet>& BulletHandler::GetBullets() {
    return bullets;
}

const std::vector<Bullet>& BulletHandler::GetBullets() const {
    return bullets;
}

const std::vector<Bullet>& BulletHandler::GetAllBullets() const {
    if (gameWorld) {
        // During transition: return empty vector since rendering should use GameWorld directly
        // TODO: Remove this method entirely when rendering is updated
        static const std::vector<Bullet> emptyBullets;
        return emptyBullets;
    } else {
        // Fallback to old system (should not be used in practice)
        return bullets;
    }
}
