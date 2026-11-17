#pragma once

#include "TankType.h"
#include <map>

namespace Gameplay {

// Complete type values. Zero initialization is structural; the provider supplies
// tuning and validation rejects incomplete capacities before use.
struct TankTypeConfig {
    float fireIntervalSeconds = 0;
    float attack = 0;
    float maxEnergy = 0;
    float energyRegen = 0;
    float moveCost = 0;
    float fireCost = 0;
    float specialChargeCost = 0;
    int bounces = 0;
    float r = 0, g = 0, b = 0;
};

// C++14 optional override: explicit zero and an omitted value are different.
template<class T> struct Override {
    bool present = false;
    T value{};
    Override& operator=(T replacement) {
        present = true;
        value = replacement;
        return *this;
    }
    void Apply(T& target) const {
        if (present) target = value;
    }
};

struct TankTypeOverride {
    Override<float> fireIntervalSeconds;
    Override<float> attack;
    Override<float> fireCost;
    Override<float> specialChargeCost;
    Override<int> bounces;
};

struct GameplayDefinitions {
    std::map<TankType, TankTypeConfig> types;
    std::map<std::pair<TankType, TankType>, TankTypeOverride> combinations;
};

// Pure functions: no application singleton, entity mutation, or file access.
TankTypeConfig ResolveTankType(const GameplayDefinitions& definitions,
                               TankType primary, TankType secondary);
// Throws std::invalid_argument with the offending field on invalid definitions.
void ValidateDefinitions(const GameplayDefinitions& definitions);
GameplayDefinitions BuildBuiltInDefinitions();
const GameplayDefinitions& BuiltInDefinitions();

}
