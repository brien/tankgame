#include "PlayerManager.h"
#include "GameWorld.h"
#include "InputTask.h"
#include "LevelHandler.h"
#include "InputHandlerFactory.h"
#include "TankHandler.h"
#include "Logger.h"
#include "App.h"
#include "GlobalTimer.h"
#include <algorithm>
#include <cmath>
#include <SDL2/SDL.h>

// Static constexpr member definitions (required for ODR-use in C++17)
constexpr int PlayerManager::MAX_PLAYERS;



void PlayerManager::Initialize(GameWorld* world) {
    Logger::Get().Write("PlayerManager: Initialize called with GameWorld\n");
    gameWorld = world;
    
    // Create initial player (but don't setup input yet)
    Logger::Get().Write("PlayerManager: Creating initial player...\n");
    CreatePlayer(0);
    
    // Claim any existing tanks created by TankHandler AND setup input
    Logger::Get().Write("PlayerManager: Claiming existing tanks from TankHandler...\n");
    ClaimExistingTanks();
    
    // numPlayers is now managed by PlayerManager only
    // (TankHandler no longer has numPlayers member)
    Logger::Get().Write("PlayerManager: Initialized with %d players\n", numPlayers);
    
    Logger::Get().Write("PlayerManager initialized with %d players\n", numPlayers);
}

void PlayerManager::ClaimExistingTanks() {
    // This method is deprecated - PlayerManager now creates its own tanks
    // Rather than claiming them from TankHandler
    Logger::Get().Write("PlayerManager::ClaimExistingTanks() - deprecated, player tanks created directly\n");
    
    // Get tanks from TankHandler and assign them to our Players
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            // Player tanks are now created by PlayerManager, not claimed from TankHandler
            Tank* existingTank = nullptr;
            if (existingTank && existingTank->alive) {
                Logger::Get().Write("PlayerManager: Player %d claiming tank from TankHandler\n", i);
                players[i]->TakeControlOf(existingTank);
            } else {
                Logger::Get().Write("PlayerManager: No valid tank found for Player %d in TankHandler\n", i);
            }
        }
    }
    
    // Set up input AFTER syncing preferences and claiming tanks
    Logger::Get().Write("PlayerManager: Setting up controllers and input with synced preferences...\n");
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
    
    // numPlayers now managed by PlayerManager only (TankHandler no longer has this member)
    
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

void PlayerManager::EnsurePlayerTankSetup() {
    // Re-apply player tank IDs and settings after level load
    // This is needed because TankHandler.players[] might be in an inconsistent state
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            Tank* tank = players[i]->GetControlledTank();
            if (tank) {
                // Ensure tank has correct identity
                tank->identity = TankIdentity::Player(i);
                tank->isPlayer = true;
                tank->jid = i;
                Logger::Get().Write("PlayerManager: Re-validated tank ID for Player %d (id=%d)\n", i, tank->identity.GetLegacyId());
            }
        }
    }
    
    // numPlayers managed by PlayerManager only (TankHandler no longer has this member)
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
    static constexpr float PLAYER_RESPAWN_DELAY = 0.5f;
    static constexpr float VERSUS_RESPAWN_DELAY = 1.5f;
    
    Logger::Get().Write("PlayerManager::NextFrame() - Starting\n");
    
    // Update player-specific game logic
    Logger::Get().Write("PlayerManager::NextFrame() - UpdatePlayerCombos\n");
    UpdatePlayerCombos();
    Logger::Get().Write("PlayerManager::NextFrame() - UpdatePlayerTargeting\n");
    UpdatePlayerTargeting();
    Logger::Get().Write("PlayerManager::NextFrame() - UpdateVersusMode\n");
    UpdateVersusMode();
    
    Logger::Get().Write("PlayerManager::NextFrame() - Updating %d players\n", numPlayers);
    for (int i = 0; i < numPlayers; i++) {
        if (players[i]) {
            Tank* tank = players[i]->GetControlledTank();
            if (tank) {
                // Check if tank is alive
                if (tank->alive) {
                    if (frameCounter % 60 == 0) { // Log every 60 frames (~1 second)
                        Logger::Get().Write("PlayerManager: Processing Player %d via NEW PlayerManager system\n", i);
                    }
                    players[i]->Update();
                    tank->deadtime = 0.0f;
                } else {
                    // Tank is dead - handle death and respawn
                    if (frameCounter % 60 == 0) {
                        Logger::Get().Write("PlayerManager: Player %d tank is DEAD (deadtime=%.2f)\n", i, tank->deadtime);
                    }
                    
                    // Increment deadtime FIRST, before checking respawn
                    tank->deadtime += GlobalTimer::dT;
                    tank->Die();
                    
                    if (versusMode) {
                        if (tank->deadtime > VERSUS_RESPAWN_DELAY) {
                            Logger::Get().Write("PlayerManager: Calling NextLevel for versus mode (deadtime=%.2f)\n", tank->deadtime);
                            LevelHandler::GetSingleton().NextLevel(true);
                            // Tank pointer is now invalid after NextLevel - don't use it again
                            break; // Exit the loop to avoid using stale pointers
                        }
                    } else {
                        if (tank->deadtime > PLAYER_RESPAWN_DELAY) {
                            Logger::Get().Write("PlayerManager: Calling NextLevel for player respawn (deadtime=%.2f)\n", tank->deadtime);
                            
                            // Release the old dead tank before NextLevel clears it
                            players[i]->ReleaseTank();
                            
                            LevelHandler::GetSingleton().NextLevel(false);
                            
                            // After NextLevel, create new player tank at spawn position
                            Logger::Get().Write("PlayerManager: Creating new player tank after respawn\n");
                            SetPlayerSpawnPosition(players[i].get(), i);
                            Logger::Get().Write("PlayerManager: Player %d respawned with new tank\n", i);
                            
                            // Exit the loop to avoid using stale pointers
                            break;
                        }
                    }
                }
            } else {
                if (frameCounter % 60 == 0) {
                    Logger::Get().Write("PlayerManager: Player %d has NO tank (nullptr)\n", i);
                }
            }
        }
    }
    frameCounter++;
    Logger::Get().Write("PlayerManager::NextFrame() - Complete\n");
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
    Logger::Get().Write("PlayerManager: Keeping TankHandler input preference isInputJoy=%d\n", isInputJoy);
    Logger::Get().Write("PlayerManager: Using %s input (from TankHandler)\n", isInputJoy ? "joystick" : "keyboard/mouse");
    
    // Don't change player count - keep what was set by TankHandler
    Logger::Get().Write("PlayerManager: Keeping numPlayers=%d from initialization\n", numPlayers);
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

