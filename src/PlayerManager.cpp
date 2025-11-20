#include "PlayerManager.h"
#include "GameWorld.h"
#include "InputTask.h"
#include "LevelHandler.h"
#include "InputHandlerFactory.h"
#include "TankHandler.h"
#include "Logger.h"
#include "App.h"
#include <algorithm>
#include <SDL2/SDL.h>

// Static constexpr member definitions (required for ODR-use in C++17)
constexpr int PlayerManager::MAX_PLAYERS;



void PlayerManager::Initialize(GameWorld* world) {
    std::cout << "PlayerManager: Initialize called with GameWorld" << std::endl;
    gameWorld = world;
    
    // Create initial player (but don't setup input yet)
    std::cout << "PlayerManager: Creating initial player..." << std::endl;
    CreatePlayer(0);
    
    // Claim any existing tanks created by TankHandler AND setup input
    std::cout << "PlayerManager: Claiming existing tanks from TankHandler..." << std::endl;
    ClaimExistingTanks();
    
    Logger::Get().Write("PlayerManager initialized with %d players\n", numPlayers);
}

void PlayerManager::ClaimExistingTanks() {
    // Sync input preferences from TankHandler
    isInputJoy = TankHandler::GetSingleton().isInputJoy;
    std::cout << "PlayerManager: Synced isInputJoy=" << isInputJoy << " from TankHandler" << std::endl;
    
    // Get tanks from TankHandler and assign them to our Players
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            Tank* existingTank = TankHandler::GetSingleton().GetPlayerTank(i);
            if (existingTank && existingTank->alive) {
                std::cout << "PlayerManager: Player " << i << " claiming tank from TankHandler" << std::endl;
                players[i]->TakeControlOf(existingTank);
            } else {
                std::cout << "PlayerManager: No valid tank found for Player " << i << " in TankHandler" << std::endl;
            }
        }
    }
    
    // Set up input AFTER syncing preferences and claiming tanks
    std::cout << "PlayerManager: Setting up controllers and input with synced preferences..." << std::endl;
    DetectControllers();
    SetupPlayerControls();
}

void PlayerManager::Shutdown() {
    for (auto& player : players) {
        player.reset();
    }
    gameWorld = nullptr;
}

Player* PlayerManager::GetPlayer(int index) {
    if (index >= 0 && index < MAX_PLAYERS && players[index]) {
        return players[index].get();
    }
    return nullptr;
}

const Player* PlayerManager::GetPlayer(int index) const {
    if (index >= 0 && index < MAX_PLAYERS && players[index]) {
        return players[index].get();
    }
    return nullptr;
}

void PlayerManager::SetNumPlayers(int count) {
    count = std::max(1, std::min(count, MAX_PLAYERS));
    
    if (count == numPlayers) return;
    
    int oldNumPlayers = numPlayers;
    numPlayers = count;
    
    // Create additional players if needed
    for (int i = oldNumPlayers; i < numPlayers; i++) {
        CreatePlayer(i);
    }
    
    // Remove excess players if reducing count
    for (int i = numPlayers; i < oldNumPlayers; i++) {
        if (players[i]) {
            players[i]->ReleaseTank();
            players[i].reset();
        }
    }
    
    Logger::Get().Write("PlayerManager now managing %d players\n", numPlayers);
}

void PlayerManager::SetDifficulty(int diff) {
    difficulty = std::max(0, std::min(diff, 2));
    
    // Apply difficulty settings to existing players
    for (int i = 0; i < numPlayers; i++) {
        if (players[i]) {
            players[i]->ApplyDifficultySettings(difficulty);
        }
    }
}

void PlayerManager::HandleInput() {
    for (int i = 0; i < numPlayers; i++) {
        if (players[i]) {
            players[i]->HandleInput();
        }
    }
}

void PlayerManager::Update() {
    UpdatePlayerStates();
    CheckForRespawns();
}

void PlayerManager::NextFrame() {
    // Update all players
    static int frameCounter = 0;
    
    // Update player-specific game logic
    UpdatePlayerCombos();
    UpdatePlayerTargeting();
    UpdateVersusMode();
    
    for (int i = 0; i < numPlayers; i++) {
        if (players[i]) {
            if (frameCounter % 60 == 0) { // Log every 60 frames (~1 second)
                std::cout << "PlayerManager: Processing Player " << i << " via NEW PlayerManager system" << std::endl;
            }
            players[i]->Update();
        }
    }
    frameCounter++;
}

void PlayerManager::SpawnPlayerTanks() {
    for (int i = 0; i < numPlayers; i++) {
        if (players[i] && !players[i]->HasTank()) {
            if (versusMode) {
                SetPlayerVersusPosition(players[i].get(), i);
            } else {
                SetPlayerSpawnPosition(players[i].get(), i);
            }
        }
    }
}

