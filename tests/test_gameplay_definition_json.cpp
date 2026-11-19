#include <gtest/gtest.h>
#include "gameplay/GameplayDefinitionJson.h"
#include "gameplay/GameplayDefinitionLoader.h"
#include "GameTask.h"
#include "App.h"
#include "LevelHandler.h"
#include "TankHandler.h"
#include "Bullet.h"
#include "events/Events.h"
#include "Tank.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iterator>
#include <cstdio>
#include <limits>
#include <chrono>
#include <algorithm>

using namespace Gameplay;
using Json = nlohmann::json;
namespace {
void Equal(const TankTypeConfig& a, const TankTypeConfig& b) {
    EXPECT_EQ(a.fireIntervalSeconds, b.fireIntervalSeconds) << "TankTypeConfig.fireIntervalSeconds";
    EXPECT_EQ(a.attack, b.attack) << "TankTypeConfig.attack";
    EXPECT_EQ(a.maxEnergy, b.maxEnergy) << "TankTypeConfig.maxEnergy";
    EXPECT_EQ(a.energyRegen, b.energyRegen) << "TankTypeConfig.energyRegen";
    EXPECT_EQ(a.moveCost, b.moveCost) << "TankTypeConfig.moveCost";
    EXPECT_EQ(a.fireCost, b.fireCost) << "TankTypeConfig.fireCost";
    EXPECT_EQ(a.specialChargeCost, b.specialChargeCost) << "TankTypeConfig.specialChargeCost";
    EXPECT_EQ(a.bounces, b.bounces) << "TankTypeConfig.bounces";
    EXPECT_EQ(a.r, b.r) << "TankTypeConfig.r";
    EXPECT_EQ(a.g, b.g) << "TankTypeConfig.g";
    EXPECT_EQ(a.b, b.b) << "TankTypeConfig.b";
}
void Equal(const TankDefaults& a, const TankDefaults& b) {
    EXPECT_EQ(a.maxHealth, b.maxHealth) << "TankDefaults.maxHealth";
    EXPECT_EQ(a.healthRegenPerSecond, b.healthRegenPerSecond) << "TankDefaults.healthRegenPerSecond";
    EXPECT_EQ(a.moveSpeed, b.moveSpeed) << "TankDefaults.moveSpeed";
    EXPECT_EQ(a.rotationDegreesPerSecond, b.rotationDegreesPerSecond) << "TankDefaults.rotationDegreesPerSecond";
    EXPECT_EQ(a.collisionRadius, b.collisionRadius) << "TankDefaults.collisionRadius";
    EXPECT_EQ(a.jumpEnergyPerSecond, b.jumpEnergyPerSecond) << "TankDefaults.jumpEnergyPerSecond";
    EXPECT_EQ(a.jumpLaunchSpeed, b.jumpLaunchSpeed) << "TankDefaults.jumpLaunchSpeed";
    EXPECT_EQ(a.jumpThrust, b.jumpThrust) << "TankDefaults.jumpThrust";
    EXPECT_EQ(a.gravity, b.gravity) << "TankDefaults.gravity";
    EXPECT_EQ(a.maxRiseSpeed, b.maxRiseSpeed) << "TankDefaults.maxRiseSpeed";
    EXPECT_EQ(a.maxFallSpeed, b.maxFallSpeed) << "TankDefaults.maxFallSpeed";
    EXPECT_EQ(a.turboEnergyPerSecond, b.turboEnergyPerSecond) << "TankDefaults.turboEnergyPerSecond";
    EXPECT_EQ(a.turretInputScale, b.turretInputScale) << "TankDefaults.turretInputScale";
    EXPECT_EQ(a.turretMaxStepDegrees, b.turretMaxStepDegrees) << "TankDefaults.turretMaxStepDegrees";
    EXPECT_EQ(a.maxForwardInput, b.maxForwardInput) << "TankDefaults.maxForwardInput";
}
void Equal(const ProjectileRules& a, const ProjectileRules& b) {
    EXPECT_EQ(a.speed, b.speed) << "ProjectileRules.speed";
    EXPECT_EQ(a.lifetimeSeconds, b.lifetimeSeconds) << "ProjectileRules.lifetimeSeconds";
    EXPECT_EQ(a.collisionRadius, b.collisionRadius) << "ProjectileRules.collisionRadius";
    EXPECT_EQ(a.registrationRadius, b.registrationRadius) << "ProjectileRules.registrationRadius";
    EXPECT_EQ(a.selfHitDelaySeconds, b.selfHitDelaySeconds) << "ProjectileRules.selfHitDelaySeconds";
    EXPECT_EQ(a.purplePrimaryTurnScale, b.purplePrimaryTurnScale) << "ProjectileRules.purplePrimaryTurnScale";
    EXPECT_EQ(a.purpleSecondaryTurnScale, b.purpleSecondaryTurnScale) << "ProjectileRules.purpleSecondaryTurnScale";
    EXPECT_EQ(a.purpleSecondaryPowerScale, b.purpleSecondaryPowerScale) << "ProjectileRules.purpleSecondaryPowerScale";
    EXPECT_EQ(a.purpleSecondaryChargeDivisor, b.purpleSecondaryChargeDivisor) << "ProjectileRules.purpleSecondaryChargeDivisor";
    EXPECT_EQ(a.purpleMinPower, b.purpleMinPower) << "ProjectileRules.purpleMinPower";
    EXPECT_EQ(a.purpleMaxPower, b.purpleMaxPower) << "ProjectileRules.purpleMaxPower";
    EXPECT_EQ(a.purpleSpecialTurnAcceleration, b.purpleSpecialTurnAcceleration) << "ProjectileRules.purpleSpecialTurnAcceleration";
    EXPECT_EQ(a.firstBouncePowerMultiplier, b.firstBouncePowerMultiplier) << "ProjectileRules.firstBouncePowerMultiplier";
    EXPECT_EQ(a.yellowFirstBouncePowerMultiplier, b.yellowFirstBouncePowerMultiplier) << "ProjectileRules.yellowFirstBouncePowerMultiplier";
    EXPECT_EQ(a.yellowRedChildCount, b.yellowRedChildCount) << "ProjectileRules.yellowRedChildCount";
    EXPECT_EQ(a.yellowBlueChildCount, b.yellowBlueChildCount) << "ProjectileRules.yellowBlueChildCount";
    EXPECT_EQ(a.yellowBlueChildSpacing, b.yellowBlueChildSpacing) << "ProjectileRules.yellowBlueChildSpacing";
    EXPECT_EQ(a.childBounces, b.childBounces) << "ProjectileRules.childBounces";
    EXPECT_EQ(a.purpleChildBounces, b.purpleChildBounces) << "ProjectileRules.purpleChildBounces";
    EXPECT_EQ(a.purpleChildPowerMultiplier, b.purpleChildPowerMultiplier) << "ProjectileRules.purpleChildPowerMultiplier";
    EXPECT_EQ(a.purpleChildTurnSpeed, b.purpleChildTurnSpeed) << "ProjectileRules.purpleChildTurnSpeed";
}
void Equal(const PlayerRules& a, const PlayerRules& b) {
    EXPECT_EQ(a.startingPrimaryType, b.startingPrimaryType) << "PlayerRules.startingPrimaryType";
    EXPECT_EQ(a.startingSecondaryType, b.startingSecondaryType) << "PlayerRules.startingSecondaryType";
    EXPECT_EQ(a.spawnHeight, b.spawnHeight) << "PlayerRules.spawnHeight";
    EXPECT_EQ(a.respawnDelaySeconds, b.respawnDelaySeconds) << "PlayerRules.respawnDelaySeconds";
    EXPECT_EQ(a.versusRespawnDelaySeconds, b.versusRespawnDelaySeconds) << "PlayerRules.versusRespawnDelaySeconds";
    EXPECT_EQ(a.comboDecayPerSecond, b.comboDecayPerSecond) << "PlayerRules.comboDecayPerSecond";
    EXPECT_EQ(a.comboChargeFraction, b.comboChargeFraction) << "PlayerRules.comboChargeFraction";
    EXPECT_EQ(a.specialChargeMax, b.specialChargeMax) << "PlayerRules.specialChargeMax";
    EXPECT_EQ(a.matchingPickupEnergyFraction, b.matchingPickupEnergyFraction) << "PlayerRules.matchingPickupEnergyFraction";
    EXPECT_EQ(a.otherPickupEnergyFraction, b.otherPickupEnergyFraction) << "PlayerRules.otherPickupEnergyFraction";
    EXPECT_EQ(a.pickupScore, b.pickupScore) << "PlayerRules.pickupScore";
}
void Equal(const CombatRules& a, const CombatRules& b) {
    EXPECT_EQ(a.healthReturnDivisor, b.healthReturnDivisor) << "CombatRules.healthReturnDivisor";
    EXPECT_EQ(a.energyReturnDivisor, b.energyReturnDivisor) << "CombatRules.energyReturnDivisor";
    EXPECT_EQ(a.friendlyResourceThresholdMultiplier, b.friendlyResourceThresholdMultiplier) << "CombatRules.friendlyResourceThresholdMultiplier";
    EXPECT_EQ(a.blueSpecialHitPowerBonus, b.blueSpecialHitPowerBonus) << "CombatRules.blueSpecialHitPowerBonus";
    EXPECT_EQ(a.hitComboDivisor, b.hitComboDivisor) << "CombatRules.hitComboDivisor";
    EXPECT_EQ(a.streakThreshold, b.streakThreshold) << "CombatRules.streakThreshold";
    EXPECT_EQ(a.streakComboDivisor, b.streakComboDivisor) << "CombatRules.streakComboDivisor";
    EXPECT_EQ(a.distanceThreshold, b.distanceThreshold) << "CombatRules.distanceThreshold";
    EXPECT_EQ(a.distanceComboDivisor, b.distanceComboDivisor) << "CombatRules.distanceComboDivisor";
    EXPECT_EQ(a.bounceBonusMaxAgeSeconds, b.bounceBonusMaxAgeSeconds) << "CombatRules.bounceBonusMaxAgeSeconds";
    EXPECT_EQ(a.bounceBonusBase, b.bounceBonusBase) << "CombatRules.bounceBonusBase";
    EXPECT_EQ(a.bounceAgeDivisor, b.bounceAgeDivisor) << "CombatRules.bounceAgeDivisor";
    EXPECT_EQ(a.killBonus, b.killBonus) << "CombatRules.killBonus";
    EXPECT_EQ(a.killComboDivisor, b.killComboDivisor) << "CombatRules.killComboDivisor";
}
void Equal(const InputRules& a, const InputRules& b) {
    EXPECT_EQ(a.mouseTurretScale, b.mouseTurretScale) << "InputRules.mouseTurretScale";
    EXPECT_EQ(a.keyboardTurretRate, b.keyboardTurretRate) << "InputRules.keyboardTurretRate";
    EXPECT_EQ(a.keyboardTurboTurretRate, b.keyboardTurboTurretRate) << "InputRules.keyboardTurboTurretRate";
    EXPECT_EQ(a.browserPadActivationDeadzone, b.browserPadActivationDeadzone) << "InputRules.browserPadActivationDeadzone";
}
void Equal(const JoystickRules& a, const JoystickRules& b) {
    EXPECT_EQ(a.forwardDeadzone, b.forwardDeadzone) << "JoystickRules.forwardDeadzone";
    EXPECT_EQ(a.reverseDeadzone, b.reverseDeadzone) << "JoystickRules.reverseDeadzone";
    EXPECT_EQ(a.bodyDeadzone, b.bodyDeadzone) << "JoystickRules.bodyDeadzone";
    EXPECT_EQ(a.turretDeadzone, b.turretDeadzone) << "JoystickRules.turretDeadzone";
    EXPECT_EQ(a.alternateTurretDeadzone, b.alternateTurretDeadzone) << "JoystickRules.alternateTurretDeadzone";
    EXPECT_EQ(a.forwardDivisor, b.forwardDivisor) << "JoystickRules.forwardDivisor";
    EXPECT_EQ(a.bodyDivisor, b.bodyDivisor) << "JoystickRules.bodyDivisor";
    EXPECT_EQ(a.turretDivisor, b.turretDivisor) << "JoystickRules.turretDivisor";
    EXPECT_EQ(a.chargeDivisor, b.chargeDivisor) << "JoystickRules.chargeDivisor";
    EXPECT_EQ(a.specialChargeDivisor, b.specialChargeDivisor) << "JoystickRules.specialChargeDivisor";
    EXPECT_EQ(a.fireAxisThreshold, b.fireAxisThreshold) << "JoystickRules.fireAxisThreshold";
    EXPECT_EQ(a.jumpAxisThreshold, b.jumpAxisThreshold) << "JoystickRules.jumpAxisThreshold";
}
void Equal(const SpecialRules& a, const SpecialRules& b) {
    EXPECT_EQ(a.muzzleHeight, b.muzzleHeight) << "SpecialRules.muzzleHeight";
    EXPECT_EQ(a.blueCenterHeight, b.blueCenterHeight) << "SpecialRules.blueCenterHeight";
    EXPECT_EQ(a.blueSideOffset, b.blueSideOffset) << "SpecialRules.blueSideOffset";
    EXPECT_EQ(a.redArcCount, b.redArcCount) << "SpecialRules.redArcCount";
    EXPECT_EQ(a.redArcStepDegrees, b.redArcStepDegrees) << "SpecialRules.redArcStepDegrees";
    EXPECT_EQ(a.redArcStartDegrees, b.redArcStartDegrees) << "SpecialRules.redArcStartDegrees";
    EXPECT_EQ(a.redArcBounces, b.redArcBounces) << "SpecialRules.redArcBounces";
    EXPECT_EQ(a.yellowBounces, b.yellowBounces) << "SpecialRules.yellowBounces";
    EXPECT_EQ(a.redForwardAnglesDegrees, b.redForwardAnglesDegrees) << "SpecialRules.redForwardAnglesDegrees";
    EXPECT_EQ(a.purpleAnglesDegrees, b.purpleAnglesDegrees) << "SpecialRules.purpleAnglesDegrees";
}
void Equal(const LandingRules& a, const LandingRules& b) {
    EXPECT_EQ(a.rewardEnergyFraction, b.rewardEnergyFraction) << "LandingRules.rewardEnergyFraction";
    EXPECT_EQ(a.rewardThresholdFraction, b.rewardThresholdFraction) << "LandingRules.rewardThresholdFraction";
}
void Equal(const EnemyBehaviorRules& a, const EnemyBehaviorRules& b) {
    EXPECT_EQ(a.activationDistance, b.activationDistance) << "EnemyBehaviorRules.activationDistance";
    EXPECT_EQ(a.activationDistancePerLevel, b.activationDistancePerLevel) << "EnemyBehaviorRules.activationDistancePerLevel";
    EXPECT_EQ(a.huntEnergyFraction, b.huntEnergyFraction) << "EnemyBehaviorRules.huntEnergyFraction";
    EXPECT_EQ(a.fearUntilLevelOffset, b.fearUntilLevelOffset) << "EnemyBehaviorRules.fearUntilLevelOffset";
    EXPECT_EQ(a.attackerCountBase, b.attackerCountBase) << "EnemyBehaviorRules.attackerCountBase";
    EXPECT_EQ(a.wanderProbeAngleDegrees, b.wanderProbeAngleDegrees) << "EnemyBehaviorRules.wanderProbeAngleDegrees";
    EXPECT_EQ(a.wanderProbeFrames, b.wanderProbeFrames) << "EnemyBehaviorRules.wanderProbeFrames";
    EXPECT_EQ(a.wanderJumpHeight, b.wanderJumpHeight) << "EnemyBehaviorRules.wanderJumpHeight";
    EXPECT_EQ(a.wanderMaxDrop, b.wanderMaxDrop) << "EnemyBehaviorRules.wanderMaxDrop";
    EXPECT_EQ(a.wanderTurnDegrees, b.wanderTurnDegrees) << "EnemyBehaviorRules.wanderTurnDegrees";
    EXPECT_EQ(a.jumpEnergyFraction, b.jumpEnergyFraction) << "EnemyBehaviorRules.jumpEnergyFraction";
    EXPECT_EQ(a.aimToleranceDegrees, b.aimToleranceDegrees) << "EnemyBehaviorRules.aimToleranceDegrees";
    EXPECT_EQ(a.fearProbeAngleDegrees, b.fearProbeAngleDegrees) << "EnemyBehaviorRules.fearProbeAngleDegrees";
    EXPECT_EQ(a.fearProbeFrames, b.fearProbeFrames) << "EnemyBehaviorRules.fearProbeFrames";
    EXPECT_EQ(a.fearJumpHeight, b.fearJumpHeight) << "EnemyBehaviorRules.fearJumpHeight";
    EXPECT_EQ(a.avoidanceTurnDegrees, b.avoidanceTurnDegrees) << "EnemyBehaviorRules.avoidanceTurnDegrees";
    EXPECT_EQ(a.huntProbeFrames, b.huntProbeFrames) << "EnemyBehaviorRules.huntProbeFrames";
    EXPECT_EQ(a.huntJumpHeight, b.huntJumpHeight) << "EnemyBehaviorRules.huntJumpHeight";
    EXPECT_EQ(a.huntStopDistance, b.huntStopDistance) << "EnemyBehaviorRules.huntStopDistance";
    EXPECT_EQ(a.fireEnergyFraction, b.fireEnergyFraction) << "EnemyBehaviorRules.fireEnergyFraction";
    EXPECT_EQ(a.shotCharge, b.shotCharge) << "EnemyBehaviorRules.shotCharge";
    EXPECT_EQ(a.spawnRotationStepDegrees, b.spawnRotationStepDegrees) << "EnemyBehaviorRules.spawnRotationStepDegrees";
    EXPECT_EQ(a.overflowSpawnOffset, b.overflowSpawnOffset) << "EnemyBehaviorRules.overflowSpawnOffset";
}
void Equal(const EnemyRules& a, const EnemyRules& b) {
    EXPECT_EQ(a.openingLevelWeakenedIndex, b.openingLevelWeakenedIndex) << "EnemyRules.openingLevelWeakenedIndex";
    EXPECT_EQ(a.openingLevelAttackScale, b.openingLevelAttackScale) << "EnemyRules.openingLevelAttackScale";
    EXPECT_EQ(a.openingTypes, b.openingTypes) << "EnemyRules.openingTypes";
    EXPECT_EQ(a.regularTypes, b.regularTypes) << "EnemyRules.regularTypes";
    EXPECT_EQ(a.advancedTypes, b.advancedTypes) << "EnemyRules.advancedTypes";
    EXPECT_EQ(a.advancedLevelOffsets, b.advancedLevelOffsets) << "EnemyRules.advancedLevelOffsets";
    EXPECT_EQ(a.openingWeakenedPrimary, b.openingWeakenedPrimary) << "EnemyRules.openingWeakenedPrimary";
    EXPECT_EQ(a.openingWeakenedSecondary, b.openingWeakenedSecondary) << "EnemyRules.openingWeakenedSecondary";
}
void Equal(const DifficultyModifiers& a, const DifficultyModifiers& b) {
    EXPECT_EQ(a.health, b.health) << "DifficultyModifiers.health";
    EXPECT_EQ(a.healthRegen, b.healthRegen) << "DifficultyModifiers.healthRegen";
    EXPECT_EQ(a.energy, b.energy) << "DifficultyModifiers.energy";
    EXPECT_EQ(a.energyRegen, b.energyRegen) << "DifficultyModifiers.energyRegen";
}
void Equal(const GameplayDefinitions& a, const GameplayDefinitions& b) {
    Equal(a.tank, b.tank);
    Equal(a.projectile, b.projectile);
    Equal(a.player, b.player);
    Equal(a.combat, b.combat);
    Equal(a.enemy, b.enemy);
    Equal(a.input, b.input);
    Equal(a.joystick, b.joystick);
    Equal(a.special, b.special);
    Equal(a.landing, b.landing);
    Equal(a.enemyBehavior, b.enemyBehavior);
    Equal(a.gameCube, b.gameCube);
    for (int i = 0; i < 3; ++i) Equal(a.difficulties[i], b.difficulties[i]);
    ASSERT_EQ(a.types.size(), b.types.size());
    for (const auto& entry : a.types) Equal(entry.second, b.types.at(entry.first));
    ASSERT_EQ(a.combinations.size(), b.combinations.size());
    for (const auto& entry : a.combinations) {
        const auto& x = entry.second;
        const auto& y = b.combinations.at(entry.first);
        EXPECT_EQ(x.fireIntervalSeconds.present, y.fireIntervalSeconds.present);
        if (x.fireIntervalSeconds.present) EXPECT_EQ(x.fireIntervalSeconds.value, y.fireIntervalSeconds.value);
        EXPECT_EQ(x.attack.present, y.attack.present);
        if (x.attack.present) EXPECT_EQ(x.attack.value, y.attack.value);
        EXPECT_EQ(x.fireCost.present, y.fireCost.present);
        if (x.fireCost.present) EXPECT_EQ(x.fireCost.value, y.fireCost.value);
        EXPECT_EQ(x.specialChargeCost.present, y.specialChargeCost.present);
        if (x.specialChargeCost.present) EXPECT_EQ(x.specialChargeCost.value, y.specialChargeCost.value);
        EXPECT_EQ(x.bounces.present, y.bounces.present);
        if (x.bounces.present) EXPECT_EQ(x.bounces.value, y.bounces.value);
    }
}
Json BuiltInJson() { return Json::parse(SerializeDefinitionJson(BuildBuiltInDefinitions())); }
void Reject(const Json& j, const std::string& diagnostic) {
    try { ParseDefinitionJson(j.dump()); FAIL() << "Accepted invalid configuration"; }
    catch (const std::invalid_argument& e) { EXPECT_NE(std::string(e.what()).find(diagnostic), std::string::npos) << e.what(); }
}
// Mutate all serialized numeric leaves together, preserving ordering/ranges.
void Retune(Json& j) {
    if (j.is_object() || j.is_array()) { for (auto& child : j) Retune(child); }
    else if (j.is_number_integer()) j = j.get<int>() + 1;
    else if (j.is_number_float()) j = j.get<float>() == 0 ? 0.01f : j.get<float>() * 0.95f;
    else if (j.is_string()) {
        const auto name = j.get<std::string>();
        if (name == "grey") j = "red";
        else if (name == "red") j = "blue";
        else if (name == "blue") j = "yellow";
        else if (name == "yellow") j = "purple";
        else if (name == "purple") j = "grey";
    }
}
struct TemporaryDefinition {
    std::string path;
    TemporaryDefinition() {
        // GoogleTest provides a platform-specific temporary directory.
        static int sequence = 0;
        path = ::testing::TempDir() + "tankgame-definition-" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()) + "-" + std::to_string(++sequence) + ".json";
    }
    ~TemporaryDefinition() { std::remove(path.c_str()); }
    void Write(const std::string& text) { std::ofstream output(path); output << text; }
};
DefinitionOptions Options(std::vector<std::string> args) {
    std::vector<char*> argv;
    for (auto& arg : args) argv.push_back(&arg[0]);
    return ParseDefinitionOptions(static_cast<int>(argv.size()), argv.data());
}
}

