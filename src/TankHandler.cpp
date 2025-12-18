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

// Reordered member initializer list to match declaration order in TankHandler.h to fix -Wreorder warning
TankHandler::TankHandler()
    : players{}, combo{0, 0}, special{0, 0}, comboNum{0, 0}, hitCombo{0, 0}, wins{0, 0}, closest(0.0f), isInputJoy(false), difficultySetting(0), numPlayers(1), numAttackingTanks(0)
{
}

void TankHandler::Init()
{
    TankTypeManager::Initialize();
    InitializePlayerTanks();
    InitializePlayerControls();
    InitializeEnemyTanks();
    
    // Log controller setup for debugging (only in debug builds)
    #ifdef DEBUG
    LogControllerSetup();
    #endif
}

void TankHandler::InitializePlayerTanks()
{
    combo[0] = 0;
    combo[1] = 0;
    special[0] += (float)comboNum[0];
    special[1] += (float)comboNum[1];
    comboNum[0] = 0;
    comboNum[1] = 0;

    for (int j = 0; j < numPlayers; j++)
    {
        players[j].Init();
        players[j].isPlayer = true;

        if (App::GetSingleton().gameTask->IsVersusMode())
        {
            SetPlayerVersusPositions(players[j], j);
        }
        players[j].x += j;

        players[j].id = -1 * (j + 1);

        // Player colors are now handled dynamically via TankTypeManager based on type1/type2

        players[j].jid = j;

        players[j].SetInputMode(j == 0 ? InputMode::MODE_KEYBOARD_MOUSE : InputMode::MODE_JOYSTICK_GENERIC);
        players[j].SetType(players[j].type1, players[j].type2);

        ApplyDifficultySettings(players[j]);
        players[j].rotRate *= ROTATION_RATE_MULTIPLIER;
    }
}

void TankHandler::SetPlayerVersusPositions(Tank& player, int playerIndex)
{
    player.x = LevelHandler::GetSingleton().enemy[VERSUS_ENEMY_POSITION_1][0];
    player.z = LevelHandler::GetSingleton().enemy[VERSUS_ENEMY_POSITION_1][1];

    if (playerIndex == 0)
    {
        player.x = LevelHandler::GetSingleton().enemy[VERSUS_ENEMY_POSITION_2][0];
        player.z = LevelHandler::GetSingleton().enemy[VERSUS_ENEMY_POSITION_2][1];
    }
}

void TankHandler::ApplyDifficultySettings(Tank& player)
{
    if (difficultySetting == 0)
    {
        player.energyRegen *= EASY_REGEN_MULTIPLIER;
        player.maxEnergy *= EASY_ENERGY_MULTIPLIER;
        player.energy *= EASY_ENERGY_MULTIPLIER;
    }
    else if (difficultySetting == 1)
    {
        player.energyRegen *= NORMAL_REGEN_MULTIPLIER;
        player.maxEnergy *= NORMAL_ENERGY_MULTIPLIER;
        player.energy *= NORMAL_ENERGY_MULTIPLIER;
    }
}

void TankHandler::InitializePlayerControls()
{
    // Simplified controller initialization:
    // 1. Set default controls for all players
    // 2. Override with joystick controls if available and enabled
    SetupDefaultPlayerControls();
    
    // Override with joystick controls if available and enabled
    if (SDL_NumJoysticks() > 0 && isInputJoy)
    {
        if (numPlayers == 1)
        {
            InitializeSinglePlayerControls();
        }
        else
        {
            InitializeMultiPlayerControls();
        }
    }
}

void TankHandler::SetupDefaultPlayerControls()
{
    // Player 1 always defaults to keyboard/mouse
    players[PRIMARY_PLAYER_INDEX].jid = PRIMARY_PLAYER_INDEX;
    players[PRIMARY_PLAYER_INDEX].SetInputMode(InputMode::MODE_KEYBOARD_MOUSE);
    
    // Additional players default to generic joystick
    for (int i = 1; i < numPlayers; i++)
    {
        players[i].jid = i;
        players[i].SetInputMode(InputMode::MODE_JOYSTICK_GENERIC);
    }
}

