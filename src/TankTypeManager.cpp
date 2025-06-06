#include "TankTypeManager.h"

// Define the static member variables
std::map<TankType, TankConfig> TankTypeManager::baseConfigs;
std::map<std::pair<TankType, TankType>, TankConfig> TankTypeManager::comboConfigs;

// Initialize the configs (you can do this in a static initializer or a separate Init function)
bool TankTypeManager::Initialize()
{
    // Default base values
    float defaultFireRate = 0.5f;
    float defaultAttack = 200.0f;
    float defaultMaxCharge = 100.0f;
    float defaultChargeRegen = 50.0f;
    float defaultMoveCost = 0.0f;
    float defaultFireCost = 50.0f;
    float defaultChargeCost = 200.0f;
    int defaultBounces = 0;

    // Base configurations for each tank type
    baseConfigs[TankType::TYPE_GREY] = {
        .fireRate = defaultFireRate,
        .attack = defaultAttack,
        .maxCharge = defaultMaxCharge,
        .chargeRegen = defaultChargeRegen,
        .moveCost = defaultMoveCost,
        .fireCost = defaultFireCost,
        .chargeCost = defaultChargeCost,
        .bounces = defaultBounces,
        .r = 0.5f,
        .g = 0.5f,
        .b = 0.5f};

    baseConfigs[TankType::TYPE_RED] = {
        .fireRate = defaultFireRate / 10.0f,
        .attack = defaultAttack / 2.0f,
        .maxCharge = defaultMaxCharge,
        .chargeRegen = defaultChargeRegen * 1.1f,
        .moveCost = defaultMoveCost,
        .fireCost = defaultMaxCharge / 10.0f,
        .chargeCost = defaultChargeCost,
        .bounces = defaultBounces,
        .r = 1.0f,
        .g = 0.0f,
        .b = 0.0f};

    baseConfigs[TankType::TYPE_BLUE] = {
        .fireRate = defaultFireRate,
        .attack = defaultAttack * 11.0f,
        .maxCharge = defaultMaxCharge,
        .chargeRegen = defaultChargeRegen,
        .moveCost = defaultMoveCost,
        .fireCost = defaultMaxCharge,
        .chargeCost = defaultChargeCost,
        .bounces = defaultBounces,
        .r = 0.0f,
        .g = 0.0f,
        .b = 1.0f};

    baseConfigs[TankType::TYPE_YELLOW] = {
        .fireRate = defaultFireRate,
        .attack = defaultAttack * 1.5f,
        .maxCharge = defaultMaxCharge,
        .chargeRegen = defaultChargeRegen,
        .moveCost = defaultMoveCost,
        .fireCost = defaultMaxCharge / 4.0f,
        .chargeCost = defaultChargeCost,
        .bounces = 400,
        .r = 1.0f,
        .g = 1.0f,
        .b = 0.0f};

    baseConfigs[TankType::TYPE_PURPLE] = {
        .fireRate = defaultFireRate,
        .attack = defaultAttack * 1.5f,
        .maxCharge = defaultMaxCharge,
        .chargeRegen = defaultChargeRegen,
        .moveCost = defaultMoveCost,
        .fireCost = defaultMaxCharge / 4.0f,
        .chargeCost = defaultChargeCost,
        .bounces = 64,
        .r = 1.0f,
        .g = 0.0f,
        .b = 1.0f};

    // Color combo configurations
    comboConfigs[{TankType::TYPE_RED, TankType::TYPE_GREY}] = {
        .fireRate = 0.4f,
        .attack = baseConfigs[TankType::TYPE_RED].attack * 0.5f,
        .fireCost = baseConfigs[TankType::TYPE_RED].fireCost * 2.0f,
    };

    comboConfigs[{TankType::TYPE_RED, TankType::TYPE_BLUE}] = {
        .fireRate = 0.1f,
        .attack = 200.0f,
        .fireCost = baseConfigs[TankType::TYPE_RED].maxCharge / 5.0f,
    };

    comboConfigs[{TankType::TYPE_RED, TankType::TYPE_YELLOW}] = {
        .fireRate = 0.075f,
        .attack = 120.0f,
        .fireCost = baseConfigs[TankType::TYPE_RED].maxCharge / 8.0f,
        .bounces = 2,
    };

    comboConfigs[{TankType::TYPE_BLUE, TankType::TYPE_RED}] = {
        .attack = 200.0f,
        .fireCost = baseConfigs[TankType::TYPE_BLUE].maxCharge / 1.5f,
    };

    comboConfigs[{TankType::TYPE_BLUE, TankType::TYPE_YELLOW}] = {
        .attack = 800.0f,
        .fireCost = baseConfigs[TankType::TYPE_BLUE].maxCharge / 1.5f,
        .bounces = 1,
    };

    comboConfigs[{TankType::TYPE_BLUE, TankType::TYPE_PURPLE}] = {
        .attack = 500.0f,
        .fireCost = baseConfigs[TankType::TYPE_BLUE].maxCharge / 1.5f,
        .bounces = 1,
    };

    comboConfigs[{TankType::TYPE_YELLOW, TankType::TYPE_RED}] = {
        .fireRate = 0.25f,
        .attack = 150.0f,
        .fireCost = baseConfigs[TankType::TYPE_YELLOW].maxCharge / 4.0f,
        .bounces = 64,
    };

    comboConfigs[{TankType::TYPE_YELLOW, TankType::TYPE_BLUE}] = {
        .fireRate = 0.8f,
        .attack = 400.0f,
        .fireCost = baseConfigs[TankType::TYPE_YELLOW].maxCharge / 2.0f,
        .bounces = 32,
    };

    comboConfigs[{TankType::TYPE_YELLOW, TankType::TYPE_PURPLE}] = {
        .fireRate = 0.8f,
        .attack = 500.0f,
        .fireCost = baseConfigs[TankType::TYPE_YELLOW].maxCharge / 2.0f,
        .bounces = 64,
    };

    comboConfigs[{TankType::TYPE_PURPLE, TankType::TYPE_GREY}] = {
        .fireRate = 0.25f,
        .attack = 150.0f,
        .fireCost = baseConfigs[TankType::TYPE_PURPLE].maxCharge / 4.0f,
    };

    comboConfigs[{TankType::TYPE_PURPLE, TankType::TYPE_RED}] = {
        .fireRate = 0.25f,
        .attack = 150.0f,
        .fireCost = baseConfigs[TankType::TYPE_PURPLE].maxCharge / 4.0f,
    };

    comboConfigs[{TankType::TYPE_PURPLE, TankType::TYPE_BLUE}] = {
        .fireRate = 0.8f,
        .attack = 500.0f,
        .fireCost = baseConfigs[TankType::TYPE_PURPLE].maxCharge / 2.0f,
    };

    comboConfigs[{TankType::TYPE_PURPLE, TankType::TYPE_YELLOW}] = {
        .fireRate = 0.8f,
        .attack = 500.0f,
        .fireCost = baseConfigs[TankType::TYPE_PURPLE].maxCharge / 2.0f,
        .bounces = 64,
    };

    return true;
}

// Static initializer to automatically initialize the configs
static bool initialized = TankTypeManager::Initialize();