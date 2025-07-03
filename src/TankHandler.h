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
    static constexpr int MAX_PLAYERS = 4;
    static constexpr int MAX_ENEMY_SPAWN_POSITIONS = 16;
    static constexpr float COMBO_DECAY_RATE = 1.0f;
    static constexpr float PLAYER_RESPAWN_DELAY = 0.5f;
    static constexpr float VERSUS_RESPAWN_DELAY = 1.5f;
    
    // Controller detection constants
    static constexpr int PRIMARY_PLAYER_INDEX = 0;
    static constexpr int SECONDARY_PLAYER_INDEX = 1;
    static constexpr int MIN_JOYSTICKS_FOR_MULTIPLAYER = 1;
    static constexpr float INITIAL_PLAYER_DISTANCE = 2024.0f;
    static constexpr float SPECIAL_CHARGE_THRESHOLD_DIVISOR = 5.0f; // fireCost / 5
    static constexpr int ROTATION_STEP_DEGREES = 45;
    static constexpr float TANK_HEIGHT_OFFSET = 0.05f;
    static constexpr float TURRET_HEIGHT_OFFSET = 0.10f;
    static constexpr float TARGETING_HEIGHT_OFFSET = 0.25f;
    static constexpr float TARGETING_EFFECT_OFFSET = 0.018f;
    static constexpr float DISTANCE_COLOR_FACTOR = 50.0f;
    static constexpr float TEXTURE_DRIFT_SPEED = 100.0f;
    static constexpr float DRIFT_RESET_THRESHOLD = 1.0f;
    static constexpr float ROTATION_EFFECT_SPEED = 100.0f;
    static constexpr float EFFECT_SCALE_FACTOR = 0.2f;
    static constexpr float EFFECT_SCALE_RESTORE = 5.0f;
    
    // Player positioning constants
    static constexpr int VERSUS_ENEMY_POSITION_1 = 8;
    static constexpr int VERSUS_ENEMY_POSITION_2 = 9;
    
    // Difficulty multipliers
    static constexpr float EASY_ENERGY_MULTIPLIER = 3.0f;
    static constexpr float EASY_REGEN_MULTIPLIER = 2.0f;
    static constexpr float NORMAL_ENERGY_MULTIPLIER = 1.5f;
    static constexpr float NORMAL_REGEN_MULTIPLIER = 1.5f;
    static constexpr float ROTATION_RATE_MULTIPLIER = 2.0f;

    std::array<Tank, 4> players{};
    std::array<float, 2> combo{};                                                                                                                                                                                                                                                                                                                                                               

    std::array<float, 2> special{};
    std::array<int, 2> comboNum{};
    std::array<int, 2> hitCombo{};
    std::array<int, 2> wins{};

    std::vector<Tank> tanks;

    void CreateTank();
    void DestroyTank();

    void NextFrame();

    void DrawTanks() const;
    void DrawBullets() const;

    float closest = 0.0f;
    bool isInputJoy = false;
    int difficultySetting = 0;
    int numPlayers = 1;
    int numAttackingTanks = 0;

private:
    // Main initialization functions
    void InitializeEnemyTanks();
    void InitializePlayerTanks();
    void InitializePlayerControls();
    
    // Helper functions for enemy tank creation
    Tank CreateEnemyTank(int index);
    void SetEnemyPosition(Tank& tank, int index);
    void SetEnemyType(Tank& tank, int index);
    
    // Helper functions for player setup
    void SetPlayerVersusPositions(Tank& player, int playerIndex);
    void ApplyDifficultySettings(Tank& player);
    
    // Controller detection helpers
    void InitializeSinglePlayerControls();
    void InitializeMultiPlayerControls();
    InputMode DetectControllerType(int joyIndex);
    
    // Simplified controller setup helpers
    void AssignControllerToPlayer(int playerIndex, int joystickIndex);
    bool IsControllerConnected(int joystickIndex);
    void SetupDefaultPlayerControls();
    void LogControllerSetup() const;
    
    // Future extensibility helper
    void AddControllerMapping(const std::string& pattern, InputMode mode);
    
    // Game loop helpers
    void UpdatePlayerCombos();
    void UpdatePlayerStates();
    void UpdateEnemyTanks();
    void UpdateVersusMode();
    void UpdatePlayerTargeting();
    
    // Rendering helpers
    void DrawPlayerTank(int playerIndex, float drift) const;
    void DrawPlayerEffects(int playerIndex, float drift) const;
    void DrawTargetingUI(int playerIndex, float drift) const;
};
