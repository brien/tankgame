#include "PlayerManager.h"
#include "Logger.h"
#include "App.h"
#include "TankTypeManager.h"
#include "GameWorld.h"
#include "LevelHandler.h"
#include "collision/CollisionSystem.h"
#include "events/CollisionEvents.h"
#include <SDL2/SDL.h>
#include <iostream>

PlayerManager::PlayerManager() = default;

void PlayerManager::Initialize() {
    Logger::Get().Write("PlayerManager: Initializing player management system\n");
    
    // Reset all player data
    Reset();
    
    // Initialize player tanks
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i] = Tank();
        players[i].id = -1 - i; // Player IDs are negative (-1, -2)
        players[i].isPlayer = true;
        
        // Initialize the tank properly first
        players[i].Init();
        
        // Set alive status AFTER Init() to prevent it being overwritten
        players[i].alive = (i < numPlayers); // Only mark active players as alive
        
        Logger::Get().Write("PlayerManager::Initialize - Player %d: numPlayers=%d, setting alive=%s\n", 
                          i, numPlayers, (i < numPlayers) ? "true" : "false");
        
        // Set default tank properties
        players[i].type1 = TankType::TYPE_GREY;
        players[i].type2 = TankType::TYPE_GREY;
        
        // Apply difficulty settings
        ApplyDifficultySettings(players[i]);
    }
    
    // Initialize controller system
    InitializePlayerControls();
    
    // Position players at spawn points
    // Check if this is versus mode and use appropriate positioning
    if (App::GetSingleton().gameTask && App::GetSingleton().gameTask->IsVersusMode()) {
        SetPlayerVersusPositions();
    } else {
        PositionPlayersAtSpawn();
    }
    
    Logger::Get().Write("PlayerManager: Initialized with %d players\n", numPlayers);
}

void PlayerManager::SetGameWorld(GameWorld* world) {
    // Set GameWorld reference for all player tanks so they can create bullets
    for (int i = 0; i < numPlayers; i++) {
        players[i].SetGameWorld(world);
    }
    
    Logger::Get().Write("PlayerManager: Set GameWorld reference for %d players\n", numPlayers);
}

void PlayerManager::RegisterPlayersWithCollisionSystem(GameWorld* world) {
    if (!world) return;
    
    // Register all active player tanks with the collision system
    for (int i = 0; i < numPlayers; i++) {
        if (players[i].alive) {
            world->GetCollisionSystem().RegisterEntity(
                &players[i], 
                CollisionShape3D(CollisionShape3D::CUSTOM, 0.4f), 
                CollisionLayer::PLAYER_TANKS
            );
            Logger::Get().Write("PlayerManager: Registered player tank %d with collision system\n", i);
        }
    }
}

void PlayerManager::Reset() {
    // Reset scoring data
    combo.fill(0.0f);
    special.fill(0.0f);
    comboNum.fill(0);
    hitCombo.fill(0);
    wins.fill(0);
    
    // Reset game state
    numPlayers = 1;
    difficultySetting = 0;
    isInputJoy = false;
    
    controllerMappings.clear();
}

void PlayerManager::NextFrame() {
    // Update all active player tanks (not just numPlayers)
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (players[i].alive) {
            float oldX = players[i].x, oldZ = players[i].z;
            
            // Handle input first
            players[i].HandleInput();
            
            // Then update physics and state
            players[i].NextFrame();
            
            float newX = players[i].x, newZ = players[i].z;
            if (oldX != newX || oldZ != newZ) {
                Logger::Get().Write("Player %d MOVED: (%.6f,%.6f) -> (%.6f,%.6f)\n", 
                                  i, oldX, oldZ, newX, newZ);
            } else if (i == 0) { // Only log player 0 position occasionally to reduce spam
                static int logCounter = 0;
                if (++logCounter % 300 == 0) { // Log every ~5 seconds at 60fps
                    Logger::Get().Write("Player %d stationary at (%.6f,%.6f)\n", i, newX, newZ);
                }
            }
        }
    }
    
    // Update combo decay
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (combo[i] > 0) {
            combo[i] -= COMBO_DECAY_RATE * 0.016f; // Assuming 60fps
            if (combo[i] < 0) combo[i] = 0;
        }
    }
}

bool PlayerManager::ValidatePlayerIndex(int playerIndex) const {
    return playerIndex >= 0 && playerIndex < MAX_PLAYERS;
}

// Scoring and combo system implementation
float PlayerManager::GetCombo(int playerIndex) const {
    return ValidatePlayerIndex(playerIndex) ? combo[playerIndex] : 0.0f;
}

void PlayerManager::SetCombo(int playerIndex, float value) {
    if (ValidatePlayerIndex(playerIndex)) {
        combo[playerIndex] = value;
    }
}

