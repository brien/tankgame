#include "GameplayDefinitions.h"
#include <cmath>
#include <stdexcept>
#include <string>

namespace Gameplay {
namespace {
bool ValidType(TankType type) {
    return type >= TankType::TYPE_GREY && type < TankType::TANK_TYPE_COUNT;
}

void Require(bool valid, const std::string& field) {
    if (!valid) throw std::invalid_argument("GameplayDefinitions: " + field);
}

void Nonnegative(float value, const std::string& field) {
    Require(std::isfinite(value) && value >= 0, field);
}

void ValidateConfig(const TankTypeConfig& config, const std::string& path) {
    Nonnegative(config.fireIntervalSeconds, path + ".fireIntervalSeconds");
    Nonnegative(config.attack, path + ".attack");
    Require(std::isfinite(config.maxEnergy) && config.maxEnergy > 0, path + ".maxEnergy");
    Nonnegative(config.energyRegen, path + ".energyRegen");
    Nonnegative(config.moveCost, path + ".moveCost");
    Nonnegative(config.fireCost, path + ".fireCost");
    Nonnegative(config.specialChargeCost, path + ".specialChargeCost");
    Require(config.bounces >= 0, path + ".bounces");
    Require(std::isfinite(config.r) && config.r >= 0 && config.r <= 1, path + ".r");
    Require(std::isfinite(config.g) && config.g >= 0 && config.g <= 1, path + ".g");
    Require(std::isfinite(config.b) && config.b >= 0 && config.b <= 1, path + ".b");
}
}

TankTypeConfig ResolveTankType(const GameplayDefinitions& definitions,
                               TankType primary, TankType secondary) {
    Require(ValidType(primary) && definitions.types.count(primary), "primary type");
    Require(ValidType(secondary) && definitions.types.count(secondary), "secondary type");
    TankTypeConfig result = definitions.types.at(primary);
    const auto combination = definitions.combinations.find({primary, secondary});
    if (combination != definitions.combinations.end()) {
        const auto& overrides = combination->second;
        overrides.fireIntervalSeconds.Apply(result.fireIntervalSeconds);
        overrides.attack.Apply(result.attack);
        overrides.fireCost.Apply(result.fireCost);
        overrides.specialChargeCost.Apply(result.specialChargeCost);
        overrides.bounces.Apply(result.bounces);
    }
    return result;
}

void ValidateDefinitions(const GameplayDefinitions& definitions) {
    Require(definitions.types.size() == static_cast<size_t>(TankType::TANK_TYPE_COUNT), "type coverage");
    for (int type = 0; type < static_cast<int>(TankType::TANK_TYPE_COUNT); ++type) {
        const auto entry = definitions.types.find(static_cast<TankType>(type));
        Require(entry != definitions.types.end(), "missing type " + std::to_string(type));
        ValidateConfig(entry->second, "type[" + std::to_string(type) + "]");
    }
    for (const auto& entry : definitions.combinations) {
        const auto primary = entry.first.first;
        const auto secondary = entry.first.second;
        Require(ValidType(primary) && ValidType(secondary), "combination type");
        ValidateConfig(ResolveTankType(definitions, primary, secondary),
            "combination[" + std::to_string(static_cast<int>(primary)) + "," +
            std::to_string(static_cast<int>(secondary)) + "]");
    }
}
}
