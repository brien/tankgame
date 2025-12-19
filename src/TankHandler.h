#pragma once

#include <array>
#include <vector>
#include "Tank.h"
#include "Singleton.h"

class TankHandler : public Singleton<TankHandler>
{
public:
    TankHandler();
    ~TankHandler() = default;

    void Init();

    // Constants to replace magic numbers
    static constexpr int MAX_PLAYERS = 2;
    static constexpr int MAX_ENEMY_SPAWN_POSITIONS = 16;
    
    // Controller detection constants
    static constexpr float SPECIAL_CHARGE_THRESHOLD_DIVISOR = 5.0f; // fireCost / 5
    static constexpr int ROTATION_STEP_DEGREES = 45;
    
    // Player positioning constants
    static constexpr int VERSUS_ENEMY_POSITION_1 = 8;
    static constexpr int VERSUS_ENEMY_POSITION_2 = 9;

    // REMOVED: Player management arrays fully migrated to PlayerManager
    // All player data (combo, special, comboNum, hitCombo, wins) now accessed via:
    //   PlayerManager::GetCombos(), GetSpecialCharges(), GetComboNumbers(), GetHitCombos(), GetWins()
    // Or directly through Player objects via PlayerManager::GetPlayer(index)
    
    // Interface to GameWorld (for enemy tanks only)
    void SetGameWorld(class GameWorld* world);
    
    // Get all enemy tanks for rendering
    std::vector<const Tank*> GetAllEnemyTanks() const;

    // Enemy tank configuration
    int numAttackingTanks = 0;

private:
    class GameWorld* gameWorld = nullptr;
    mutable std::vector<const Tank*> unifiedEnemyView; // Mutable for const GetAllEnemyTanks()
    
    // Enemy tank initialization and management
    void InitializeEnemyTanks();
    
    // Helper functions for enemy tank creation
    Tank CreateEnemyTank(int index);
    void SetEnemyPosition(Tank& tank, int index);
    void SetEnemyType(Tank& tank, int index);
};
