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
#include <array>

// Forward declarations
class Tank;
class Bullet;

/**
 * @brief Handles rendering of all bullets in the game
 * 
 * This class is responsible for rendering bullets from both player tanks
 * and enemy tanks. Separated from TankHandler to improve code organization
 * and provide a focused interface for bullet rendering operations.
 */
class BulletRenderer 
{
public:
    // Main rendering method
    static void DrawAllBullets(const std::vector<Tank>& enemyTanks,
                              const std::array<Tank, 4>& players,
                              int numPlayers);
    
    // Individual rendering methods for different bullet sources
    static void DrawEnemyBullets(const std::vector<Tank>& enemyTanks);
    static void DrawPlayerBullets(const std::array<Tank, 4>& players, int numPlayers);
    
private:
    // Helper methods
    static void SetupBulletRenderState();
    static void RestoreBulletRenderState();
    static void DrawBulletsFromTank(const Tank& tank);
};
