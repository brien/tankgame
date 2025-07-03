#pragma once

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vector>

// Forward declarations
class Tank;

/**
 * @brief Handles rendering of enemy/AI tanks
 * 
 * This class is responsible for rendering all enemy tanks in the game.
 * Uses the existing TankRenderer for individual tank rendering but provides
 * a clean interface for rendering all enemy tanks at once.
 * Separated from TankHandler to improve code organization.
 */
class EnemyTankRenderer 
{
public:
    // Main rendering method
    static void DrawEnemyTanks(const std::vector<Tank>& enemyTanks);
    
private:
    // Helper methods
    static void SetupEnemyTankRenderState();
    static void RestoreEnemyTankRenderState();
};
