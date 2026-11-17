#include "GameplayDefinitions.h"
#include <cmath>
#include <stdexcept>
#include <string>
#include <limits>

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
    // Tank still exposes an integer attack field during the compatibility migration.
    Require(static_cast<double>(config.attack) <= std::numeric_limits<int>::max(), path + ".attack");
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
    Require(std::isfinite(definitions.projectile.registrationRadius) && definitions.projectile.registrationRadius > 0,
            "projectile.registrationRadius");
    Require(definitions.enemy.openingLevelWeakenedIndex >= 0, "enemy.openingLevelWeakenedIndex");
    Nonnegative(definitions.enemy.openingLevelAttackScale, "enemy.openingLevelAttackScale");
    Nonnegative(definitions.tank.turboEnergyPerSecond, "tank.turboEnergyPerSecond");
    Nonnegative(definitions.tank.turretInputScale, "tank.turretInputScale");
    Nonnegative(definitions.tank.turretMaxStepDegrees, "tank.turretMaxStepDegrees");
    Nonnegative(definitions.tank.maxForwardInput, "tank.maxForwardInput");
    Nonnegative(definitions.projectile.speed, "projectile.speed");
    Require(std::isfinite(definitions.projectile.lifetimeSeconds) && definitions.projectile.lifetimeSeconds > 0, "projectile.lifetimeSeconds");
    Require(std::isfinite(definitions.projectile.collisionRadius) && definitions.projectile.collisionRadius > 0, "projectile.collisionRadius");
    Nonnegative(definitions.projectile.selfHitDelaySeconds, "projectile.selfHitDelaySeconds");
    Nonnegative(definitions.projectile.purplePrimaryTurnScale, "projectile.purplePrimaryTurnScale");
    Nonnegative(definitions.projectile.purpleSecondaryTurnScale, "projectile.purpleSecondaryTurnScale");
    Nonnegative(definitions.projectile.purpleSecondaryPowerScale, "projectile.purpleSecondaryPowerScale");
    Require(std::isfinite(definitions.projectile.purpleSecondaryChargeDivisor) && definitions.projectile.purpleSecondaryChargeDivisor > 0, "projectile.purpleSecondaryChargeDivisor");
    Nonnegative(definitions.projectile.purpleMinPower, "projectile.purpleMinPower");
    Nonnegative(definitions.projectile.purpleMaxPower, "projectile.purpleMaxPower");
    Nonnegative(definitions.projectile.purpleSpecialTurnAcceleration, "projectile.purpleSpecialTurnAcceleration");
    Nonnegative(definitions.projectile.firstBouncePowerMultiplier, "projectile.firstBouncePowerMultiplier");
    Nonnegative(definitions.projectile.yellowFirstBouncePowerMultiplier, "projectile.yellowFirstBouncePowerMultiplier");
    Require(definitions.projectile.yellowRedChildCount > 0 && definitions.projectile.yellowRedChildCount <= 1024, "projectile.yellowRedChildCount");
    Require(definitions.projectile.yellowBlueChildCount > 0 && definitions.projectile.yellowBlueChildCount <= 1024, "projectile.yellowBlueChildCount");
    Nonnegative(definitions.projectile.yellowBlueChildSpacing, "projectile.yellowBlueChildSpacing");
    Require(definitions.projectile.childBounces >= 0, "projectile.childBounces");
    Require(definitions.projectile.purpleChildBounces >= 0, "projectile.purpleChildBounces");
    Nonnegative(definitions.projectile.purpleChildPowerMultiplier, "projectile.purpleChildPowerMultiplier");
    Nonnegative(definitions.projectile.purpleChildTurnSpeed, "projectile.purpleChildTurnSpeed");
    Nonnegative(definitions.player.spawnHeight, "player.spawnHeight");
    Nonnegative(definitions.player.respawnDelaySeconds, "player.respawnDelaySeconds");
    Nonnegative(definitions.player.versusRespawnDelaySeconds, "player.versusRespawnDelaySeconds");
    Nonnegative(definitions.player.comboDecayPerSecond, "player.comboDecayPerSecond");
    Nonnegative(definitions.player.comboChargeFraction, "player.comboChargeFraction");
    Require(std::isfinite(definitions.player.specialChargeMax) && definitions.player.specialChargeMax > 0, "player.specialChargeMax");
    Nonnegative(definitions.player.matchingPickupEnergyFraction, "player.matchingPickupEnergyFraction");
    Nonnegative(definitions.player.otherPickupEnergyFraction, "player.otherPickupEnergyFraction");
    Require(definitions.player.pickupScore >= 0, "player.pickupScore");
    Require(std::isfinite(definitions.combat.healthReturnDivisor) && definitions.combat.healthReturnDivisor > 0, "combat.healthReturnDivisor");
    Require(std::isfinite(definitions.combat.energyReturnDivisor) && definitions.combat.energyReturnDivisor > 0, "combat.energyReturnDivisor");
    Nonnegative(definitions.combat.friendlyResourceThresholdMultiplier, "combat.friendlyResourceThresholdMultiplier");
    Nonnegative(definitions.combat.blueSpecialHitPowerBonus, "combat.blueSpecialHitPowerBonus");
    Require(std::isfinite(definitions.combat.hitComboDivisor) && definitions.combat.hitComboDivisor > 0, "combat.hitComboDivisor");
    Require(definitions.combat.streakThreshold >= 0, "combat.streakThreshold");
    Require(std::isfinite(definitions.combat.streakComboDivisor) && definitions.combat.streakComboDivisor > 0, "combat.streakComboDivisor");
    Nonnegative(definitions.combat.distanceThreshold, "combat.distanceThreshold");
    Require(std::isfinite(definitions.combat.distanceComboDivisor) && definitions.combat.distanceComboDivisor > 0, "combat.distanceComboDivisor");
    Nonnegative(definitions.combat.bounceBonusMaxAgeSeconds, "combat.bounceBonusMaxAgeSeconds");
    Nonnegative(definitions.combat.bounceBonusBase, "combat.bounceBonusBase");
    Require(std::isfinite(definitions.combat.bounceAgeDivisor) && definitions.combat.bounceAgeDivisor > 0, "combat.bounceAgeDivisor");
    Nonnegative(definitions.combat.killBonus, "combat.killBonus");
    Require(std::isfinite(definitions.combat.killComboDivisor) && definitions.combat.killComboDivisor > 0, "combat.killComboDivisor");
    Require(definitions.projectile.purpleMaxPower >= definitions.projectile.purpleMinPower, "projectile.purpleMaxPower");
    const auto& tank = definitions.tank;
    Require(std::isfinite(tank.maxHealth) && tank.maxHealth > 0, "tank.maxHealth");
    Nonnegative(tank.healthRegenPerSecond, "tank.healthRegenPerSecond");
    Nonnegative(tank.moveSpeed, "tank.moveSpeed");
    Nonnegative(tank.rotationDegreesPerSecond, "tank.rotationDegreesPerSecond");
    Require(std::isfinite(tank.collisionRadius) && tank.collisionRadius > 0, "tank.collisionRadius");
    Nonnegative(tank.jumpEnergyPerSecond, "tank.jumpEnergyPerSecond");
    Nonnegative(tank.jumpLaunchSpeed, "tank.jumpLaunchSpeed");
    Nonnegative(tank.jumpThrust, "tank.jumpThrust");
    Nonnegative(tank.gravity, "tank.gravity");
    Nonnegative(tank.maxRiseSpeed, "tank.maxRiseSpeed");
    Nonnegative(tank.maxFallSpeed, "tank.maxFallSpeed");
    for (size_t i = 0; i < definitions.difficulties.size(); ++i) {
        const auto& difficulty = definitions.difficulties[i];
        const auto path = "difficulty[" + std::to_string(i) + "]";
        Require(std::isfinite(difficulty.health) && difficulty.health > 0, path + ".health");
        Require(std::isfinite(difficulty.energy) && difficulty.energy > 0, path + ".energy");
        Nonnegative(difficulty.healthRegen, path + ".healthRegen");
        Nonnegative(difficulty.energyRegen, path + ".energyRegen");
    }
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
    // Validate composed values too: individually finite multipliers can overflow.
    for (const auto& primary : definitions.types) for (const auto& secondary : definitions.types) {
        ResolveTank(definitions, primary.first, secondary.first,
                    {true, 2, definitions.enemy.openingLevelAttackScale});
        for (int difficulty = 0; difficulty < 3; ++difficulty) {
            const auto resolved = ResolveTank(definitions, primary.first, secondary.first, {false, difficulty, 1});
            ValidateConfig(resolved.weapon, "resolved weapon");
            Require(std::isfinite(resolved.tank.maxHealth) && resolved.tank.maxHealth > 0, "resolved maxHealth");
            Require(std::isfinite(resolved.tank.healthRegenPerSecond), "resolved healthRegenPerSecond");
        }
    }
}

ResolvedTankConfig ResolveTank(const GameplayDefinitions& definitions,
                              TankType primary, TankType secondary, const TankContext& context) {
    Require(context.difficulty >= 0 && context.difficulty < 3, "difficulty index");
    Nonnegative(context.attackScale, "attackScale");
    ResolvedTankConfig result{ResolveTankType(definitions, primary, secondary), definitions.tank};
    result.weapon.attack *= context.attackScale;
    Require(std::isfinite(result.weapon.attack) &&
        static_cast<double>(result.weapon.attack) <= std::numeric_limits<int>::max(), "resolved attack");
    if (context.enemy) {
        result.tank.jumpEnergyPerSecond = 0;
    } else {
        const auto& modifiers = definitions.difficulties[context.difficulty];
        result.tank.maxHealth *= modifiers.health;
        result.tank.healthRegenPerSecond *= modifiers.healthRegen;
        result.weapon.maxEnergy *= modifiers.energy;
        result.weapon.energyRegen *= modifiers.energyRegen;
    }
    return result;
}
}
