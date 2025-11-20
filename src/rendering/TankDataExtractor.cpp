#include "TankDataExtractor.h"

TankRenderData TankDataExtractor::ExtractRenderData(const Tank& tank) {
    TankRenderData data;
    
    // Extract position and rotation
    data.position = Vector3(tank.x, tank.y, tank.z);
    data.bodyRotation = Vector3(tank.rx, tank.ry, tank.rz);
    data.turretRotation = Vector3(tank.rtx, tank.rty, tank.rtz);
    data.targetRotation = tank.rrl;
    
    // Extract health and energy (was: health and charge)
    data.health = tank.health;      // Now correctly maps to health
    data.maxHealth = tank.maxHealth;
    data.charge = tank.energy;      // Charge field now represents energy for actions
    data.maxCharge = tank.maxEnergy;
    data.fireCost = tank.fireCost;
    
    // Extract colors
    data.r = tank.r;
    data.g = tank.g;
    data.b = tank.b;
    data.r2 = tank.r2;
    data.g2 = tank.g2;
    data.b2 = tank.b2;
    
    // Extract status
    data.alive = tank.alive;
    data.playerId = tank.id;
    data.isPlayer = (tank.id < 0); // Players have negative IDs
    
    // Extract tank types
    data.type1 = tank.type1;
    data.type2 = tank.type2;
    
    // Extract movement state
    data.grounded = tank.isGrounded;
    data.isJumping = tank.isJumping;
    data.velocity = Vector3(tank.vx, tank.vy, tank.vz);
    
    // Extract special effects data
    data.bonus = tank.bonus;
    data.bonusTime = tank.bonusTime;
    data.hitAlpha = tank.hitAlpha;
    data.hitNum = tank.hitNum;
    
    return data;
}

std::vector<TankRenderData> TankDataExtractor::ExtractPlayerData(
    const std::array<Tank, TankHandler::MAX_PLAYERS>& players,
    const std::array<float, TankHandler::MAX_PLAYERS>& special,
    int numPlayers) {
    
    std::vector<TankRenderData> renderData;
    renderData.reserve(numPlayers);
    
    for (int i = 0; i < numPlayers; ++i) {
        if (players[i].alive) {
            TankRenderData data = ExtractRenderData(players[i]);
            // Add special charge information for players
            // Note: special array might be used for additional effects
            renderData.push_back(data);
        }
    }
    
    return renderData;
}

std::vector<TankRenderData> TankDataExtractor::ExtractEnemyData(const std::vector<Tank>& enemyTanks) {
    std::vector<TankRenderData> renderData;
    renderData.reserve(enemyTanks.size());
    
    for (const auto& tank : enemyTanks) {
        if (tank.alive) {
            renderData.push_back(ExtractRenderData(tank));
        }
    }
    
    return renderData;
}

std::vector<TankRenderData> TankDataExtractor::ExtractEnemyData(const std::vector<const Tank*>& enemyTanks) {
    std::vector<TankRenderData> renderData;
    renderData.reserve(enemyTanks.size());
    
    for (const auto* tank : enemyTanks) {
        if (tank && tank->alive) {
            renderData.push_back(ExtractRenderData(*tank));
        }
    }
    
    return renderData;
}

std::vector<TankRenderData> TankDataExtractor::ExtractAllTankData(
    const std::array<Tank, TankHandler::MAX_PLAYERS>& players,
    const std::vector<Tank>& enemyTanks,
    const std::array<float, TankHandler::MAX_PLAYERS>& special,
    int numPlayers) {
    
    std::vector<TankRenderData> allRenderData;
    
    // Extract player data
    auto playerData = ExtractPlayerData(players, special, numPlayers);
    allRenderData.insert(allRenderData.end(), playerData.begin(), playerData.end());
    
    // Extract enemy data
    auto enemyData = ExtractEnemyData(enemyTanks);
    allRenderData.insert(allRenderData.end(), enemyData.begin(), enemyData.end());
    
    return allRenderData;
}