void TankHandler::InitializeSinglePlayerControls()
{
    // For single player, if joystick is preferred and available, use it
    if (IsControllerConnected(PRIMARY_PLAYER_INDEX))
    {
        AssignControllerToPlayer(PRIMARY_PLAYER_INDEX, PRIMARY_PLAYER_INDEX);
    }
}

void TankHandler::InitializeMultiPlayerControls()
{
    int availableJoysticks = SDL_NumJoysticks();
    
    // Assign controllers based on availability
    if (availableJoysticks >= numPlayers)
    {
        // Enough controllers for all players
        for (int i = 0; i < numPlayers; i++)
        {
            AssignControllerToPlayer(i, i);
        }
    }
    else if (availableJoysticks == MIN_JOYSTICKS_FOR_MULTIPLAYER && numPlayers == 2)
    {
        // Special case: one controller for two players
        // Player 1 keeps keyboard/mouse, Player 2 gets the controller
        AssignControllerToPlayer(SECONDARY_PLAYER_INDEX, PRIMARY_PLAYER_INDEX);
    }
    else if (availableJoysticks > MIN_JOYSTICKS_FOR_MULTIPLAYER)
    {
        // Some controllers available, assign what we can
        for (int i = 0; i < std::min(numPlayers, availableJoysticks); i++)
        {
            AssignControllerToPlayer(i, i);
        }
    }
}

void TankHandler::AssignControllerToPlayer(int playerIndex, int joystickIndex)
{
    if (playerIndex >= 0 && playerIndex < MAX_PLAYERS && IsControllerConnected(joystickIndex))
    {
        players[playerIndex].jid = joystickIndex;
        players[playerIndex].SetInputMode(DetectControllerType(joystickIndex));
    }
}

bool TankHandler::IsControllerConnected(int joystickIndex)
{
    return joystickIndex >= 0 && joystickIndex < SDL_NumJoysticks();
}

void TankHandler::LogControllerSetup() const
{
    // Debug function to log current controller configuration
    // Can be called after initialization to verify setup
    for (int i = 0; i < numPlayers; i++)
    {
        const Tank& player = players[i];
        std::string inputModeStr;
        
        switch (player.inputMode)
        {
            case InputMode::MODE_KEYBOARD_MOUSE:
                inputModeStr = "Keyboard/Mouse";
                break;
            case InputMode::MODE_JOYSTICK_GENERIC:
                inputModeStr = "Generic Joystick";
                break;
            case InputMode::MODE_NINTENDO_GC:
                inputModeStr = "GameCube Controller";
                break;
            case InputMode::MODE_EXTREME_3D:
                inputModeStr = "Extreme 3D Pro";
                break;
            default:
                inputModeStr = "Unknown";
                break;
        }
           
        Logger::Get().Write("Player %d: %s (JoyID: %d)\n", i + 1, inputModeStr.c_str(), player.jid);
        Logger::Get().Write("Player %d: %s (JoyID: %d)\n", i + 1, inputModeStr.c_str(), player.jid);
    }
}

void TankHandler::AddControllerMapping(const std::string& pattern, InputMode mode)
{
    // Future extensibility: Allow runtime addition of controller mappings
    // This could be used to add support for new controllers without recompiling
    // Implementation would extend the static mapping in DetectControllerType
    // For now, this is a placeholder for future enhancement
}

