#pragma once

#include "LevelHandler.h"

class Tank;

/**
 * Helper class for Tank collision detection operations.
 * Encapsulates common collision patterns to reduce code duplication.
 */
class TankCollisionHelper {
public:
    /**
     * Check if any of the tank's four collision points collide with terrain.
     * @param tank The tank to check
     * @param offsetY Y-axis offset for the collision check
     * @return true if any collision point collides
     */
    static bool CheckFourPointCollision(const Tank& tank, float offsetY = 0.0f);
    
    /**
     * Check if any of the tank's four collision points have float collision.
     * @param tank The tank to check
     * @param offsetY Y-axis offset for the collision check
     * @return true if any collision point has float collision
     */
    static bool CheckFourPointFloatCollision(const Tank& tank, float offsetY = 0.0f);
    
    /**
     * Check if any of the tank's four collision points have fall collision.
     * @param tank The tank to check
     * @return true if any collision point has fall collision
     */
    static bool CheckFourPointFallCollision(const Tank& tank);
    
    /**
     * Find the highest terrain height among the tank's four collision points.
     * @param tank The tank to check
     * @return The highest terrain height found
     */
    static int FindHighestTerrainHeight(const Tank& tank);
    
    /**
     * Find the highest float height among the tank's four collision points.
     * @param tank The tank to check
     * @return The highest float height found
     */
    static int FindHighestFloatHeight(const Tank& tank);
    
    /**
     * Determine which specific collision point is colliding and return its index.
     * @param tank The tank to check
     * @return Index of the colliding point (0-3), or -1 if none
     */
    static int FindCollidingPoint(const Tank& tank);
    
    /**
     * Check collision for a specific point by index (0-3).
     * @param tank The tank to check
     * @param pointIndex Index of the collision point (0-3)
     * @return true if the specified point collides
     */
    static bool CheckSpecificPointCollision(const Tank& tank, int pointIndex);

private:
    // Helper to get collision point coordinates
    static void GetCollisionPoint(const Tank& tank, int pointIndex, float& outX, float& outZ);
};
