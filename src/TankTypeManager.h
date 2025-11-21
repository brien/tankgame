#pragma once

#include <map>
#include "Color.h"

// Forward declaration
enum class TankType;

struct TankConfig
{
    float fireRate;
    float attack;
    float maxEnergy;        // Was: maxCharge (now consistent with Tank class)
    float energyRegen;      // Was: chargeRegen (now consistent with Tank class)
    float moveCost;
    float fireCost;
    float specialCost;      // Was: chargeCost (now consistent with Tank class)
    int bounces;
    float r, g, b;          // Tank type color
};

class TankTypeManager
{
private:
    static std::map<TankType, TankConfig> baseConfigs;
    static std::map<std::pair<TankType, TankType>, TankConfig> comboConfigs;

public:
    static bool Initialize();
    
    static TankConfig GetConfig(TankType type1, TankType type2);
    
    // Color mapping methods - get color for any tank type
    static void GetTankTypeColor(TankType type, float& r, float& g, float& b);
    static void GetTankTypeColor(TankType type, float& r, float& g, float& b, float& a);
    
    // Convenience method that returns color as Color struct
    static Color GetTankTypeColor(TankType type);
};