TEST(GameplayDefinitionJson, BuiltInsRoundTripEveryFieldAndResolvedPair) {
    const auto original = BuildBuiltInDefinitions();
    const auto text = SerializeDefinitionJson(original);
    const auto loaded = ParseDefinitionJson(text);
    Equal(original, loaded);
    EXPECT_EQ(text, SerializeDefinitionJson(loaded));
    for (const auto& primary : original.types) for (const auto& secondary : original.types) {
        for (int difficulty = 0; difficulty < 3; ++difficulty) {
            const auto a = ResolveTank(original, primary.first, secondary.first, {false, difficulty, 1});
            const auto b = ResolveTank(loaded, primary.first, secondary.first, {false, difficulty, 1});
            Equal(a.tank, b.tank); Equal(a.weapon, b.weapon);
        }
        const auto a = ResolveTank(original, primary.first, secondary.first, {true, 2, original.enemy.openingLevelAttackScale});
        const auto b = ResolveTank(loaded, primary.first, secondary.first, {true, 2, loaded.enemy.openingLevelAttackScale});
        Equal(a.tank, b.tank); Equal(a.weapon, b.weapon);
    }
}

TEST(GameplayDefinitionJson, AlternateValuesRoundTripAndPreserveSequence) {
    auto document = BuiltInJson();
    Retune(document["definitions"]);
    auto candidate = ParseDefinitionJson(document.dump());
    auto exported = Json::parse(SerializeDefinitionJson(candidate));
    // Combination records serialize in canonical map order; compare their values
    // independently of that order. Everything else must retain its exact shape.
    auto order = [](const Json& a, const Json& b) {
        return std::make_pair(a.at("primary").get<std::string>(), a.at("secondary").get<std::string>()) <
               std::make_pair(b.at("primary").get<std::string>(), b.at("secondary").get<std::string>());
    };
    auto& expectedPairs = document["definitions"]["combinations"];
    auto& actualPairs = exported["definitions"]["combinations"];
    std::sort(expectedPairs.begin(), expectedPairs.end(), order);
    std::sort(actualPairs.begin(), actualPairs.end(), order);
    EXPECT_EQ(document, exported);
    candidate.enemy.openingTypes.push_back(candidate.enemy.openingTypes.front());
    Equal(candidate, ParseDefinitionJson(SerializeDefinitionJson(candidate)));
}

