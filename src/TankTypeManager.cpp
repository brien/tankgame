#include "TankTypeManager.h"
#include "Tank.h"  // For TankType enum

// Define the static member variables
std::map<TankType, TankConfig> TankTypeManager::baseConfigs;
std::map<std::pair<TankType, TankType>, TankConfig> TankTypeManager::comboConfigs;

// Initialize the configs (you can do this in a static initializer or a separate Init function)
bool TankTypeManager::Initialize()
{
    // Default base values
    float defaultFireRate = 0.5f;
    float defaultAttack = 200.0f;
    float defaultMaxEnergy = 100.0f;     // Was: defaultMaxCharge
    float defaultEnergyRegen = 50.0f;    // Was: defaultChargeRegen
    float defaultMoveCost = 0.0f;
    float defaultFireCost = 50.0f;
    float defaultSpecialCost = 200.0f;   // Was: defaultChargeCost
    int defaultBounces = 0;

    // Base configurations for each tank type
    baseConfigs[TankType::TYPE_GREY] = {
        .fireRate = defaultFireRate,
        .attack = defaultAttack,
        .maxEnergy = defaultMaxEnergy,      // Updated field name
        .energyRegen = defaultEnergyRegen,  // Updated field name
        .moveCost = defaultMoveCost,
        .fireCost = defaultFireCost,
        .specialCost = defaultSpecialCost,  // Updated field name
        .bounces = defaultBounces,
        .r = 0.5f, .g = 0.5f, .b = 0.5f};

    baseConfigs[TankType::TYPE_RED] = {
        .fireRate = defaultFireRate / 10.0f,
        .attack = defaultAttack / 2.0f,
        .maxEnergy = defaultMaxEnergy,        // Updated field name
        .energyRegen = defaultEnergyRegen * 1.1f,  // Updated field name
        .moveCost = defaultMoveCost,
        .fireCost = defaultMaxEnergy / 10.0f, // Updated reference
        .specialCost = defaultSpecialCost,    // Updated field name
        .bounces = defaultBounces,
        .r = 1.0f, .g = 0.0f, .b = 0.0f};

    baseConfigs[TankType::TYPE_BLUE] = {
        .fireRate = defaultFireRate,
        .attack = defaultAttack * 11.0f,
        .maxEnergy = defaultMaxEnergy,      // Updated field name
        .energyRegen = defaultEnergyRegen,  // Updated field name
        .moveCost = defaultMoveCost,
        .fireCost = defaultMaxEnergy,       // Updated reference
        .specialCost = defaultSpecialCost,  // Updated field name
        .bounces = defaultBounces,
        .r = 0.0f, .g = 0.0f, .b = 1.0f};

    baseConfigs[TankType::TYPE_YELLOW] = {
        .fireRate = defaultFireRate,
        .attack = defaultAttack * 1.5f,
        .maxEnergy = defaultMaxEnergy,        // Updated field name
        .energyRegen = defaultEnergyRegen,    // Updated field name
        .moveCost = defaultMoveCost,
        .fireCost = defaultMaxEnergy / 4.0f,  // Updated reference
        .specialCost = defaultSpecialCost,    // Updated field name
        .bounces = 400,
        .r = 1.0f, .g = 1.0f, .b = 0.0f};

    baseConfigs[TankType::TYPE_PURPLE] = {
        .fireRate = defaultFireRate,
        .attack = defaultAttack * 1.5f,
        .maxEnergy = defaultMaxEnergy,        // Updated field name
        .energyRegen = defaultEnergyRegen,    // Updated field name
        .moveCost = defaultMoveCost,
        .fireCost = defaultMaxEnergy / 4.0f,  // Updated reference
        .specialCost = defaultSpecialCost,    // Updated field name
        .bounces = 64,
        .r = 1.0f, .g = 0.0f, .b = 1.0f};

    // Color combo configurations
    comboConfigs[{TankType::TYPE_RED, TankType::TYPE_GREY}] = {
        .fireRate = 0.4f,
        .attack = baseConfigs[TankType::TYPE_RED].attack * 0.5f,
        .fireCost = baseConfigs[TankType::TYPE_RED].fireCost * 2.0f,
    };

    comboConfigs[{TankType::TYPE_RED, TankType::TYPE_BLUE}] = {
        .fireRate = 0.1f,
        .attack = 200.0f,
        .fireCost = baseConfigs[TankType::TYPE_RED].maxEnergy / 5.0f,  // Updated field name
    };

    comboConfigs[{TankType::TYPE_RED, TankType::TYPE_YELLOW}] = {
        .fireRate = 0.075f,
        .attack = 120.0f,
        .fireCost = baseConfigs[TankType::TYPE_RED].maxEnergy / 8.0f,  // Updated field name
        .bounces = 2,
    };

    comboConfigs[{TankType::TYPE_BLUE, TankType::TYPE_GREY}] = {
        .attack = 100.0f,
        .fireCost = baseConfigs[TankType::TYPE_BLUE].maxEnergy / 1.5f,  // Updated field name
    };

    comboConfigs[{TankType::TYPE_BLUE, TankType::TYPE_RED}] = {
        .attack = 200.0f,
        .fireCost = baseConfigs[TankType::TYPE_BLUE].maxEnergy / 1.5f,  // Updated field name
    };

    comboConfigs[{TankType::TYPE_BLUE, TankType::TYPE_YELLOW}] = {
        .attack = 800.0f,
        .fireCost = baseConfigs[TankType::TYPE_BLUE].maxEnergy / 1.5f,  // Updated field name
        .bounces = 1,
    };

    comboConfigs[{TankType::TYPE_BLUE, TankType::TYPE_PURPLE}] = {
        .attack = 500.0f,
        .fireCost = baseConfigs[TankType::TYPE_BLUE].maxEnergy / 1.5f,  // Updated field name
        .bounces = 1,
    };

    comboConfigs[{TankType::TYPE_YELLOW, TankType::TYPE_RED}] = {
        .fireRate = 0.25f,
        .attack = 150.0f,
        .fireCost = baseConfigs[TankType::TYPE_YELLOW].maxEnergy / 4.0f,  // Updated field name
        .bounces = 64,
    };

    comboConfigs[{TankType::TYPE_YELLOW, TankType::TYPE_BLUE}] = {
        .fireRate = 0.8f,
        .attack = 400.0f,
        .fireCost = baseConfigs[TankType::TYPE_YELLOW].maxEnergy / 2.0f,  // Updated field name
        .bounces = 32,
    };

    comboConfigs[{TankType::TYPE_YELLOW, TankType::TYPE_PURPLE}] = {
        .fireRate = 0.8f,
        .attack = 500.0f,
        .fireCost = baseConfigs[TankType::TYPE_YELLOW].maxEnergy / 2.0f,  // Updated field name
        .bounces = 64,
    };

    comboConfigs[{TankType::TYPE_PURPLE, TankType::TYPE_GREY}] = {
        .fireRate = 0.25f,
        .attack = 150.0f,
        .fireCost = baseConfigs[TankType::TYPE_PURPLE].maxEnergy / 4.0f,  // Updated field name
    };

    comboConfigs[{TankType::TYPE_PURPLE, TankType::TYPE_RED}] = {
        .fireRate = 0.25f,
        .attack = 150.0f,
        .fireCost = baseConfigs[TankType::TYPE_PURPLE].maxEnergy / 4.0f,  // Updated field name
    };

    comboConfigs[{TankType::TYPE_PURPLE, TankType::TYPE_BLUE}] = {
        .fireRate = 0.8f,
        .attack = 500.0f,
        .fireCost = baseConfigs[TankType::TYPE_PURPLE].maxEnergy / 2.0f,  // Updated field name
    };

    comboConfigs[{TankType::TYPE_PURPLE, TankType::TYPE_YELLOW}] = {
        .fireRate = 0.8f,
        .attack = 500.0f,
        .fireCost = baseConfigs[TankType::TYPE_PURPLE].maxEnergy / 2.0f,  // Updated field name
        .bounces = 64,
    };

    return true;
}

// GetConfig method implementation
TankConfig TankTypeManager::GetConfig(TankType type1, TankType type2)
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

// Color mapping method implementations
void TankTypeManager::GetTankTypeColor(TankType type, float& r, float& g, float& b)
{
    // Ensure configs are initialized
    if (baseConfigs.empty()) {
        Initialize();
    }
    
    auto it = baseConfigs.find(type);
    if (it != baseConfigs.end()) {
        r = it->second.r;
        g = it->second.g;
        b = it->second.b;
    } else {
        // Fallback to grey if type not found
        r = 0.5f;
        g = 0.5f;
        b = 0.5f;
    }
}

void TankTypeManager::GetTankTypeColor(TankType type, float& r, float& g, float& b, float& a)
{
    GetTankTypeColor(type, r, g, b);
    a = 1.0f; // Default alpha for tank colors
}

Color TankTypeManager::GetTankTypeColor(TankType type)
{
    Color color;
    GetTankTypeColor(type, color.r, color.g, color.b, color.a);
    return color;
}

// Static initializer to automatically initialize the configs
static bool initialized = TankTypeManager::Initialize();