InputMode TankHandler::DetectControllerType(int joyIndex)
{
    // Validate input
    if (!IsControllerConnected(joyIndex))
    {
        return InputMode::MODE_JOYSTICK_GENERIC;
    }
    
    // Get controller name safely
    const std::string& joyName = InputTask::joynames[joyIndex];
    if (joyName.empty() || joyName == "NULL")
    {
        return InputMode::MODE_JOYSTICK_GENERIC;
    }
    
    // Organized controller type mapping for easier maintenance and extension
    static const std::vector<std::pair<std::vector<std::string>, InputMode>> controllerMappings = {
        // PlayStation controllers (all variants use generic joystick mode)
        {{"PS2", "MP-8866", "PlayStation", "Playstation"}, InputMode::MODE_JOYSTICK_GENERIC},
        
        // GameCube controllers (have specific button layout)
        {{"NGC", "GameCube"}, InputMode::MODE_NINTENDO_GC},
        
        // Logitech Extreme 3D Pro (flight stick with unique axis mapping)
        {{"Extreme 3D"}, InputMode::MODE_EXTREME_3D}
    };
    
    // Search for matching controller patterns
    for (const auto& mapping : controllerMappings)
    {
        for (const auto& pattern : mapping.first)
        {
            if (joyName.find(pattern) != std::string::npos)
            {
                return mapping.second;
            }
        }
    }
    
    // Default fallback for unknown controllers
    return InputMode::MODE_JOYSTICK_GENERIC;
}

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
            newTank->id = i; // Set proper enemy tank ID
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
    tank.id = index;
}

