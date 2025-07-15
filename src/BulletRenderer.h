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
#include "Bullet.h"

/**
 * @brief Handles rendering of all bullets in the game
 * 
 * This class is responsible for rende ring bullets from both player tanks
 * and enemy tanks. Separated from TankHandler to improve code organization
 * and provide a focused interface for bullet rendering operations.
 */
class BulletRenderer 
{
public:
    // Draw all bullets from a vector
    static void Draw(const std::vector<Bullet>& bullets);
    
private:
    static void SetupBulletRenderState();
    static void RestoreBulletRenderState();
};