const std::array<float, PlayerManager::MAX_PLAYERS> PlayerManager::GetCombos() const {
    std::array<float, MAX_PLAYERS> combos = {0.0f, 0.0f};
    
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            combos[i] = players[i]->GetCombo();
        }
    }
    
    return combos;
}

const std::array<int, PlayerManager::MAX_PLAYERS> PlayerManager::GetComboNumbers() const {
    std::array<int, MAX_PLAYERS> comboNums = {0, 0};
    
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            comboNums[i] = players[i]->GetComboNumber();
        }
    }
    
    return comboNums;
}

const std::array<int, PlayerManager::MAX_PLAYERS> PlayerManager::GetHitCombos() const {
    std::array<int, MAX_PLAYERS> hitCombos = {0, 0};
    
    for (int i = 0; i < numPlayers && i < MAX_PLAYERS; i++) {
        if (players[i]) {
            hitCombos[i] = players[i]->GetHitCombo();
        }
    }
    
    return hitCombos;
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
    
    Logger::Get().Write("PlayerManager: Setting up input for player %d isInputJoy=%d assignedJoystick=%d\n",
                       playerIndex, isInputJoy, assignedJoysticks[playerIndex]);
    
    if (isInputJoy && assignedJoysticks[playerIndex] >= 0) {
        // Use assigned joystick
        inputMode = DetectControllerType(assignedJoysticks[playerIndex]);
        Logger::Get().Write("PlayerManager: Player %d using joystick input mode\n", playerIndex);
    } else if (playerIndex == 0) {
        // Player 1 defaults to keyboard/mouse
        inputMode = InputMode::MODE_KEYBOARD_MOUSE;
        Logger::Get().Write("PlayerManager: Player %d using keyboard/mouse input mode\n", playerIndex);
    } else {
        // Player 2 needs a joystick if available
        inputMode = InputMode::MODE_JOYSTICK_GENERIC;
        Logger::Get().Write("PlayerManager: Player %d using generic joystick input mode\n", playerIndex);
    }
    
    // Create input handler
    auto inputHandler = InputHandlerFactory::CreateInputHandler(inputMode);
    if (inputHandler) {
        Logger::Get().Write("PlayerManager: Input handler created successfully for player %d\n", playerIndex);
        players[playerIndex]->SetInputHandler(std::move(inputHandler));
    } else {
        Logger::Get().Write("PlayerManager: ERROR - Failed to create input handler for player %d\n", playerIndex);
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
    if (!gameWorld) return;
    
    for (int i = 0; i < numPlayers; i++) {
        if (!players[i]) continue;
        
        Tank* playerTank = players[i]->GetControlledTank();
        if (!playerTank || !playerTank->alive) continue;
        
        // Find closest enemy tank for targeting
        float closestDistance = 99999.0f;
        const auto& allTanks = gameWorld->GetTanks();
        
        for (const auto& tankPtr : allTanks) {
            if (!tankPtr || !tankPtr->IsAlive()) continue;
            if (tankPtr.get() == playerTank) continue; // Skip self
            if (tankPtr->isPlayer) continue; // Skip other players in versus
            
            float dx = tankPtr->x - playerTank->x;
            float dz = tankPtr->z - playerTank->z;
            float distance = std::sqrt(dx * dx + dz * dz);
            
            if (distance < closestDistance) {
                closestDistance = distance;
            }
        }
        
        // Store closest distance in TankHandler for compatibility
        // (Some systems may still read this)
        // Targeting distance is managed by PlayerManager
    }
}

Player* PlayerManager::GetPlayerByTankId(int tankId) {
    if (tankId >= 0) return nullptr; // Not a player tank
    int playerIndex = (-1 * tankId) - 1;
    if (playerIndex < 0 || playerIndex >= numPlayers) return nullptr;
    return players[playerIndex].get();
}

void PlayerManager::ResetHitComboByTankId(int tankId) {
    Player* player = GetPlayerByTankId(tankId);
    if (player) {
        player->ResetHitCombo();
    }
}

void PlayerManager::UpdateVersusMode() {
    // Versus mode logic (migrated from TankHandler)  
    if (!versusMode) return;
    
    // Check for winner in versus mode
    int alivePlayers = 0;
    int lastAlivePlayer = -1;
    
    for (int i = 0; i < numPlayers; i++) {
        if (players[i]) {
            Tank* tank = players[i]->GetControlledTank();
            if (tank && tank->alive) {
                alivePlayers++;
                lastAlivePlayer = i;
            }
        }
    }
    
    // If only one player alive in versus, they win
    if (alivePlayers == 1 && lastAlivePlayer >= 0) {
        players[lastAlivePlayer]->AddWin();
        Logger::Get().Write("Player %d wins the round!\n", lastAlivePlayer);
        
        // Trigger level reset for next round
        // (death timer will handle actual respawn via NextLevel)
    }
}
