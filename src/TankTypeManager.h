#pragma once

#include "gameplay/GameplayDefinitions.h"
#include "Color.h"

using TankConfig = Gameplay::TankTypeConfig;

// Compatibility facade while gameplay consumers migrate to explicit definitions.
class TankTypeManager
{
public:
    static bool Initialize();
    
    static TankConfig GetConfig(TankType type1, TankType type2);
    
    // Color mapping methods - get color for any tank type
    static void GetTankTypeColor(TankType type, float& r, float& g, float& b);
    static void GetTankTypeColor(TankType type, float& r, float& g, float& b, float& a);
    
    // Convenience method that returns color as Color struct
    static Color GetTankTypeColor(TankType type);
};