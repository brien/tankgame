#pragma once

#include "Player.h"
#include <array>
#include <memory>

// Forward declarations
class GameWorld;
class InputTask;

/**
 * Manager for player instances and player-related game logic.
 * Replaces the player-specific functionality from TankHandler.
 * Manages 1-2 Player instances (not an EntityManager since Players aren't entities).
 * Owned by GameTask, not a singleton.
 */
class PlayerManager {
public:
    PlayerManager() = default;
    ~PlayerManager() = default;

    // Constants
    static constexpr int MAX_PLAYERS = 2;
    
    // Initialization and lifecycle
    void Initialize(GameWorld* world);
    void Shutdown();
    
    // Player management
    Player* GetPlayer(int index);
    const Player* GetPlayer(int index) const;
    int GetNumPlayers() const { return numPlayers; }
    
    // Game setup
    void SetNumPlayers(int count);
    void SetDifficulty(int difficulty);
    int GetDifficulty() const { return difficulty; }
    
    // Input handling
    void HandleInput();
    void SetupPlayerControls();
    
    // Game state updates
    void Update();
    void NextFrame(); // Called by GameTask
    
    // Player-specific game logic (migrated from TankHandler)
    void UpdatePlayerCombos();
    void UpdatePlayerTargeting(); 
    void UpdateVersusMode();
    
    // Player tank management
    void SpawnPlayerTanks();
    void RespawnDeadPlayers();
    void ClaimExistingTanks(); // Take control of tanks created by TankHandler
    
    // Versus mode support
    void SetVersusMode(bool enabled) { versusMode = enabled; }
    bool IsVersusMode() const { return versusMode; }
    void SetupVersusPositions();
    
    // Match management
    void ResetScores();
    void ResetWins();
    bool HasWinner() const;
    int GetWinningPlayer() const;
    
    // Controller management
    void DetectControllers();
    void AssignControllerToPlayer(int playerIndex, int joystickIndex);
    bool IsControllerConnected(int joystickIndex) const;
    
    // Compatibility with existing rendering system
    // These methods provide access to player data in the format expected by current renderers
    const std::array<Tank*, MAX_PLAYERS> GetPlayerTanks() const;
    const std::array<float, MAX_PLAYERS> GetSpecialCharges() const;
    const std::array<int, MAX_PLAYERS> GetScores() const;
    const std::array<int, MAX_PLAYERS> GetWins() const;

private:
    // Player instances
    std::array<std::unique_ptr<Player>, MAX_PLAYERS> players;
    int numPlayers = 1;
    
    // Game settings
    int difficulty = 1; // 0=easy, 1=normal, 2=hard
    bool versusMode = false;
    
    // GameWorld reference
    GameWorld* gameWorld = nullptr;
    
    // Controller management
    bool isInputJoy = false;
    std::array<int, MAX_PLAYERS> assignedJoysticks = {-1, -1};
    
    // Helper methods
    void CreatePlayer(int index);
    void SetupPlayerInputHandler(int playerIndex);
    void UpdatePlayerStates();
    void CheckForRespawns();
    
    // Input detection helpers
    InputMode DetectControllerType(int joyIndex);
    void SetupDefaultPlayerControls();
    void InitializeSinglePlayerControls();
    void InitializeMultiPlayerControls();
    
    // Positioning helpers
    void SetPlayerSpawnPosition(Player* player, int playerIndex);
    void SetPlayerVersusPosition(Player* player, int playerIndex);
};