TEST(GameplayDefinitionJson, OverridesDistinguishOmissionZeroAndOrderedPairs) {
    auto j = BuiltInJson();
    j["definitions"]["combinations"] = Json::array();
    auto inherited = ParseDefinitionJson(j.dump());
    EXPECT_FLOAT_EQ(ResolveTankType(inherited, TankType::TYPE_BLUE, TankType::TYPE_GREY).fireIntervalSeconds, 0.5f);
    j["definitions"]["combinations"].push_back({{"primary", "blue"}, {"secondary", "grey"}, {"overrides", {{"fireIntervalSeconds", 0}, {"fireCost", 0}}}});
    auto zero = ParseDefinitionJson(j.dump());
    EXPECT_FLOAT_EQ(ResolveTankType(zero, TankType::TYPE_BLUE, TankType::TYPE_GREY).fireIntervalSeconds, 0);
    EXPECT_GT(ResolveTankType(zero, TankType::TYPE_GREY, TankType::TYPE_BLUE).fireCost, 0);
    Equal(zero, ParseDefinitionJson(SerializeDefinitionJson(zero)));
    j["definitions"]["combinations"][0]["overrides"] = Json::object();
    EXPECT_NO_THROW(ParseDefinitionJson(j.dump()));
}

TEST(GameplayDefinitionJson, RejectsInvalidStructureAndNumbersWithDiagnostics) {
    auto original = BuiltInJson();
    for (const auto& version : {Json(), Json(true), Json(1.0), Json(2), Json("1")}) {
        auto j = original; j["schemaVersion"] = version; Reject(j, "schemaVersion");
    }
    auto j = original; j.erase("schemaVersion"); Reject(j, "schemaVersion");
    j = original; j["unexpected"] = 1; Reject(j, "unexpected");
    j = original; j["definitions"].erase("tank"); Reject(j, "tank");
    j = original; j["definitions"]["types"].erase("blue"); Reject(j, "blue");
    j = original; j["definitions"]["difficulties"].erase("easy"); Reject(j, "easy");
    j = original; j["definitions"]["player"]["startingPrimaryType"] = "green"; Reject(j, "startingPrimaryType");
    for (const auto& value : {Json(), Json(true), Json("4"), Json(1e100), Json(-1)}) {
        j = original; j["definitions"]["tank"]["maxHealth"] = value; Reject(j, "maxHealth");
    }
    for (const auto& value : {Json(1.5), Json(1.0), Json(true), Json(4294967296ULL), Json(-2147483649LL)}) {
        j = original; j["definitions"]["player"]["pickupScore"] = value; Reject(j, "pickupScore");
    }
    j = original; j["definitions"]["tank"]["typo"] = 1; Reject(j, "typo");
    j = original; j["definitions"]["tank"].erase("gravity"); Reject(j, "gravity");
    j = original; j["definitions"]["combinations"].push_back(j["definitions"]["combinations"][0]); Reject(j, "duplicate ordered pair");
    j = original; j["definitions"]["combinations"][0]["overrides"]["maxEnergy"] = 2; Reject(j, "maxEnergy");
    j = original; j["definitions"]["enemy"]["openingTypes"][0]["secondary"] = 0; Reject(j, "secondary");
    j = original; j["definitions"]["special"]["purpleAnglesDegrees"] = Json::array(); Reject(j, "angle count");
}