void PlayerManager::RespawnDeadPlayers() {
    for (int i = 0; i < numPlayers; i++) {
        if (players[i] && !players[i]->HasTank()) {
            players[i]->Respawn();
        }
    }
}

void PlayerManager::SetupVersusPositions() {
    versusMode = true;
    
    for (int i = 0; i < numPlayers; i++) {
        if (players[i]) {
            SetPlayerVersusPosition(players[i].get(), i);
        }
    }
}

void PlayerManager::ResetScores() {
    for (int i = 0; i < numPlayers; i++) {
        if (players[i]) {
            players[i]->ResetScore();
        }
    }
}

void PlayerManager::ResetWins() {
    for (int i = 0; i < numPlayers; i++) {
        if (players[i]) {
            players[i]->ResetWins();
        }
    }
}

bool PlayerManager::HasWinner() const {
    for (int i = 0; i < numPlayers; i++) {
        if (players[i] && players[i]->GetWins() >= 3) { // Win condition: 3 wins
            return true;
        }
    }
    return false;
}

int PlayerManager::GetWinningPlayer() const {
    for (int i = 0; i < numPlayers; i++) {
        if (players[i] && players[i]->GetWins() >= 3) {
            return i;
        }
    }
    return -1; // No winner
}

void PlayerManager::DetectControllers() {
    // Count available joysticks
    int numJoysticks = SDL_NumJoysticks();
    Logger::Get().Write("PlayerManager: Detected %d joysticks\n", numJoysticks);
    
    // Respect TankHandler's input preference instead of overriding
    std::cout << "PlayerManager: Keeping TankHandler input preference isInputJoy=" << isInputJoy << std::endl;
    Logger::Get().Write("PlayerManager: Using %s input (from TankHandler)\n", isInputJoy ? "joystick" : "keyboard/mouse");
    
    // Don't change player count - keep what was set by TankHandler
    std::cout << "PlayerManager: Keeping numPlayers=" << numPlayers << " from initialization" << std::endl;
}

void PlayerManager::SetupPlayerControls() {
    if (numPlayers == 1) {
        InitializeSinglePlayerControls();
    } else {
        InitializeMultiPlayerControls();
    }
}

// Compatibility methods for existing rendering system
const std::array<Tank*, PlayerManager::MAX_PLAYERS> PlayerManager::GetPlayerTanks() const {
    std::array<Tank*, MAX_PLAYERS> tanks = {nullptr, nullptr};
    
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            tanks[i] = players[i]->GetControlledTank();
        }
    }
    
    return tanks;
}

const std::array<float, PlayerManager::MAX_PLAYERS> PlayerManager::GetSpecialCharges() const {
    std::array<float, MAX_PLAYERS> charges = {0.0f, 0.0f};
    
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            charges[i] = players[i]->GetSpecialCharge();
        }
    }
    
    return charges;
}

const std::array<int, PlayerManager::MAX_PLAYERS> PlayerManager::GetScores() const {
    std::array<int, MAX_PLAYERS> scores = {0, 0};
    
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            scores[i] = players[i]->GetScore();
        }
    }
    
    return scores;
}

const std::array<int, PlayerManager::MAX_PLAYERS> PlayerManager::GetWins() const {
    std::array<int, MAX_PLAYERS> wins = {0, 0};
    
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            wins[i] = players[i]->GetWins();
        }
    }
    
    return wins;
}

// Private helper methods
void PlayerManager::CreatePlayer(int index) {
    if (index < 0 || index >= MAX_PLAYERS) return;
    
    players[index] = std::make_unique<Player>(index, gameWorld);
    
    // Apply current difficulty settings
    players[index]->ApplyDifficultySettings(difficulty);
    
    // Set up input handler
    SetupPlayerInputHandler(index);
}

