#include <gtest/gtest.h>
#include "gameplay/GameplayDefinitions.h"
#include "Tank.h"
#include <limits>
#include <stdexcept>

using namespace Gameplay;

TEST(GameplayDefinitionsTest, AllOrderedPairsHaveExpectedResolvedWeaponValues) {
    const auto definitions = BuildBuiltInDefinitions();
    // Rows are primary, columns secondary: grey, red, blue, yellow, purple.
    const float attacks[5][5] = {
        {200,200,200,200,200}, {50,100,200,120,100},
        {100,200,2200,800,500}, {300,150,400,300,500}, {150,150,500,500,300}
    };
    const float intervals[5][5] = {
        {.5f,.5f,.5f,.5f,.5f}, {.4f,.05f,.1f,.075f,.05f},
        {.5f,.5f,.5f,.5f,.5f}, {.5f,.25f,.8f,.5f,.8f}, {.25f,.25f,.8f,.8f,.5f}
    };
    const float costs[5][5] = {
        {50,50,50,50,50}, {20,10,20,12.5f,10},
        {100/1.5f,100/1.5f,100,100/1.5f,100/1.5f},
        {25,25,50,25,50}, {25,25,50,50,25}
    };
    const int bounces[5][5] = {
        {0,0,0,0,0}, {0,0,0,2,0}, {0,0,0,1,1},
        {400,64,32,400,64}, {64,64,64,64,64}
    };
    for (int p = 0; p < 5; ++p) for (int s = 0; s < 5; ++s) {
        SCOPED_TRACE(p * 5 + s);
        auto config = ResolveTankType(definitions, static_cast<TankType>(p), static_cast<TankType>(s));
        EXPECT_FLOAT_EQ(config.attack, attacks[p][s]);
        EXPECT_FLOAT_EQ(config.fireIntervalSeconds, intervals[p][s]);
        EXPECT_FLOAT_EQ(config.fireCost, costs[p][s]);
        EXPECT_EQ(config.bounces, bounces[p][s]);
        EXPECT_FLOAT_EQ(config.specialChargeCost, 2);
        EXPECT_FLOAT_EQ(config.maxEnergy, 100);
        EXPECT_FLOAT_EQ(config.energyRegen, p == 1 ? 55 : 50);
    }
}

TEST(GameplayDefinitionsTest, MissingOverridesInheritAndExplicitZeroOverridesApply) {
    auto definitions = BuildBuiltInDefinitions();
    auto& combo = definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_GREY}];
    EXPECT_EQ(ResolveTankType(definitions, TankType::TYPE_PURPLE, TankType::TYPE_GREY).bounces, 64);
    combo.bounces = 0;
    combo.fireIntervalSeconds = 0;
    combo.fireCost = 0;
    EXPECT_NO_THROW(ValidateDefinitions(definitions));
    const auto config = ResolveTankType(definitions, TankType::TYPE_PURPLE, TankType::TYPE_GREY);
    EXPECT_EQ(config.bounces, 0);
    EXPECT_FLOAT_EQ(config.fireIntervalSeconds, 0);
    EXPECT_FLOAT_EQ(config.fireCost, 0);
    // Resolving alternate definitions must never modify the built-in provider.
    EXPECT_EQ(TankTypeManager::GetConfig(TankType::TYPE_PURPLE, TankType::TYPE_GREY).bounces, 64);
}

TEST(GameplayDefinitionsTest, InvalidTypesDoNotInsertDefinitions) {
    auto definitions = BuildBuiltInDefinitions();
    const auto size = definitions.types.size();
    EXPECT_THROW(ResolveTankType(definitions, TankType::TANK_TYPE_COUNT, TankType::TYPE_RED), std::invalid_argument);
    EXPECT_THROW(ResolveTankType(definitions, TankType::TYPE_RED, static_cast<TankType>(-1)), std::invalid_argument);
    EXPECT_EQ(definitions.types.size(), size);
    definitions.types.erase(TankType::TYPE_GREY);
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
}

TEST(GameplayDefinitionsTest, ValidationIdentifiesInvalidFieldsAndReferences) {
    auto definitions = BuildBuiltInDefinitions();
    definitions.types.at(TankType::TYPE_RED).maxEnergy = 0;
    try {
        ValidateDefinitions(definitions);
        FAIL() << "Expected invalid capacity";
    } catch (const std::invalid_argument& error) {
        EXPECT_NE(std::string(error.what()).find("maxEnergy"), std::string::npos);
    }
    for (float invalid : {-1.f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()}) {
        definitions = BuildBuiltInDefinitions();
        definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_GREY}].fireCost = invalid;
        EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
    }
    definitions = BuildBuiltInDefinitions();
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TANK_TYPE_COUNT}] = {};
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
    definitions = BuildBuiltInDefinitions();
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_RED}].bounces = -1;
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
}

TEST(GameplayDefinitionsTest, TypeApplicationUsesResolvedCostsAndPreservesLiveState) {
    Tank tank;
    tank.health = 37;
    tank.energy = 71;
    tank.fireTimer = .3f;
    tank.vy = 4;
    tank.SetType(TankType::TYPE_BLUE, TankType::TYPE_GREY);
    EXPECT_FLOAT_EQ(tank.fireCost, 100/1.5f);
    EXPECT_FLOAT_EQ(tank.fireRate, .5f);
    EXPECT_FLOAT_EQ(tank.specialChargeCost, 2);
    EXPECT_FLOAT_EQ(tank.health, 37);
    EXPECT_FLOAT_EQ(tank.energy, 71);
    EXPECT_FLOAT_EQ(tank.fireTimer, .3f);
    EXPECT_FLOAT_EQ(tank.vy, 4);
}