TEST(GameplayDefinitionJson, RejectsDuplicateKeysTrailingDataAndResourceLimits) {
    const auto text = SerializeDefinitionJson(BuildBuiltInDefinitions());
    EXPECT_THROW(ParseDefinitionJson("{\"schemaVersion\":1,\"schemaVersion\":1}"), std::invalid_argument);
    EXPECT_THROW(ParseDefinitionJson("{\"definitions\":{\"tank\":{},\"tank\":{}}}"), std::invalid_argument);
    EXPECT_THROW(ParseDefinitionJson(text + "{}"), std::invalid_argument);
    EXPECT_THROW(ParseDefinitionJson("/* comment */" + text), std::invalid_argument);
    EXPECT_THROW(ParseDefinitionJson(std::string(1024 * 1024 + 1, ' ')), std::invalid_argument);
    EXPECT_THROW(ParseDefinitionJson(std::string(40, '[') + "0" + std::string(40, ']')), std::invalid_argument);
    EXPECT_THROW(ParseDefinitionJson("{\"schemaVersion\":1e1000}"), std::invalid_argument);
    auto j = BuiltInJson(); j["definitions"]["special"]["purpleAnglesDegrees"] = std::vector<int>(1025, 0);
    Reject(j, "1024");
}

TEST(GameplayDefinitionLoader, ExportLoadAndFailuresDoNotReplaceExistingFiles) {
    TemporaryDefinition file;
    EXPECT_THROW(LoadDefinitionFile(file.path), std::runtime_error);
    ExportBuiltInDefinitionFile(file.path);
    Equal(BuildBuiltInDefinitions(), LoadDefinitionFile(file.path));
    {
        std::ifstream input(file.path, std::ios::binary);
        const std::string bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        EXPECT_EQ(bytes, SerializeDefinitionJson(BuildBuiltInDefinitions()));
    }
    EXPECT_THROW(ExportBuiltInDefinitionFile(file.path), std::runtime_error);
    Equal(BuildBuiltInDefinitions(), LoadDefinitionFile(file.path));
    file.Write("Keep this authored configuration");
    EXPECT_THROW(ExportBuiltInDefinitionFile(file.path), std::runtime_error);
    {
        std::ifstream input(file.path, std::ios::binary);
        const std::string bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        EXPECT_EQ(bytes, "Keep this authored configuration");
    }
    file.Write("{}");
    try { LoadDefinitionFile(file.path); FAIL(); }
    catch (const std::exception& e) { EXPECT_NE(std::string(e.what()).find(file.path), std::string::npos); }
    file.Write(std::string(1024 * 1024 + 1, ' '));
    EXPECT_THROW(LoadDefinitionFile(file.path), std::invalid_argument);
}