void PlayerManager::AddCombo(int playerIndex, float amount) {
    if (ValidatePlayerIndex(playerIndex)) {
        combo[playerIndex] += amount;
    }
}

float PlayerManager::GetSpecial(int playerIndex) const {
    return ValidatePlayerIndex(playerIndex) ? special[playerIndex] : 0.0f;
}

void PlayerManager::SetSpecial(int playerIndex, float value) {
    if (ValidatePlayerIndex(playerIndex)) {
        special[playerIndex] = value;
    }
}

int PlayerManager::GetComboNum(int playerIndex) const {
    return ValidatePlayerIndex(playerIndex) ? comboNum[playerIndex] : 0;
}

void PlayerManager::SetComboNum(int playerIndex, int value) {
    if (ValidatePlayerIndex(playerIndex)) {
        comboNum[playerIndex] = value;
    }
}

void PlayerManager::AddComboNum(int playerIndex, int amount) {
    if (ValidatePlayerIndex(playerIndex)) {
        comboNum[playerIndex] += amount;
    }
}

int PlayerManager::GetHitCombo(int playerIndex) const {
    return ValidatePlayerIndex(playerIndex) ? hitCombo[playerIndex] : 0;
}

void PlayerManager::SetHitCombo(int playerIndex, int value) {
    if (ValidatePlayerIndex(playerIndex)) {
        hitCombo[playerIndex] = value;
    }
}

void PlayerManager::AddHitCombo(int playerIndex, int amount) {
    if (ValidatePlayerIndex(playerIndex)) {
        hitCombo[playerIndex] += amount;
    }
}

void PlayerManager::ResetHitCombo(int playerIndex) {
    SetHitCombo(playerIndex, 0);
}

int PlayerManager::GetWins(int playerIndex) const {
    return ValidatePlayerIndex(playerIndex) ? wins[playerIndex] : 0;
}

void PlayerManager::SetWins(int playerIndex, int value) {
    if (ValidatePlayerIndex(playerIndex)) {
        wins[playerIndex] = value;
    }
}

void PlayerManager::AddWin(int playerIndex) {
    if (ValidatePlayerIndex(playerIndex)) {
        wins[playerIndex]++;
    }
}

// Player Tank access
Tank& PlayerManager::GetPlayer(int playerIndex) {
    if (!ValidatePlayerIndex(playerIndex)) {
        Logger::Get().Write("ERROR: Invalid player index %d\n", playerIndex);
        return players[0]; // Return first player as fallback
    }
    return players[playerIndex];
}

const Tank& PlayerManager::GetPlayer(int playerIndex) const {
    if (!ValidatePlayerIndex(playerIndex)) {
        Logger::Get().Write("ERROR: Invalid player index %d\n", playerIndex);
        return players[0]; // Return first player as fallback
    }
    return players[playerIndex];
}

void PlayerManager::ApplyDifficultySettings(Tank& player) {
    switch (difficultySetting) {
        case 0: // Easy
            player.maxEnergy *= EASY_ENERGY_MULTIPLIER;
            player.energy = player.maxEnergy;
            player.energyRegen *= EASY_REGEN_MULTIPLIER;
            break;
        case 1: // Normal
            player.maxEnergy *= NORMAL_ENERGY_MULTIPLIER;
            player.energy = player.maxEnergy;
            player.energyRegen *= NORMAL_REGEN_MULTIPLIER;
            break;
        case 2: // Hard
            // No changes for hard mode
            break;
        default:
            Logger::Get().Write("WARNING: Unknown difficulty setting %d\n", difficultySetting);
            break;
    }
}

// Controller/Input management implementation
void PlayerManager::InitializePlayerControls() {
    Logger::Get().Write("PlayerManager: Initializing player controls\n");
    
    // Set up default controls first
    SetupDefaultPlayerControls();
    
    // Override with joystick controls if available and enabled
    if (SDL_NumJoysticks() > 0 && isInputJoy) {
        if (numPlayers == 1) {
            InitializeSinglePlayerControls();
        } else {
            InitializeMultiPlayerControls();
        }
    }
    
    LogControllerSetup();
}

void PlayerManager::SetupDefaultPlayerControls() {
    // Player 1 defaults to keyboard/mouse
    players[PRIMARY_PLAYER_INDEX].SetInputMode(InputMode::MODE_KEYBOARD_MOUSE);
    
    // Additional players default to generic joystick
    for (int i = 1; i < numPlayers; i++) {
        players[i].SetInputMode(InputMode::MODE_JOYSTICK_GENERIC);
    }
}

void PlayerManager::InitializeSinglePlayerControls() {
    // For single player, if joystick is preferred and available, use it
    if (IsControllerConnected(PRIMARY_PLAYER_INDEX)) {
        AssignControllerToPlayer(PRIMARY_PLAYER_INDEX, PRIMARY_PLAYER_INDEX);
    }
}

