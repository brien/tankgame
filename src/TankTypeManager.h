#include "Tank.h"
#include <map>

struct TankConfig
{
    float fireRate;
    float attack;
    float maxCharge;
    float chargeRegen;
    float moveCost;
    float fireCost;
    float chargeCost;
    int bounces;
    float r, g, b;
};

class TankTypeManager
{
private:
    static std::map<TankType, TankConfig> baseConfigs;
    static std::map<std::pair<TankType, TankType>, TankConfig> comboConfigs;

public:
    static bool Initialize();
    
    static TankConfig GetConfig(TankType type1, TankType type2)
    {
        TankConfig config = baseConfigs[type1];

        auto combo = std::make_pair(type1, type2);
        if (comboConfigs.find(combo) != comboConfigs.end())
        {
            // Apply combo modifications
            auto comboConfig = comboConfigs[combo];
            config.fireRate = comboConfig.fireRate;
            config.attack = comboConfig.attack;
            config.fireCost = comboConfig.fireCost;
            config.bounces = comboConfig.bounces;
        }

        return config;
    }
};