TEST(GameplayDefinitionLoader, OptionsAreExplicitAndMutuallyExclusive) {
    EXPECT_EQ(Options({"tankgame"}).command, DefinitionCommand::Run);
    EXPECT_TRUE(Options({"tankgame"}).path.empty());
    EXPECT_EQ(Options({"tankgame", "--gameplay-config", "relative.json"}).path, "relative.json");
    EXPECT_EQ(Options({"tankgame", "--validate-gameplay-config", "a.json"}).command, DefinitionCommand::Validate);
    EXPECT_EQ(Options({"tankgame", "--export-gameplay-config", "a.json"}).command, DefinitionCommand::Export);
    EXPECT_EQ(Options({"tankgame", "--help"}).command, DefinitionCommand::Help);
    EXPECT_THROW(Options({"tankgame", "--gameplay-config"}), std::invalid_argument);
    EXPECT_THROW(Options({"tankgame", "--gameplay-config", "--help"}), std::invalid_argument);
    EXPECT_THROW(Options({"tankgame", "--gameplay-config", "a", "--export-gameplay-config", "b"}), std::invalid_argument);
    EXPECT_THROW(Options({"tankgame", "--unknown"}), std::invalid_argument);
}

class GameplayDefinitionLoaderTest : public ::testing::Test {
protected:
    GameWorld& World(GameTask& task) { return task.gameWorld; }
    void StartMode(GameTask& task, int mode) { task.menuState = mode; task.SetUpGame(); }
};

