#include "BulletDataExtractor.h"
#include "../Bullet.h"
#include <cmath>

std::vector<BulletRenderData> BulletDataExtractor::ExtractBulletRenderData(const std::vector<Bullet>& bullets) {
    std::vector<BulletRenderData> renderData;
    renderData.reserve(bullets.size());
    
    for (const Bullet& bullet : bullets) {
        if (bullet.IsAlive()) {
            renderData.push_back(ExtractSingleBulletData(bullet));
        }
    }
    
    return renderData;
}

BulletRenderData BulletDataExtractor::ExtractSingleBulletData(const Bullet& bullet) {
    BulletRenderData data;
    
    // Position
    data.position.x = bullet.GetX();
    data.position.y = bullet.GetY();
    data.position.z = bullet.GetZ();
    
    // Rotation
    data.rotation.x = bullet.GetRX();
    data.rotation.y = bullet.GetRY();
    data.rotation.z = bullet.GetRZ();
    
    // Velocity (computed from rotation and move rate)
    float moveRate = bullet.GetMoveRate();
    float ry = bullet.GetRY();
    const float DTR = 3.14159265f / 180.0f; // Degrees to radians
    
    data.velocity.x = moveRate * std::cos(ry * DTR);
    data.velocity.y = bullet.GetVY();
    data.velocity.z = moveRate * std::sin(ry * DTR);
    
    // Colors
    data.primaryColor.r = bullet.GetR();
    data.primaryColor.g = bullet.GetG();
    data.primaryColor.b = bullet.GetB();
    data.primaryColor.a = 1.0f;
    data.secondaryColor.r = bullet.GetR2();
    data.secondaryColor.g = bullet.GetG2();
    data.secondaryColor.b = bullet.GetB2();
    data.secondaryColor.a = 1.0f;
    
    // Type information
    data.type1 = bullet.GetType1();
    data.type2 = bullet.GetType2();
    data.ownerId = bullet.GetTankId();
    
    // Speed and power
    data.speed = moveRate;
    data.power = bullet.GetPower();
    
    return data;
}
