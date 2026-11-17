#include "GameplayDefinitions.h"

namespace Gameplay {
GameplayDefinitions BuildBuiltInDefinitions()
{
    GameplayDefinitions definitions;

    const float defaultFireRate = 0.5f;
    const float defaultAttack = 200.0f;
    const float defaultMaxEnergy = 100.0f;
    const float defaultEnergyRegen = 50.0f;
    const float defaultMoveCost = 0.0f;
    const float defaultFireCost = 50.0f;
    const float defaultSpecialChargeCost = 2.0f;
    const int defaultBounces = 0;
    definitions.types[TankType::TYPE_GREY].fireIntervalSeconds = defaultFireRate;
    definitions.types[TankType::TYPE_GREY].attack = defaultAttack;
    definitions.types[TankType::TYPE_GREY].maxEnergy = defaultMaxEnergy;
    definitions.types[TankType::TYPE_GREY].energyRegen = defaultEnergyRegen;
    definitions.types[TankType::TYPE_GREY].moveCost = defaultMoveCost;
    definitions.types[TankType::TYPE_GREY].fireCost = defaultFireCost;
    definitions.types[TankType::TYPE_GREY].specialChargeCost = defaultSpecialChargeCost;
    definitions.types[TankType::TYPE_GREY].bounces = defaultBounces;
    definitions.types[TankType::TYPE_GREY].r = 0.5f;
    definitions.types[TankType::TYPE_GREY].g = 0.5f;
    definitions.types[TankType::TYPE_GREY].b = 0.5f;

    definitions.types[TankType::TYPE_RED].fireIntervalSeconds = defaultFireRate / 10.0f;
    definitions.types[TankType::TYPE_RED].attack = defaultAttack / 2.0f;
    definitions.types[TankType::TYPE_RED].maxEnergy = defaultMaxEnergy;
    definitions.types[TankType::TYPE_RED].energyRegen = defaultEnergyRegen * 1.1f;
    definitions.types[TankType::TYPE_RED].moveCost = defaultMoveCost;
    definitions.types[TankType::TYPE_RED].fireCost = defaultMaxEnergy / 10.0f;
    definitions.types[TankType::TYPE_RED].specialChargeCost = defaultSpecialChargeCost;
    definitions.types[TankType::TYPE_RED].bounces = defaultBounces;
    definitions.types[TankType::TYPE_RED].r = 1.0f;
    definitions.types[TankType::TYPE_RED].g = 0.0f;
    definitions.types[TankType::TYPE_RED].b = 0.0f;

    definitions.types[TankType::TYPE_BLUE].fireIntervalSeconds = defaultFireRate;
    definitions.types[TankType::TYPE_BLUE].attack = defaultAttack * 11.0f;
    definitions.types[TankType::TYPE_BLUE].maxEnergy = defaultMaxEnergy;
    definitions.types[TankType::TYPE_BLUE].energyRegen = defaultEnergyRegen;
    definitions.types[TankType::TYPE_BLUE].moveCost = defaultMoveCost;
    definitions.types[TankType::TYPE_BLUE].fireCost = defaultMaxEnergy;
    definitions.types[TankType::TYPE_BLUE].specialChargeCost = defaultSpecialChargeCost;
    definitions.types[TankType::TYPE_BLUE].bounces = defaultBounces;
    definitions.types[TankType::TYPE_BLUE].r = 0.0f;
    definitions.types[TankType::TYPE_BLUE].g = 0.0f;
    definitions.types[TankType::TYPE_BLUE].b = 1.0f;

    definitions.types[TankType::TYPE_YELLOW].fireIntervalSeconds = defaultFireRate;
    definitions.types[TankType::TYPE_YELLOW].attack = defaultAttack * 1.5f;
    definitions.types[TankType::TYPE_YELLOW].maxEnergy = defaultMaxEnergy;
    definitions.types[TankType::TYPE_YELLOW].energyRegen = defaultEnergyRegen;
    definitions.types[TankType::TYPE_YELLOW].moveCost = defaultMoveCost;
    definitions.types[TankType::TYPE_YELLOW].fireCost = defaultMaxEnergy / 4.0f;
    definitions.types[TankType::TYPE_YELLOW].specialChargeCost = defaultSpecialChargeCost;
    definitions.types[TankType::TYPE_YELLOW].bounces = 400;
    definitions.types[TankType::TYPE_YELLOW].r = 1.0f;
    definitions.types[TankType::TYPE_YELLOW].g = 1.0f;
    definitions.types[TankType::TYPE_YELLOW].b = 0.0f;

    definitions.types[TankType::TYPE_PURPLE].fireIntervalSeconds = defaultFireRate;
    definitions.types[TankType::TYPE_PURPLE].attack = defaultAttack * 1.5f;
    definitions.types[TankType::TYPE_PURPLE].maxEnergy = defaultMaxEnergy;
    definitions.types[TankType::TYPE_PURPLE].energyRegen = defaultEnergyRegen;
    definitions.types[TankType::TYPE_PURPLE].moveCost = defaultMoveCost;
    definitions.types[TankType::TYPE_PURPLE].fireCost = defaultMaxEnergy / 4.0f;
    definitions.types[TankType::TYPE_PURPLE].specialChargeCost = defaultSpecialChargeCost;
    definitions.types[TankType::TYPE_PURPLE].bounces = 64;
    definitions.types[TankType::TYPE_PURPLE].r = 1.0f;
    definitions.types[TankType::TYPE_PURPLE].g = 0.0f;
    definitions.types[TankType::TYPE_PURPLE].b = 1.0f;
    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_GREY}].fireIntervalSeconds = 0.4f;
    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_GREY}].attack = definitions.types[TankType::TYPE_RED].attack * 0.5f;
    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_GREY}].fireCost = definitions.types[TankType::TYPE_RED].fireCost * 2.0f;

    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_BLUE}].fireIntervalSeconds = 0.1f;
    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_BLUE}].attack = 200.0f;
    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_BLUE}].fireCost = definitions.types[TankType::TYPE_RED].maxEnergy / 5.0f;

    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_YELLOW}].fireIntervalSeconds = 0.075f;
    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_YELLOW}].attack = 120.0f;
    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_YELLOW}].fireCost = definitions.types[TankType::TYPE_RED].maxEnergy / 8.0f;
    definitions.combinations[{TankType::TYPE_RED, TankType::TYPE_YELLOW}].bounces = 2;

    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_GREY}].attack = 100.0f;
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_GREY}].fireCost = definitions.types[TankType::TYPE_BLUE].maxEnergy / 1.5f;

    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_RED}].attack = 200.0f;
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_RED}].fireCost = definitions.types[TankType::TYPE_BLUE].maxEnergy / 1.5f;

    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_YELLOW}].attack = 800.0f;
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_YELLOW}].fireCost = definitions.types[TankType::TYPE_BLUE].maxEnergy / 1.5f;
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_YELLOW}].bounces = 1;

    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_PURPLE}].attack = 500.0f;
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_PURPLE}].fireCost = definitions.types[TankType::TYPE_BLUE].maxEnergy / 1.5f;
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_PURPLE}].bounces = 1;

    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_RED}].fireIntervalSeconds = 0.25f;
    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_RED}].attack = 150.0f;
    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_RED}].fireCost = definitions.types[TankType::TYPE_YELLOW].maxEnergy / 4.0f;
    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_RED}].bounces = 64;

    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_BLUE}].fireIntervalSeconds = 0.8f;
    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_BLUE}].attack = 400.0f;
    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_BLUE}].fireCost = definitions.types[TankType::TYPE_YELLOW].maxEnergy / 2.0f;
    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_BLUE}].bounces = 32;

    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_PURPLE}].fireIntervalSeconds = 0.8f;
    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_PURPLE}].attack = 500.0f;
    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_PURPLE}].fireCost = definitions.types[TankType::TYPE_YELLOW].maxEnergy / 2.0f;
    definitions.combinations[{TankType::TYPE_YELLOW, TankType::TYPE_PURPLE}].bounces = 64;

    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_GREY}].fireIntervalSeconds = 0.25f;
    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_GREY}].attack = 150.0f;
    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_GREY}].fireCost = definitions.types[TankType::TYPE_PURPLE].maxEnergy / 4.0f;

    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_RED}].fireIntervalSeconds = 0.25f;
    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_RED}].attack = 150.0f;
    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_RED}].fireCost = definitions.types[TankType::TYPE_PURPLE].maxEnergy / 4.0f;

    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_BLUE}].fireIntervalSeconds = 0.8f;
    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_BLUE}].attack = 500.0f;
    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_BLUE}].fireCost = definitions.types[TankType::TYPE_PURPLE].maxEnergy / 2.0f;

    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_YELLOW}].fireIntervalSeconds = 0.8f;
    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_YELLOW}].attack = 500.0f;
    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_YELLOW}].fireCost = definitions.types[TankType::TYPE_PURPLE].maxEnergy / 2.0f;
    definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_YELLOW}].bounces = 64;


    ValidateDefinitions(definitions);
    return definitions;
}

const GameplayDefinitions& BuiltInDefinitions()
{
    // Transitional provider for the legacy static TankTypeManager API.
    // Construct once on first use; no mutable maps or translation-unit initializer.
    static const GameplayDefinitions definitions = BuildBuiltInDefinitions();
    return definitions;
}
}