TEST_F(GameplayDefinitionLoaderTest, SelectedFileConstructsWorldAndChangesOnlyOnRestart) {
    TemporaryDefinition file;
    auto definitions = BuildBuiltInDefinitions();
    definitions.tank.maxHealth = 432;
    definitions.projectile.speed = 17;
    file.Write(SerializeDefinitionJson(definitions));
    const auto options = Options({"tankgame", "--gameplay-config", file.path});
    GameTask task(LoadStartupDefinitions(options));
    EXPECT_FLOAT_EQ(World(task).CreateTank()->GetStats().tank.maxHealth, 432);
    definitions.tank.maxHealth = 234;
    file.Write(SerializeDefinitionJson(definitions));
    EXPECT_FLOAT_EQ(World(task).CreateTank()->GetStats().tank.maxHealth, 432);
    GameTask next(LoadStartupDefinitions(options));
    EXPECT_FLOAT_EQ(World(next).CreateTank()->GetStats().tank.maxHealth, 234);
    file.Write("{}");
    std::string diagnostic;
    Equal(BuildBuiltInDefinitions(), LoadStartupDefinitions(options, &diagnostic));
    EXPECT_NE(diagnostic.find("using built-in defaults"), std::string::npos);
    EXPECT_FLOAT_EQ(World(task).CreateTank()->GetStats().tank.maxHealth, 432);
}

