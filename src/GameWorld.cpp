#include "GameWorld.h"
#include "TankHandler.h"
#include "BulletHandler.h"
#include "LevelHandler.h"
#include "FXHandler.h"
#include "FXEntity.h"
#include "Logger.h"

GameWorld::GameWorld() {
    Logger::Get().Write("GameWorld: Phase 2A - Initializing with FX management\n");
    initialized = true;
}

void GameWorld::Update() {
    // Phase 2A: Now actively manages FX entities
    effects.Update();
    
    // Future phases will add other entity updates here:
    // enemies.Update();
    // bullets.Update();  
    // items.Update();
    
    // Track that we're running
    if (!initialized) {
        initialized = true;
    }
}

void GameWorld::Reset() {
    Logger::Get().Write("GameWorld: Phase 2A - Reset with FX cleanup\n");
    
    // Phase 2A: Clear our FX entities
    effects.Clear();
    
    // Future: Will clear other entity managers
    // enemies.Clear();
    // bullets.Clear();
    // items.Clear();
    
    initialized = false;
}

bool GameWorld::AreAllEnemiesDead() const {
    // Phase 1: Delegate to existing TankHandler
    const auto& tanks = TankHandler::GetSingleton().tanks;
    return std::all_of(tanks.begin(), tanks.end(), 
        [](const Tank& tank) { return !tank.alive; });
}

bool GameWorld::AreAllPlayersDead() const {
    // Phase 1: Delegate to existing TankHandler
    const auto& players = TankHandler::GetSingleton().players;
    int numPlayers = TankHandler::GetSingleton().numPlayers;
    
    for (int i = 0; i < numPlayers; i++) {
        if (players[i].alive) {
            return false;
        }
    }
    return true;
}

int GameWorld::GetRemainingPlayers() const {
    // Phase 1: Delegate to existing TankHandler
    const auto& players = TankHandler::GetSingleton().players;
    int numPlayers = TankHandler::GetSingleton().numPlayers;
    int remaining = 0;
    
    for (int i = 0; i < numPlayers; i++) {
        if (players[i].alive) {
            remaining++;
        }
    }
    return remaining;
}

size_t GameWorld::GetEnemyCount() const {
    // Phase 1: Delegate to existing TankHandler
    const auto& tanks = TankHandler::GetSingleton().tanks;
    return std::count_if(tanks.begin(), tanks.end(),
        [](const Tank& tank) { return tank.alive; });
}

size_t GameWorld::GetBulletCount() const {
    // Phase 1: Delegate to existing BulletHandler
    return BulletHandler::GetSingleton().GetBullets().size();
}

size_t GameWorld::GetItemCount() const {
    // Phase 1: Delegate to existing LevelHandler
    const auto& items = LevelHandler::GetSingleton().items;
    return std::count_if(items.begin(), items.end(),
        [](const Item& item) { return item.alive; });
}

size_t GameWorld::GetEffectCount() const {
    // Phase 2A: Return count from both old system and new system
    size_t oldCount = FXHandler::GetSingleton().fx.size();
    size_t newCount = effects.GetAliveCount();
    return oldCount + newCount;
}

void GameWorld::CreateFX(FxType type, float x, float y, float z, 
                        float rx, float ry, float rz, 
                        float r, float g, float b, float a) {
    // Phase 2A-1: Create FX in new system
    effects.Create(type, x, y, z, rx, ry, rz, r, g, b, a);
}

void GameWorld::CreateFX(FxType type, float x, float y, float z, 
                        float dx, float dy, float dz,
                        float rx, float ry, float rz, 
                        float r, float g, float b, float a) {
    // Phase 2A-1: Create FX with velocity in new system
    effects.Create(type, x, y, z, dx, dy, dz, rx, ry, rz, r, g, b, a);
}
