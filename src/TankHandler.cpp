#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#else
// If building on Linux:
#include <GL/gl.h>
#endif

#include "TankHandler.h"
#include "LevelHandler.h"
#include "GlobalTimer.h"
#include "math.h"
#include "App.h"
#include "GameWorld.h"
#include "Logger.h"
#include <string>
#include <utility>
#include <algorithm>
#include <vector>

// Constructor initializes enemy tank management state
TankHandler::TankHandler()
    : numAttackingTanks(0)
{
}

void TankHandler::Init()
{
    
    // Initialize enemy tanks only
    // Player tanks are managed by PlayerManager
    InitializeEnemyTanks();
}

// All player initialization and controller setup handled by PlayerManager

void TankHandler::InitializeEnemyTanks()
{
    int enemyCount = LevelHandler::GetSingleton().GetEnemyCountForLevel(LevelHandler::GetSingleton().levelNumber);
    
    for (int i = 0; i < enemyCount; ++i)
    {
        // Delegate to GameWorld - create enemy tank there
        Tank* newTank = gameWorld->CreateTank();
        if (newTank) {
            newTank->isPlayer = false; // Ensure it's marked as enemy tank
            newTank->identity = TankIdentity::Enemy(i);
            SetEnemyType(*newTank, i);
            newTank->Init();
            SetEnemyPosition(*newTank, i);
        }
    }
}

Tank TankHandler::CreateEnemyTank(int index)
{
    Tank tank(gameWorld ? gameWorld->GetGameplayDefinitions() : nullptr);
    SetEnemyType(tank, index);
    tank.Init();
    SetEnemyPosition(tank, index);
    
    
    // Set GameWorld reference
    tank.SetGameWorld(gameWorld);
    
    return tank;
}

void TankHandler::SetEnemyPosition(Tank& tank, int index)
{
    if (index < MAX_ENEMY_SPAWN_POSITIONS)
    {
        tank.x = LevelHandler::GetSingleton().enemy[index][0];
        tank.z = LevelHandler::GetSingleton().enemy[index][1];
    }
    else
    {
        tank.x = LevelHandler::GetSingleton().enemy[index % MAX_ENEMY_SPAWN_POSITIONS][0] + (index % 2) * tank.GetGameplayDefinitions().enemyBehavior.overflowSpawnOffset;
        tank.z = LevelHandler::GetSingleton().enemy[index % MAX_ENEMY_SPAWN_POSITIONS][1] + (index % 2) * tank.GetGameplayDefinitions().enemyBehavior.overflowSpawnOffset;
    }
    
    tank.y = LevelHandler::GetSingleton().GetTerrainHeight((int)tank.x, (int)tank.z);
    tank.ry = index * tank.GetGameplayDefinitions().enemyBehavior.spawnRotationStepDegrees;
    tank.identity = TankIdentity::Enemy(index);
}

void TankHandler::SetEnemyType(Tank& tank, int index)
{
    int levelOffset = LevelHandler::GetSingleton().levelNumber - 48;
    const auto& rules = gameWorld ? gameWorld->GetGameplayDefinitions()->enemy : Gameplay::BuiltInDefinitions().enemy;
    tank.ConfigureStats({true, 2, levelOffset == 0 && index == rules.openingLevelWeakenedIndex
        ? rules.openingLevelAttackScale : 1.0f});
    
    const bool advanced = std::find(rules.advancedLevelOffsets.begin(), rules.advancedLevelOffsets.end(), levelOffset)
        != rules.advancedLevelOffsets.end();
    const auto& cycle = levelOffset == 0 ? rules.openingTypes : advanced ? rules.advancedTypes : rules.regularTypes;
    const auto types = cycle.at(static_cast<size_t>(index) % cycle.size());
    if (levelOffset == 0 && index == rules.openingLevelWeakenedIndex)
        tank.SetType(rules.openingWeakenedPrimary, rules.openingWeakenedSecondary);
    else
        tank.SetType(types.first, types.second);
}

// Removed: All player-related update methods
// - NextFrame() - No longer needed, enemies update via GameWorld
// - UpdatePlayerCombos/States/Targeting/VersusMode() - Handled by PlayerManager
// - UpdateEnemyTanks() - Handled by GameWorld's EntityManager

std::vector<const Tank*> TankHandler::GetAllEnemyTanks() const {
    // Return enemy tanks from GameWorld
    std::vector<const Tank*> enemyTanks;
    
    const auto& worldTanks = gameWorld->GetTanks();
    enemyTanks.reserve(worldTanks.size());
    
    for (const auto& tankPtr : worldTanks) {
        if (tankPtr && tankPtr->IsAlive()) {
            enemyTanks.push_back(tankPtr.get());
        }
    }
    
    return enemyTanks;
}

void TankHandler::SetGameWorld(GameWorld* world) {
    gameWorld = world;
}

// All player-related functionality now handled by PlayerManager
