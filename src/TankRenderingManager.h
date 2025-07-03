#pragma once

#include <array>
#include <vector>

// Forward declarations
class Tank;

/**
 * @brief Central manager for all tank-related rendering operations
 * 
 * This class serves as a facade that coordinates all tank rendering operations,
 * including player tanks, enemy tanks, bullets, and UI elements. It provides
 * a simple interface for the TankHandler to use while delegating the actual
 * rendering work to specialized renderer classes.
 */
class TankRenderingManager 
{
public:
    // Main rendering interface - replaces TankHandler::DrawTanks() and DrawBullets()
    static void RenderAllTanks(const std::array<Tank, 4>& players,
                              const std::vector<Tank>& enemyTanks,
                              const std::array<float, 2>& special,
                              int numPlayers);
    
    static void RenderAllBullets(const std::array<Tank, 4>& players,
                                const std::vector<Tank>& enemyTanks,
                                int numPlayers);
    
    // Individual rendering operations (for more fine-grained control if needed)
    static void RenderPlayerTanks(const std::array<Tank, 4>& players,
                                 const std::array<float, 2>& special,
                                 int numPlayers,
                                 bool hasEnemyTargets);
                                 
    static void RenderEnemyTanks(const std::vector<Tank>& enemyTanks);
    
private:
    // Helper method to determine if targeting UI should be shown
    static bool ShouldShowTargetingUI(const std::vector<Tank>& enemyTanks);
};