void PlayerManager::InitializeMultiPlayerControls() {
    int availableJoysticks = SDL_NumJoysticks();
    
    if (availableJoysticks >= MIN_JOYSTICKS_FOR_MULTIPLAYER) {
        // Assign joysticks to players sequentially
        for (int i = 0; i < numPlayers && i < availableJoysticks; i++) {
            if (IsControllerConnected(i)) {
                AssignControllerToPlayer(i, i);
            }
        }
    } else {
        Logger::Get().Write("WARNING: Insufficient joysticks for multiplayer (%d available, %d needed)\n", 
                          availableJoysticks, MIN_JOYSTICKS_FOR_MULTIPLAYER);
    }
}

void PlayerManager::AssignControllerToPlayer(int playerIndex, int joystickIndex) {
    if (!ValidatePlayerIndex(playerIndex)) return;
    
    InputMode detectedMode = DetectControllerType(joystickIndex);
    players[playerIndex].SetInputMode(detectedMode);
    players[playerIndex].SetJoyIndex(joystickIndex);
    
    Logger::Get().Write("PlayerManager: Assigned joystick %d to player %d (mode: %d)\n", 
                       joystickIndex, playerIndex, static_cast<int>(detectedMode));
}

bool PlayerManager::IsControllerConnected(int joystickIndex) {
    return SDL_IsGameController(joystickIndex) == SDL_TRUE;
}

InputMode PlayerManager::DetectControllerType(int joyIndex) {
    if (!IsControllerConnected(joyIndex)) {
        return InputMode::MODE_JOYSTICK_GENERIC;
    }
    
    const char* controllerName = SDL_GameControllerNameForIndex(joyIndex);
    if (!controllerName) {
        return InputMode::MODE_JOYSTICK_GENERIC;
    }
    
    std::string name(controllerName);
    
    // Check controller mappings
    for (const auto& mapping : controllerMappings) {
        if (name.find(mapping.first) != std::string::npos) {
            return mapping.second;
        }
    }
    
    // Default controller type detection
    if (name.find("GameCube") != std::string::npos || 
        name.find("Nintendo") != std::string::npos) {
        return InputMode::MODE_NINTENDO_GC;
    } else if (name.find("Extreme 3D") != std::string::npos) {
        return InputMode::MODE_EXTREME_3D;
    }
    
    return InputMode::MODE_JOYSTICK_GENERIC;
}

void PlayerManager::AddControllerMapping(const std::string& pattern, InputMode mode) {
    controllerMappings.emplace_back(pattern, mode);
}

void PlayerManager::LogControllerSetup() const {
    Logger::Get().Write("=== PlayerManager Controller Setup ===\n");
    Logger::Get().Write("Number of players: %d\n", numPlayers);
    Logger::Get().Write("Joystick input enabled: %s\n", isInputJoy ? "Yes" : "No");
    Logger::Get().Write("Available joysticks: %d\n", SDL_NumJoysticks());
    
    for (int i = 0; i < numPlayers; i++) {
        Logger::Get().Write("Player %d: Mode=%d, JoyIndex=%d\n", 
                          i, static_cast<int>(players[i].GetInputMode()), players[i].GetJoyIndex());
    }
    Logger::Get().Write("=== End Controller Setup ===\n");
}

void PlayerManager::PositionPlayersAtSpawn() {
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (players[i].alive) {
            // Position at default spawn point with slight offset per player
            players[i].x = LevelHandler::GetSingleton().start[0] + i;
            players[i].z = LevelHandler::GetSingleton().start[1];
            players[i].y = 0; // Ground level
            players[i].rr = 0;
            Logger::Get().Write("Positioned player %d at spawn: (%f, %f)\n", 
                              i, players[i].x, players[i].z);
        }
    }
}

void PlayerManager::SetPlayerVersusPositions() {
    // Position players at versus spawn points
    if (players[PRIMARY_PLAYER_INDEX].alive) {
        players[PRIMARY_PLAYER_INDEX].x = LevelHandler::GetSingleton().enemy[VERSUS_ENEMY_POSITION_1][0];
        players[PRIMARY_PLAYER_INDEX].z = LevelHandler::GetSingleton().enemy[VERSUS_ENEMY_POSITION_1][1];
        players[PRIMARY_PLAYER_INDEX].y = 0;
        
        if (players[SECONDARY_PLAYER_INDEX].alive) {
            players[SECONDARY_PLAYER_INDEX].x = LevelHandler::GetSingleton().enemy[VERSUS_ENEMY_POSITION_2][0];
            players[SECONDARY_PLAYER_INDEX].z = LevelHandler::GetSingleton().enemy[VERSUS_ENEMY_POSITION_2][1];
            players[SECONDARY_PLAYER_INDEX].y = 0;
        }
        
        Logger::Get().Write("Set versus positions for active players\n");
    }
}