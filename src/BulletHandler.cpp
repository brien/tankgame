#include "BulletHandler.h"
#include "GameWorld.h"

BulletHandler& BulletHandler::GetSingleton() {
    static BulletHandler instance;
    return instance;
}

void BulletHandler::AddBullet(const Bullet& bullet) {
    if (gameWorld) {
        // Delegate to GameWorld - extract all parameters from the bullet
        gameWorld->CreateBullet(bullet.GetTankId(), bullet.GetPower(), 
                               bullet.GetType1(), bullet.GetType2(), 
                               bullet.GetBounces(), 1.0f, // dTpressed - use default
                               bullet.GetR(), bullet.GetG(), bullet.GetB(),
                               bullet.GetR2(), bullet.GetG2(), bullet.GetB2(),
                               bullet.GetX(), bullet.GetY(), bullet.GetZ(),
                               bullet.GetRX(), bullet.GetRY(), bullet.GetRZ());
    } else {
        // Fallback to old system during transition
        bullets.push_back(bullet);
    }
}

void BulletHandler::NextFrame() {
    if (gameWorld) {
        // GameWorld handles updates, we just need to sync for rendering
        // The GameWorld's EntityManager will handle the update/cleanup
        return;
    }
    
    // Legacy update loop for fallback
    for (auto it = bullets.begin(); it != bullets.end(); ) {
        if (it->IsAlive()) {
            it->NextFrame();
            ++it;
        } else {
            it = bullets.erase(it);
        }
    }
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
        // Return unified view of both old bullets and GameWorld bullets
        unifiedBulletView.clear();
        
        // Add old system bullets
        unifiedBulletView.insert(unifiedBulletView.end(), bullets.begin(), bullets.end());
        
        // Add GameWorld bullets by dereferencing the unique_ptrs
        const auto& worldBullets = gameWorld->GetBullets();
        for (const auto& bulletPtr : worldBullets) {
            if (bulletPtr && bulletPtr->IsAlive()) {
                unifiedBulletView.push_back(*bulletPtr);
            }
        }
        
        return unifiedBulletView;
    } else {
        // Fallback to old system
        return bullets;
    }
}
