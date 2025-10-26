#pragma once

#include "RenderData.h"
#include "../Tank.h"
#include "../TankHandler.h"
#include <vector>
#include <array>

/**
 * Extracts rendering data from Tank objects for the TankRenderer.
 * This class serves as a bridge between game logic (Tank) and rendering system.
 */
class TankDataExtractor {
public:
    /**
     * Extracts rendering data from a single Tank object.
     * @param tank The Tank object to extract data from
     * @return TankRenderData structure containing all necessary rendering information
     */
    static TankRenderData ExtractRenderData(const Tank& tank);
    
    /**
     * Extracts rendering data from player tanks array.
     * @param players Array of player Tank objects
     * @param special Array of special charge values for players
     * @param numPlayers Number of active players
     * @return Vector of TankRenderData structures for rendering
     */
    static std::vector<TankRenderData> ExtractPlayerData(
        const std::array<Tank, TankHandler::MAX_PLAYERS>& players,
        const std::array<float, TankHandler::MAX_PLAYERS>& special,
        int numPlayers);
    
    /**
     * Extracts rendering data from enemy tanks vector.
     * @param enemyTanks Vector of enemy Tank objects
     * @return Vector of TankRenderData structures for rendering
     */
    static std::vector<TankRenderData> ExtractEnemyData(const std::vector<Tank>& enemyTanks);
    
    /**
     * Extracts rendering data from enemy tanks vector (pointer version).
     * @param enemyTanks Vector of pointers to enemy Tank objects
     * @return Vector of TankRenderData structures for rendering
     */
    static std::vector<TankRenderData> ExtractEnemyData(const std::vector<const Tank*>& enemyTanks);
    
    /**
     * Extracts rendering data from all tanks (players + enemies).
     * @param players Array of player Tank objects
     * @param enemyTanks Vector of enemy Tank objects
     * @param special Array of special charge values for players
     * @param numPlayers Number of active players
     * @return Vector of TankRenderData structures for all tanks
     */
    static std::vector<TankRenderData> ExtractAllTankData(
        const std::array<Tank, TankHandler::MAX_PLAYERS>& players,
        const std::vector<Tank>& enemyTanks,
        const std::array<float, TankHandler::MAX_PLAYERS>& special,
        int numPlayers);
};
