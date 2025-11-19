#pragma once

#include <array>
#include <string>
#include <vector>
#include "Tank.h"

/**
 * Manages player-specific game state data.
 * Handles scoring, combos, difficulty settings, and controller configuration.
 * Replaces the player-related functionality previously in TankHandler.
 */
class PlayerManager {
public:
    // Constants from TankHandler
    static constexpr int MAX_PLAYERS = 2;
    static constexpr float COMBO_DECAY_RATE = 1.0f;
    static constexpr float PLAYER_RESPAWN_DELAY = 0.5f;
    static constexpr float VERSUS_RESPAWN_DELAY = 1.5f;
    static constexpr int PRIMARY_PLAYER_INDEX = 0;
    static constexpr int SECONDARY_PLAYER_INDEX = 1;
    static constexpr int MIN_JOYSTICKS_FOR_MULTIPLAYER = 1;
    static constexpr float SPECIAL_CHARGE_THRESHOLD_DIVISOR = 5.0f;

    // Difficulty multipliers
    static constexpr float EASY_ENERGY_MULTIPLIER = 3.0f;
    static constexpr float EASY_REGEN_MULTIPLIER = 2.0f;
    static constexpr float NORMAL_ENERGY_MULTIPLIER = 1.5f;
    static constexpr float NORMAL_REGEN_MULTIPLIER = 1.5f;
    
    // Versus mode spawn positions
    static constexpr int VERSUS_ENEMY_POSITION_1 = 14;
    static constexpr int VERSUS_ENEMY_POSITION_2 = 15;

    PlayerManager();
    ~PlayerManager() = default;

    void Initialize();
    void Reset();
    void NextFrame();

    // Player management
    void SetPlayerCount(int count) { numPlayers = count; }
    int GetPlayerCount() const { return numPlayers; }

    // Scoring and combo system
    float GetCombo(int playerIndex) const;
    void SetCombo(int playerIndex, float value);
    void AddCombo(int playerIndex, float amount);

    float GetSpecial(int playerIndex) const;
    void SetSpecial(int playerIndex, float value);

    int GetComboNum(int playerIndex) const;
    void SetComboNum(int playerIndex, int value);
    void AddComboNum(int playerIndex, int amount);

    int GetHitCombo(int playerIndex) const;
    void SetHitCombo(int playerIndex, int value);
    void AddHitCombo(int playerIndex, int amount);
    void ResetHitCombo(int playerIndex);

    int GetWins(int playerIndex) const;
    void SetWins(int playerIndex, int value);
    void AddWin(int playerIndex);

    // Difficulty system
    int GetDifficultySetting() const { return difficultySetting; }
    void SetDifficultySetting(int setting) { difficultySetting = setting; }

    // Controller/Input management
    bool IsInputJoy() const { return isInputJoy; }
    void SetInputJoy(bool enabled) { isInputJoy = enabled; }

    void InitializePlayerControls();
    void AssignControllerToPlayer(int playerIndex, int joystickIndex);
    bool IsControllerConnected(int joystickIndex);
    InputMode DetectControllerType(int joyIndex);
    void AddControllerMapping(const std::string& pattern, InputMode mode);
    void LogControllerSetup() const;

    // GameWorld integration
    void SetGameWorld(class GameWorld* world);
    void RegisterPlayersWithCollisionSystem(class GameWorld* world);
    
    // Player positioning
    void PositionPlayersAtSpawn();
    void SetPlayerVersusPositions();

    // Player Tank access (manages the player Tank instances)
    Tank& GetPlayer(int playerIndex);
    const Tank& GetPlayer(int playerIndex) const;
    std::array<Tank, MAX_PLAYERS>& GetPlayers() { return players; }
    const std::array<Tank, MAX_PLAYERS>& GetPlayers() const { return players; }
    const std::array<float, MAX_PLAYERS>& GetSpecial() const { return special; }
    int GetNumPlayers() const { return numPlayers; }

    // Difficulty application
    void ApplyDifficultySettings(Tank& player);

private:
    // Player instances
    std::array<Tank, MAX_PLAYERS> players{};

    // Scoring data
    std::array<float, MAX_PLAYERS> combo{};
    std::array<float, MAX_PLAYERS> special{};
    std::array<int, MAX_PLAYERS> comboNum{};
    std::array<int, MAX_PLAYERS> hitCombo{};
    std::array<int, MAX_PLAYERS> wins{};

    // Game state
    int numPlayers = 1;
    int difficultySetting = 0;
    bool isInputJoy = false;

    // Controller mappings (moved from TankHandler)
    std::vector<std::pair<std::string, InputMode>> controllerMappings;

    // Helper methods
    void SetupDefaultPlayerControls();
    void InitializeSinglePlayerControls();
    void InitializeMultiPlayerControls();
    bool ValidatePlayerIndex(int playerIndex) const;
};