TEST_F(GameplayDefinitionLoaderTest, FileRulesSurviveFiringPickupRespawnAndModeReloads) {
    TemporaryDefinition file;
    auto definitions = BuildBuiltInDefinitions();
    definitions.tank.maxHealth = 240;
    definitions.projectile.speed = 17;
    definitions.player.otherPickupEnergyFraction = 0.25f;
    file.Write(SerializeDefinitionJson(definitions));
    App app;
    SoundTask sound;
    sound.disable = true;
    GameTask task(LoadStartupDefinitions(Options({"tankgame", "--gameplay-config", file.path})));
    app.gameTask = &task;
    app.soundTask = &sound;
    auto& world = World(task);
    auto* manager = task.GetPlayerManager();
    LevelHandler::Create();
    TankHandler::Create();
    LevelHandler::GetSingleton().SetGameWorld(&world);
    TankHandler::GetSingleton().SetGameWorld(&world);
    world.Initialize();
    manager->Initialize(&world);
    struct Cleanup {
        PlayerManager* manager;
        GameWorld* world;
        ~Cleanup() {
            Events::Clear();
            manager->ReleaseTanks();
            manager->Shutdown();
            world->Clear();
            world->Shutdown();
            TankHandler::Destroy();
            LevelHandler::Destroy();
        }
    } cleanup{manager, &world};
    manager->SetDifficulty(2);
    for (int mode : {0, 1, 2, 0}) {
        StartMode(task, mode);
        auto* player = manager->GetPlayer(0);
        auto* tank = player->GetControlledTank();
        ASSERT_NE(tank, nullptr);
        EXPECT_FLOAT_EQ(tank->GetStats().tank.maxHealth, 240);
        tank->fireTimer = 10;
        const auto before = world.GetBullets().size();
        tank->Fire(1);
        ASSERT_GT(world.GetBullets().size(), before);
        auto* shot = world.GetBullets().back().get();
        EXPECT_FLOAT_EQ(shot->GetMoveRate(), 17);
        tank->health = 75;
        manager->SetDifficulty(0);
        manager->SetDifficulty(0);
        EXPECT_FLOAT_EQ(tank->health, 75);
        EXPECT_FLOAT_EQ(tank->GetStats().tank.maxHealth, 720);
        tank->SetType(TankType::TYPE_RED, TankType::TYPE_GREY);
        tank->energy = 0;
        world.CreateItem(tank->x, tank->y + 0.1f, tank->z, TankType::TYPE_BLUE);
        player->CheckItemCollisions();
        EXPECT_EQ(tank->GetPrimaryType(), TankType::TYPE_BLUE);
        EXPECT_FLOAT_EQ(tank->energy, tank->GetStats().weapon.maxEnergy * 0.25f);
        EXPECT_FLOAT_EQ(shot->GetMoveRate(), 17);
        tank->alive = false;
        player->Respawn();
        tank = player->GetControlledTank();
        EXPECT_FLOAT_EQ(tank->health, 720);
        EXPECT_EQ(tank->GetPrimaryType(), TankType::TYPE_BLUE);
        manager->ReleaseTanks();
        world.Clear();
        ASSERT_TRUE(LevelHandler::GetSingleton().Load("levels/level1@@.txt"));
        manager->SpawnPlayerTanks();
        EXPECT_FLOAT_EQ(manager->GetPlayer(0)->GetControlledTank()->GetStats().tank.maxHealth, 720);
        manager->SetDifficulty(2);
    }
}

