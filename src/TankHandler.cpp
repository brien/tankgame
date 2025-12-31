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
#include "TankTypeManager.h"
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
    TankTypeManager::Initialize();
    
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
            newTank->Init();
            newTank->isPlayer = false; // Ensure it's marked as enemy tank
            newTank->identity = TankIdentity::Enemy(i);
            SetEnemyPosition(*newTank, i);
            SetEnemyType(*newTank, i);
            newTank->jumpCost = 0; // Enemy tanks don't pay jump cost
        }
    }
}

Tank TankHandler::CreateEnemyTank(int index)
{
    Tank tank;
    tank.Init();
    
    SetEnemyPosition(tank, index);
    SetEnemyType(tank, index);
    
    tank.jumpCost = 0;
    
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
        tank.x = LevelHandler::GetSingleton().enemy[index % MAX_ENEMY_SPAWN_POSITIONS][0] + index % 2;
        tank.z = LevelHandler::GetSingleton().enemy[index % MAX_ENEMY_SPAWN_POSITIONS][1] + index % 2;
    }
    
    tank.y = LevelHandler::GetSingleton().GetTerrainHeight((int)tank.x, (int)tank.z);
    tank.ry = index * ROTATION_STEP_DEGREES;
    tank.identity = TankIdentity::Enemy(index);
}

void TankHandler::SetEnemyType(Tank& tank, int index)
{
    int levelOffset = LevelHandler::GetSingleton().levelNumber - 48;
    
    if (levelOffset == 0)
    {
        tank.SetType(static_cast<TankType>(1 + index % 3), TankType::TYPE_GREY);
        if (tank.type1 == TankType::TYPE_BLUE)
            tank.SetType(TankType::TYPE_YELLOW, TankType::TYPE_GREY);
        if (tank.identity.GetEnemyIndex() == 4)
        {
            tank.SetType(TankType::TYPE_BLUE, TankType::TYPE_GREY);
            tank.attack = tank.attack / 3;
        }
    }
    else
    {
        if (levelOffset == 2 || levelOffset == 5 || levelOffset == 12)
        {
            tank.SetType(static_cast<TankType>(index % 4 + 1), static_cast<TankType>(index % 3 + 1));
        }
        else
        {
            tank.SetType(static_cast<TankType>(1 + index % 3), static_cast<TankType>(index % 3));
        }
    }
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