void TankHandler::SetEnemyType(Tank& tank, int index)
{
    int levelOffset = LevelHandler::GetSingleton().levelNumber - 48;
    
    if (levelOffset == 0)
    {
        tank.SetType(static_cast<TankType>(1 + index % 3), TankType::TYPE_GREY);
        if (tank.type1 == TankType::TYPE_BLUE)
            tank.SetType(TankType::TYPE_YELLOW, TankType::TYPE_GREY);
        if (tank.id == 4)
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

void TankHandler::NextFrame()
{
    static int frameCounter = 0;
    if (frameCounter % 60 == 0) {
        Logger::Get().Write("TankHandler::NextFrame() - Processing players through legacy system\n");
    }
    
    UpdatePlayerCombos();
    UpdatePlayerStates();
    UpdatePlayerTargeting();
    UpdateEnemyTanks();
    UpdateVersusMode();
    
    frameCounter++;
    
    LevelHandler::GetSingleton().UpdateItems();  // Update item animations
    LevelHandler::GetSingleton().ItemCollision();
}

void TankHandler::UpdatePlayerCombos()
{
    // Skip if PlayerManager is handling players
    if (playerManagerActive) {
        return;
    }
    
    for (int i = 0; i < numPlayers; i++)
    {
        if (combo[i] > 0.0f)
        {
            combo[i] -= COMBO_DECAY_RATE * GlobalTimer::dT;
            if (combo[i] < 0.0f)
            {
                combo[i] = 0.0f;
            }
        }
        else
        {
            comboNum[i] = 0;
        }
    }
}

void TankHandler::UpdatePlayerStates()
{
    static int logCounter = 0;
    
    // Skip player processing if PlayerManager is handling them
    if (playerManagerActive) {
        if (logCounter % 120 == 0) {
            Logger::Get().Write("TankHandler: SKIPPING player processing - PlayerManager is active\n");
        }
        logCounter++;
        return;
    }
    
    for (int i = 0; i < numPlayers; i++)
    {
        if (players[i].alive)
        {
            if (logCounter % 120 == 0) {
                Logger::Get().Write("TankHandler: LEGACY processing player %d NextFrame + HandleInput\n", i);
            }
            players[i].NextFrame();
            players[i].HandleInput();
            players[i].deadtime = 0.0f;
        }
        else
        {
            players[i].Die();
            if (App::GetSingleton().gameTask->IsVersusMode())
            {
                if (players[i].deadtime > VERSUS_RESPAWN_DELAY)
                {
                    LevelHandler::GetSingleton().NextLevel(true);
                }
            }
            else
            {
                if (players[i].deadtime > PLAYER_RESPAWN_DELAY)
                    LevelHandler::GetSingleton().NextLevel(false);
            }
            players[i].deadtime += GlobalTimer::dT;
        }
    }
    logCounter++;
}

void TankHandler::UpdatePlayerTargeting()
{
    // Skip if PlayerManager is handling players
    if (playerManagerActive) {
        return;
    }
    
    players[0].dist = INITIAL_PLAYER_DISTANCE;
    players[1].dist = INITIAL_PLAYER_DISTANCE;

    // Get enemy tanks from GameWorld
    if (!gameWorld) {
        return;
    }
    
    const auto& enemyTanks = gameWorld->GetTanks();
    if (!enemyTanks.empty())
    {
        auto update_player_target = [](Tank &player, const Tank &tank)
        {
            float dist = sqrt((tank.x - player.x) * (tank.x - player.x) + (tank.z - player.z) * (tank.z - player.z));
            if (dist < player.dist)
            {
                player.dist = dist;
                float ratio = (double)(player.z - tank.z) / (double)(player.x - tank.x);
                float rtyp = toDegrees(atan(ratio));
                if (tank.x < player.x)
                {
                    rtyp += 180;
                }
                player.rr = rtyp;
                player.rrl = player.rr;
            }
        };

        for (const auto& tankPtr : enemyTanks)
        {
            if (tankPtr && tankPtr->IsAlive())
            {
                const Tank& tank = *tankPtr;
                for (int p = 0; p < 2; ++p)
                {
                    update_player_target(players[p], tank);
                }
            }
        }
    }
}

void TankHandler::UpdateEnemyTanks()
{
    // GameWorld handles enemy tank updates through EntityManager
    // The enemy tanks are managed by GameWorld's EntityManager<Tank>
    // This method is kept for API compatibility but is now a no-op
}

void TankHandler::UpdateVersusMode()
{
    // Skip if PlayerManager is handling players  
    if (playerManagerActive) {
        return;
    }
    
    if (App::GetSingleton().gameTask->IsVersusMode())
    {
        players[0].dist = sqrt((players[0].x - players[1].x) * (players[0].x - players[1].x) + (players[0].z - players[1].z) * (players[0].z - players[1].z));
        players[1].dist = players[0].dist;

        float ratio = (double)(players[0].z - players[1].z) / (double)(players[0].x - players[1].x);

        float rtyp = toDegrees(atan(ratio));

        if (players[1].x < players[0].x)
        {
            rtyp += 180;
        }

        players[0].rr = rtyp;
        players[0].rrl = players[0].rr;

        rtyp += 180;

        players[1].rr = rtyp;
        players[1].rrl = players[1].rr;
    }
}

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
    
    // Set GameWorld reference for all player tanks
    for (int i = 0; i < numPlayers; i++) {
        players[i].SetGameWorld(gameWorld);
    }
    
    RegisterPlayersWithCollisionSystem();
}

void TankHandler::RegisterPlayersWithCollisionSystem() {
    if (!gameWorld) return;
    
    // Register all active player tanks with the collision system
    for (int i = 0; i < numPlayers; i++) {
        Tank* player = &players[i];
        if (player->alive) {
            player->isPlayer = true;  // Ensure it's marked as a player
            gameWorld->GetCollisionSystem().RegisterEntity(
                player, 
                CollisionShape3D(CollisionShape3D::CUSTOM, 0.4f), 
                CollisionLayer::PLAYER_TANKS
            );
            Logger::Get().Write("Registered player tank %d with collision system\n", i);
        }
    }
}

Tank* TankHandler::GetPlayerTank(int playerIndex) {
    if (playerIndex >= 0 && playerIndex < MAX_PLAYERS) {
        return &players[playerIndex];
    }
    return nullptr;
}

const Tank* TankHandler::GetPlayerTank(int playerIndex) const {
    if (playerIndex >= 0 && playerIndex < MAX_PLAYERS) {
        return &players[playerIndex];
    }
    return nullptr;
}

void TankHandler::SetPlayerManager(PlayerManager* pm) {
    playerManager = pm;
    Logger::Get().Write("TankHandler: PlayerManager reference set\n");
}

void TankHandler::SetPlayerManagerActive(bool active) {
    playerManagerActive = active;
    Logger::Get().Write("TankHandler: PlayerManager %s - player processing %s\n", 
                       active ? "ACTIVATED" : "DEACTIVATED", 
                       active ? "DISABLED" : "ENABLED");
}