TEST(GameplayDefinitionLoader, StartupFailuresFallBackWithSourceAndReason) {
    TemporaryDefinition file;
    const auto options = Options({"tankgame", "--gameplay-config", file.path});
    for (const std::string& contents : {std::string(), std::string("{bad json"),
            std::string("{}"), std::string("{\"schemaVersion\":99,\"definitions\":{}}"),
            std::string(1024 * 1024 + 1, ' ')}) {
        if (!contents.empty()) file.Write(contents);
        std::string diagnostic;
        Equal(BuildBuiltInDefinitions(), LoadStartupDefinitions(options, &diagnostic));
        EXPECT_NE(diagnostic.find(file.path), std::string::npos);
        EXPECT_NE(diagnostic.find("Could not load"), std::string::npos);
        EXPECT_NE(diagnostic.find("using built-in defaults"), std::string::npos);
    }
    auto invalid = BuildBuiltInDefinitions();
    auto document = Json::parse(SerializeDefinitionJson(invalid));
    document["definitions"]["tank"]["maxHealth"] = -1;
    file.Write(document.dump());
    std::string diagnostic;
    Equal(BuildBuiltInDefinitions(), LoadStartupDefinitions(options, &diagnostic));
    EXPECT_NE(diagnostic.find("maxHealth"), std::string::npos);
    file.Write(SerializeDefinitionJson(BuildBuiltInDefinitions()));
    Equal(BuildBuiltInDefinitions(), LoadStartupDefinitions(options, &diagnostic));
    EXPECT_EQ(diagnostic, "Loaded " + file.path + " (schema version 1)");
}
