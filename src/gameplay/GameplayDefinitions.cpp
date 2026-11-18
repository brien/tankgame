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
    // Preserve the historical integer projectile-input power conversion.
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
    Nonnegative(definitions.input.mouseTurretScale, "input.mouseTurretScale");
    Nonnegative(definitions.input.keyboardTurretRate, "input.keyboardTurretRate");
    Nonnegative(definitions.input.keyboardTurboTurretRate, "input.keyboardTurboTurretRate");
    Require(definitions.input.browserPadActivationDeadzone >= 0 && definitions.input.browserPadActivationDeadzone <= 32767, "input.browserPadActivationDeadzone");
    Require(definitions.joystick.forwardDeadzone >= 0 && definitions.joystick.forwardDeadzone <= 32767, "joystick.forwardDeadzone");
    Require(definitions.joystick.reverseDeadzone >= 0 && definitions.joystick.reverseDeadzone <= 32767, "joystick.reverseDeadzone");
    Require(definitions.joystick.bodyDeadzone >= 0 && definitions.joystick.bodyDeadzone <= 32767, "joystick.bodyDeadzone");
    Require(definitions.joystick.turretDeadzone >= 0 && definitions.joystick.turretDeadzone <= 32767, "joystick.turretDeadzone");
    Require(definitions.joystick.alternateTurretDeadzone >= 0 && definitions.joystick.alternateTurretDeadzone <= 32767, "joystick.alternateTurretDeadzone");
    Require(std::isfinite(definitions.joystick.forwardDivisor) && definitions.joystick.forwardDivisor > 0, "joystick.forwardDivisor");
    Require(std::isfinite(definitions.joystick.bodyDivisor) && definitions.joystick.bodyDivisor > 0, "joystick.bodyDivisor");
    Require(std::isfinite(definitions.joystick.turretDivisor) && definitions.joystick.turretDivisor > 0, "joystick.turretDivisor");
    Require(std::isfinite(definitions.joystick.chargeDivisor) && definitions.joystick.chargeDivisor > 0, "joystick.chargeDivisor");
    Require(std::isfinite(definitions.joystick.specialChargeDivisor) && definitions.joystick.specialChargeDivisor > 0, "joystick.specialChargeDivisor");
    Require(definitions.joystick.fireAxisThreshold >= -32768 && definitions.joystick.fireAxisThreshold <= 32767, "joystick.fireAxisThreshold");
    Require(definitions.joystick.jumpAxisThreshold >= -32768 && definitions.joystick.jumpAxisThreshold <= 32767, "joystick.jumpAxisThreshold");
    Require(definitions.gameCube.forwardDeadzone >= 0 && definitions.gameCube.forwardDeadzone <= 32767, "gameCube.forwardDeadzone");
    Require(definitions.gameCube.reverseDeadzone >= 0 && definitions.gameCube.reverseDeadzone <= 32767, "gameCube.reverseDeadzone");
    Require(definitions.gameCube.bodyDeadzone >= 0 && definitions.gameCube.bodyDeadzone <= 32767, "gameCube.bodyDeadzone");
    Require(definitions.gameCube.turretDeadzone >= 0 && definitions.gameCube.turretDeadzone <= 32767, "gameCube.turretDeadzone");
    Require(definitions.gameCube.alternateTurretDeadzone >= 0 && definitions.gameCube.alternateTurretDeadzone <= 32767, "gameCube.alternateTurretDeadzone");
    Require(std::isfinite(definitions.gameCube.forwardDivisor) && definitions.gameCube.forwardDivisor > 0, "gameCube.forwardDivisor");
    Require(std::isfinite(definitions.gameCube.bodyDivisor) && definitions.gameCube.bodyDivisor > 0, "gameCube.bodyDivisor");
    Require(std::isfinite(definitions.gameCube.turretDivisor) && definitions.gameCube.turretDivisor > 0, "gameCube.turretDivisor");
    Require(std::isfinite(definitions.gameCube.chargeDivisor) && definitions.gameCube.chargeDivisor > 0, "gameCube.chargeDivisor");
    Require(std::isfinite(definitions.gameCube.specialChargeDivisor) && definitions.gameCube.specialChargeDivisor > 0, "gameCube.specialChargeDivisor");
    Require(definitions.gameCube.fireAxisThreshold >= -32768 && definitions.gameCube.fireAxisThreshold <= 32767, "gameCube.fireAxisThreshold");
    Require(definitions.gameCube.jumpAxisThreshold >= -32768 && definitions.gameCube.jumpAxisThreshold <= 32767, "gameCube.jumpAxisThreshold");
    Nonnegative(definitions.special.muzzleHeight, "special.muzzleHeight");
    Nonnegative(definitions.special.blueCenterHeight, "special.blueCenterHeight");
    Nonnegative(definitions.special.blueSideOffset, "special.blueSideOffset");
    Require(definitions.special.redArcCount > 0 && definitions.special.redArcCount <= 1024, "special.redArcCount");
    Nonnegative(definitions.special.redArcStepDegrees, "special.redArcStepDegrees");
    Nonnegative(definitions.special.redArcStartDegrees, "special.redArcStartDegrees");
    Require(definitions.special.redArcBounces >= 0, "special.redArcBounces");
    Require(definitions.special.yellowBounces >= 0, "special.yellowBounces");
    Require(std::isfinite(definitions.landing.rewardEnergyFraction) && definitions.landing.rewardEnergyFraction >= 0 && definitions.landing.rewardEnergyFraction <= 1, "landing.rewardEnergyFraction");
    Require(std::isfinite(definitions.landing.rewardThresholdFraction) && definitions.landing.rewardThresholdFraction >= 0 && definitions.landing.rewardThresholdFraction <= 1, "landing.rewardThresholdFraction");
    Nonnegative(definitions.enemyBehavior.activationDistance, "enemyBehavior.activationDistance");
    Nonnegative(definitions.enemyBehavior.activationDistancePerLevel, "enemyBehavior.activationDistancePerLevel");
    Require(std::isfinite(definitions.enemyBehavior.huntEnergyFraction) && definitions.enemyBehavior.huntEnergyFraction >= 0 && definitions.enemyBehavior.huntEnergyFraction <= 1, "enemyBehavior.huntEnergyFraction");
    Require(definitions.enemyBehavior.fearUntilLevelOffset >= 0, "enemyBehavior.fearUntilLevelOffset");
    Require(definitions.enemyBehavior.attackerCountBase >= 0, "enemyBehavior.attackerCountBase");
    Nonnegative(definitions.enemyBehavior.wanderProbeAngleDegrees, "enemyBehavior.wanderProbeAngleDegrees");
    Nonnegative(definitions.enemyBehavior.wanderProbeFrames, "enemyBehavior.wanderProbeFrames");
    Nonnegative(definitions.enemyBehavior.wanderJumpHeight, "enemyBehavior.wanderJumpHeight");
    Nonnegative(definitions.enemyBehavior.wanderMaxDrop, "enemyBehavior.wanderMaxDrop");
    Nonnegative(definitions.enemyBehavior.wanderTurnDegrees, "enemyBehavior.wanderTurnDegrees");
    Require(std::isfinite(definitions.enemyBehavior.jumpEnergyFraction) && definitions.enemyBehavior.jumpEnergyFraction >= 0 && definitions.enemyBehavior.jumpEnergyFraction <= 1, "enemyBehavior.jumpEnergyFraction");
    Nonnegative(definitions.enemyBehavior.aimToleranceDegrees, "enemyBehavior.aimToleranceDegrees");
    Nonnegative(definitions.enemyBehavior.fearProbeAngleDegrees, "enemyBehavior.fearProbeAngleDegrees");
    Nonnegative(definitions.enemyBehavior.fearProbeFrames, "enemyBehavior.fearProbeFrames");
    Nonnegative(definitions.enemyBehavior.fearJumpHeight, "enemyBehavior.fearJumpHeight");
    Nonnegative(definitions.enemyBehavior.avoidanceTurnDegrees, "enemyBehavior.avoidanceTurnDegrees");
    Nonnegative(definitions.enemyBehavior.huntProbeFrames, "enemyBehavior.huntProbeFrames");
    Nonnegative(definitions.enemyBehavior.huntJumpHeight, "enemyBehavior.huntJumpHeight");
    Nonnegative(definitions.enemyBehavior.huntStopDistance, "enemyBehavior.huntStopDistance");
    Require(std::isfinite(definitions.enemyBehavior.fireEnergyFraction) && definitions.enemyBehavior.fireEnergyFraction >= 0 && definitions.enemyBehavior.fireEnergyFraction <= 1, "enemyBehavior.fireEnergyFraction");
    Nonnegative(definitions.enemyBehavior.shotCharge, "enemyBehavior.shotCharge");
    Nonnegative(definitions.enemyBehavior.spawnRotationStepDegrees, "enemyBehavior.spawnRotationStepDegrees");
    Nonnegative(definitions.enemyBehavior.overflowSpawnOffset, "enemyBehavior.overflowSpawnOffset");
    for (const auto* angles : {&definitions.special.redForwardAnglesDegrees, &definitions.special.purpleAnglesDegrees}) {
        Require(!angles->empty() && angles->size() <= 1024, "special angle count");
        for (float angle : *angles) Require(std::isfinite(angle), "special angle degrees");
    }
    Require(std::isfinite(definitions.special.redArcCount * definitions.special.redArcStepDegrees +
                          definitions.special.redArcStartDegrees), "special red arc span");
    for (const auto* types : {&definitions.enemy.openingTypes, &definitions.enemy.regularTypes, &definitions.enemy.advancedTypes}) {
        Require(!types->empty() && types->size() <= 1024, "enemy type cycle size");
        for (const auto& pair : *types) Require(ValidType(pair.first) && ValidType(pair.second), "enemy type cycle reference");
    }
    Require(ValidType(definitions.enemy.openingWeakenedPrimary) && ValidType(definitions.enemy.openingWeakenedSecondary),
            "enemy opening weakened types");
    for (int offset : definitions.enemy.advancedLevelOffsets) Require(offset >= 0, "enemy advancedLevelOffsets");

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
    Require(ValidType(definitions.player.startingPrimaryType) && ValidType(definitions.player.startingSecondaryType), "player starting types");
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
