#include "GameWorld.h"
#include "TankHandler.h"
#include "BulletHandler.h"
#include "LevelHandler.h"
#include "FXHandler.h"
#include "Logger.h"

GameWorld::GameWorld() {
    Logger::Get().Write("GameWorld: Initializing alongside existing systems\n");
    initialized = true;
}

void GameWorld::Update() {
    // Phase 1: Empty implementation - existing handlers still do the work
    // This demonstrates the structure and allows GameTask to call gameWorld.Update()
    
    // Future phases will move entity updates here:
    // enemies.Update();
    // bullets.Update();  
    // items.Update();
    // effects.Update();
    
    // For now, just track that we're running
    if (!initialized) {
        initialized = true;
    }
}

void GameWorld::Reset() {
    Logger::Get().Write("GameWorld: Reset called\n");
    
    // Phase 1: Delegate to existing systems
    // Future: Will clear our own entity managers
    // enemies.Clear();
    // bullets.Clear();
    // items.Clear();
    // effects.Clear();
    
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
    // Phase 1: Delegate to existing FXHandler
    return FXHandler::GetSingleton().fx.size();
}
