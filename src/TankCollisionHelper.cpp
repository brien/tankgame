#include "TankCollisionHelper.h"
#include "Tank.h"

bool TankCollisionHelper::CheckFourPointCollision(const Tank& tank, float offsetY) {
    LevelHandler& level = LevelHandler::GetSingleton();
    float checkY = tank.y + offsetY;
    
    return level.PointCollision(tank.x, checkY, tank.z) ||
           level.PointCollision(tank.x + tank.collisionPoints[0], checkY, tank.z + tank.collisionPoints[2]) ||
           level.PointCollision(tank.x + tank.collisionPoints[3], checkY, tank.z + tank.collisionPoints[5]) ||
           level.PointCollision(tank.x + tank.collisionPoints[6], checkY, tank.z + tank.collisionPoints[8]) ||
           level.PointCollision(tank.x + tank.collisionPoints[9], checkY, tank.z + tank.collisionPoints[11]);
}

bool TankCollisionHelper::CheckFourPointFloatCollision(const Tank& tank, float offsetY) {
    LevelHandler& level = LevelHandler::GetSingleton();
    float checkY = tank.y + offsetY;
    
    return level.FloatCollision(tank.x, checkY, tank.z) ||
           level.FloatCollision(tank.x + tank.collisionPoints[0], checkY, tank.z + tank.collisionPoints[2]) ||
           level.FloatCollision(tank.x + tank.collisionPoints[3], checkY, tank.z + tank.collisionPoints[5]) ||
           level.FloatCollision(tank.x + tank.collisionPoints[6], checkY, tank.z + tank.collisionPoints[8]) ||
           level.FloatCollision(tank.x + tank.collisionPoints[9], checkY, tank.z + tank.collisionPoints[11]);
}

bool TankCollisionHelper::CheckFourPointFallCollision(const Tank& tank) {
    LevelHandler& level = LevelHandler::GetSingleton();
    
    return level.FallCollision(tank.x, tank.y, tank.z) ||
           level.FallCollision(tank.x + tank.collisionPoints[0], tank.y, tank.z + tank.collisionPoints[2]) ||
           level.FallCollision(tank.x + tank.collisionPoints[3], tank.y, tank.z + tank.collisionPoints[5]) ||
           level.FallCollision(tank.x + tank.collisionPoints[6], tank.y, tank.z + tank.collisionPoints[8]) ||
           level.FallCollision(tank.x + tank.collisionPoints[9], tank.y, tank.z + tank.collisionPoints[11]);
}

int TankCollisionHelper::FindHighestTerrainHeight(const Tank& tank) {
    LevelHandler& level = LevelHandler::GetSingleton();
    
    int highest = level.GetTerrainHeight(static_cast<int>(tank.x), static_cast<int>(tank.z));
    
    int height = level.GetTerrainHeight(static_cast<int>(tank.x + tank.collisionPoints[0]), static_cast<int>(tank.z + tank.collisionPoints[2]));
    if (height > highest) highest = height;
    
    height = level.GetTerrainHeight(static_cast<int>(tank.x + tank.collisionPoints[3]), static_cast<int>(tank.z + tank.collisionPoints[5]));
    if (height > highest) highest = height;
    
    height = level.GetTerrainHeight(static_cast<int>(tank.x + tank.collisionPoints[6]), static_cast<int>(tank.z + tank.collisionPoints[8]));
    if (height > highest) highest = height;
    
    height = level.GetTerrainHeight(static_cast<int>(tank.x + tank.collisionPoints[9]), static_cast<int>(tank.z + tank.collisionPoints[11]));
    if (height > highest) highest = height;
    
    return highest;
}

int TankCollisionHelper::FindHighestFloatHeight(const Tank& tank) {
    LevelHandler& level = LevelHandler::GetSingleton();
    
    int highest = level.GetFloatHeight(static_cast<int>(tank.x), static_cast<int>(tank.z));
    
    int height = level.GetFloatHeight(static_cast<int>(tank.x + tank.collisionPoints[0]), static_cast<int>(tank.z + tank.collisionPoints[2]));
    if (height > highest) highest = height;
    
    height = level.GetFloatHeight(static_cast<int>(tank.x + tank.collisionPoints[3]), static_cast<int>(tank.z + tank.collisionPoints[5]));
    if (height > highest) highest = height;
    
    height = level.GetFloatHeight(static_cast<int>(tank.x + tank.collisionPoints[6]), static_cast<int>(tank.z + tank.collisionPoints[8]));
    if (height > highest) highest = height;
    
    height = level.GetFloatHeight(static_cast<int>(tank.x + tank.collisionPoints[9]), static_cast<int>(tank.z + tank.collisionPoints[11]));
    if (height > highest) highest = height;
    
    return highest;
}

int TankCollisionHelper::FindCollidingPoint(const Tank& tank) {
    LevelHandler& level = LevelHandler::GetSingleton();
    
    if (level.PointCollision(tank.x + tank.collisionPoints[0], tank.y, tank.z + tank.collisionPoints[2])) {
        return 0;
    }
    if (level.PointCollision(tank.x + tank.collisionPoints[3], tank.y, tank.z + tank.collisionPoints[5])) {
        return 1;
    }
    if (level.PointCollision(tank.x + tank.collisionPoints[6], tank.y, tank.z + tank.collisionPoints[8])) {
        return 2;
    }
    if (level.PointCollision(tank.x + tank.collisionPoints[9], tank.y, tank.z + tank.collisionPoints[11])) {
        return 3;
    }
    
    return -1; // No collision found
}

bool TankCollisionHelper::CheckSpecificPointCollision(const Tank& tank, int pointIndex) {
    if (pointIndex < 0 || pointIndex > 3) return false;
    
    LevelHandler& level = LevelHandler::GetSingleton();
    int xIndex = pointIndex * 3;
    int zIndex = xIndex + 2;
    
    return level.PointCollision(tank.x + tank.collisionPoints[xIndex], tank.y, tank.z + tank.collisionPoints[zIndex]);
}

void TankCollisionHelper::GetCollisionPoint(const Tank& tank, int pointIndex, float& outX, float& outZ) {
    if (pointIndex < 0 || pointIndex > 3) {
        outX = tank.x;
        outZ = tank.z;
        return;
    }
    
    int xIndex = pointIndex * 3;
    int zIndex = xIndex + 2;
    
    outX = tank.x + tank.collisionPoints[xIndex];
    outZ = tank.z + tank.collisionPoints[zIndex];
}
