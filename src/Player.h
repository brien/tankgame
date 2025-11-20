#pragma once

#include "Tank.h"
#include "InputHandler.h"
#include <memory>

// Forward declarations
class GameWorld;
class InputHandler;

/**
 * Player controller class that manages player-specific state and controls a Tank entity.
 * Separates player concerns (input, scoring, UI) from tank physics/rendering.
 * Players are NOT entities - they are controllers that exist throughout the game session.
 */
class Player {
public:
    Player(int playerIndex, GameWorld* world);
    ~Player() = default;

    // Delete copy operations for clear ownership semantics
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    // Move operations allowed
    Player(Player&&) = default;
    Player& operator=(Player&&) = default;

    // === CORE PLAYER PROPERTIES ===
    int GetPlayerIndex() const { return playerIndex; }
    Tank* GetControlledTank() const { return controlledTank; }
    bool HasTank() const { return controlledTank != nullptr; }

    // === TANK CONTROL MANAGEMENT ===
    /**
     * Take control of a tank entity. Updates collision layers and tank properties.
     * @param tank Pointer to tank in GameWorld's EntityManager, or nullptr to release control
     */
    void TakeControlOf(Tank* tank);

    /**
     * Release control of current tank (sets it to AI/enemy control)
     */
    void ReleaseTank();

    /**
     * Create a new tank for this player at specified position
     */
    Tank* CreatePlayerTank(float x, float y, float z);

    // === PLAYER STATE MANAGEMENT ===
    // Game statistics
    int GetScore() const { return score; }
    void AddScore(int points) { score += points; }
    void ResetScore() { score = 0; }

    // Combo system
    float GetCombo() const { return combo; }
    int GetComboNumber() const { return comboNumber; }
    void AddCombo(float comboPoints, int comboNum);
    void ResetCombo();

    // Special weapon charge
    float GetSpecialCharge() const { return specialCharge; }
    void AddSpecialCharge(float charge);
    void UseSpecialCharge(float cost);
    bool CanUseSpecial(float cost) const { return specialCharge >= cost; }

    // Match statistics
    int GetWins() const { return wins; }
    void AddWin() { wins++; }
    void ResetWins() { wins = 0; }

    // === INPUT HANDLING ===
    /**
     * Process input and control the tank accordingly.
     * Called by PlayerManager during input processing.
     */
    void HandleInput();

    /**
     * Set the input handler for this player (keyboard, controller, etc.)
     */
    void SetInputHandler(std::unique_ptr<InputHandler> handler);

    // === GAME WORLD INTERACTIONS ===
    /**
     * Check for item collisions and apply effects.
     * Only player-controlled tanks can pickup items.
     */
    void CheckItemCollisions();

    /**
     * Update player state each frame (combo decay, special charge, etc.)
     */
    void Update();

    // === RESPAWN AND LIFECYCLE ===
    /**
     * Handle player tank death - manages respawn logic
     */
    void OnTankDestroyed();

    /**
     * Respawn player tank at appropriate location
     */
    void Respawn();

    // === DIFFICULTY AND SETTINGS ===
    void ApplyDifficultySettings(int difficulty);
    
    // === COMBO AND GAME STATE MANAGEMENT ===
    /**
     * Update combo decay for this player (called by PlayerManager)
     */
    void UpdateComboDecay();

private:
    // Player identification
    int playerIndex;           // 0 or 1 for player 1/2
    GameWorld* gameWorld;      // Reference to game world for tank creation/management

    // Tank control
    Tank* controlledTank = nullptr;  // Pointer to controlled tank in GameWorld EntityManager

    // Player statistics
    int score = 0;
    int wins = 0;

    // Combo system
    float combo = 0.0f;
    int comboNumber = 0;
    
    // Special weapon system
    float specialCharge = 0.0f;

    // Input handling
    std::unique_ptr<InputHandler> inputHandler;

    // Constants for gameplay balance
    static constexpr float COMBO_DECAY_RATE = 1.0f;
    static constexpr float SPECIAL_CHARGE_MAX = 100.0f;
    static constexpr float RESPAWN_DELAY = 2.0f;

    // Helper methods
    void ApplyItemEffect(const class Item* item);
    void SetupTankForPlayer(Tank* tank);
};