void PlayerManager::SetupPlayerInputHandler(int playerIndex) {
    if (!players[playerIndex]) return;
    
    // Determine input mode and create appropriate handler
    InputMode inputMode;
    
    std::cout << "PlayerManager: Setting up input for player " << playerIndex 
              << " isInputJoy=" << isInputJoy 
              << " assignedJoystick=" << assignedJoysticks[playerIndex] << std::endl;
    
    if (isInputJoy && assignedJoysticks[playerIndex] >= 0) {
        // Use assigned joystick
        inputMode = DetectControllerType(assignedJoysticks[playerIndex]);
        std::cout << "PlayerManager: Player " << playerIndex << " using joystick input mode" << std::endl;
    } else if (playerIndex == 0) {
        // Player 1 defaults to keyboard/mouse
        inputMode = InputMode::MODE_KEYBOARD_MOUSE;
        std::cout << "PlayerManager: Player " << playerIndex << " using keyboard/mouse input mode" << std::endl;
    } else {
        // Player 2 needs a joystick if available
        inputMode = InputMode::MODE_JOYSTICK_GENERIC;
        std::cout << "PlayerManager: Player " << playerIndex << " using generic joystick input mode" << std::endl;
    }
    
    // Create input handler
    auto inputHandler = InputHandlerFactory::CreateInputHandler(inputMode);
    if (inputHandler) {
        std::cout << "PlayerManager: Input handler created successfully for player " << playerIndex << std::endl;
        players[playerIndex]->SetInputHandler(std::move(inputHandler));
    } else {
        std::cout << "PlayerManager: ERROR - Failed to create input handler for player " << playerIndex << std::endl;
    }
}

void PlayerManager::UpdatePlayerStates() {
    // Update player-specific states that don't belong on Tank entities
    // (Combo decay, special charge updates, etc. are handled in Player::Update())
}

void PlayerManager::CheckForRespawns() {
    // Check if any players need to respawn
    for (int i = 0; i < numPlayers; i++) {
        if (players[i] && !players[i]->HasTank()) {
            // TODO: Implement respawn timer logic
            // For now, respawn immediately
            players[i]->Respawn();
        }
    }
}

InputMode PlayerManager::DetectControllerType(int joyIndex) {
    // Simplified controller detection - can be expanded later
    if (joyIndex >= 0 && joyIndex < SDL_NumJoysticks()) {
        return InputMode::MODE_JOYSTICK_GENERIC;
    }
    return InputMode::MODE_KEYBOARD_MOUSE;
}

void PlayerManager::InitializeSinglePlayerControls() {
    if (players[0]) {
        assignedJoysticks[0] = isInputJoy ? 0 : -1;
        SetupPlayerInputHandler(0);
    }
}

void PlayerManager::InitializeMultiPlayerControls() {
    // Assign joysticks to players
    int numJoysticks = SDL_NumJoysticks();
    
    for (int i = 0; i < numPlayers; i++) {
        if (i < numJoysticks) {
            assignedJoysticks[i] = i;
        } else {
            assignedJoysticks[i] = -1; // No joystick available
        }
        
        if (players[i]) {
            SetupPlayerInputHandler(i);
        }
    }
}

void PlayerManager::SetPlayerSpawnPosition(Player* player, int playerIndex) {
    if (!player) return;
    
    // Use default spawn position from LevelHandler
    float spawnX = LevelHandler::GetSingleton().start[0];
    float spawnZ = LevelHandler::GetSingleton().start[1];
    float spawnY = LevelHandler::GetSingleton().GetTerrainHeight(static_cast<int>(spawnX), static_cast<int>(spawnZ));
    
    player->CreatePlayerTank(spawnX, spawnY, spawnZ);
}

void PlayerManager::SetPlayerVersusPosition(Player* player, int playerIndex) {
    if (!player) return;
    
    // Use versus mode spawn positions
    float spawnX, spawnZ;
    
    if (playerIndex == 0) {
        spawnX = LevelHandler::GetSingleton().enemy[8][0]; // VERSUS_ENEMY_POSITION_2
        spawnZ = LevelHandler::GetSingleton().enemy[8][1];
    } else {
        spawnX = LevelHandler::GetSingleton().enemy[9][0]; // VERSUS_ENEMY_POSITION_1  
        spawnZ = LevelHandler::GetSingleton().enemy[9][1];
    }
    
    float spawnY = LevelHandler::GetSingleton().GetTerrainHeight(static_cast<int>(spawnX), static_cast<int>(spawnZ));
    
    player->CreatePlayerTank(spawnX, spawnY, spawnZ);
}
void PlayerManager::UpdatePlayerCombos() {
    // Player combo decay logic (migrated from TankHandler)
    for (int i = 0; i < numPlayers; i++) {
        if (players[i]) {
            players[i]->UpdateComboDecay();
        }
    }
}

void PlayerManager::UpdatePlayerTargeting() {
    // Player targeting logic (migrated from TankHandler)
    // TODO: Implement player targeting logic specific to Player class
    // This method handled distance calculations and targeting in TankHandler
}

void PlayerManager::UpdateVersusMode() {
    // Versus mode logic (migrated from TankHandler)  
    if (App::GetSingleton().gameTask->IsVersusMode()) {
        // TODO: Implement versus mode player logic
        // This method handled player vs player interactions in TankHandler
    }
}
