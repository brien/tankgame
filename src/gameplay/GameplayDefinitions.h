#pragma once

#include "TankType.h"
#include <map>
#include <array>
#include <vector>

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

struct TankDefaults {
    float maxHealth = 0;
    float healthRegenPerSecond = 0;
    float moveSpeed = 0;
    float rotationDegreesPerSecond = 0;
    float collisionRadius = 0;
    float jumpEnergyPerSecond = 0;
    float jumpLaunchSpeed = 0;
    float jumpThrust = 0;
    float gravity = 0;
    float maxRiseSpeed = 0;
    float maxFallSpeed = 0;
    float turboEnergyPerSecond = 0;
    float turretInputScale = 0;
    float turretMaxStepDegrees = 0;
    float maxForwardInput = 0;
};

struct ProjectileRules {
    float speed = 0;
    float lifetimeSeconds = 0;
    float collisionRadius = 0;
    float registrationRadius = 0; // Preserve the broader registered collision shape.
    float selfHitDelaySeconds = 0;
    float purplePrimaryTurnScale = 0;
    float purpleSecondaryTurnScale = 0;
    float purpleSecondaryPowerScale = 0;
    float purpleSecondaryChargeDivisor = 0;
    float purpleMinPower = 0;
    float purpleMaxPower = 0;
    float purpleSpecialTurnAcceleration = 0;
    float firstBouncePowerMultiplier = 0;
    float yellowFirstBouncePowerMultiplier = 0;
    int yellowRedChildCount = 0;
    int yellowBlueChildCount = 0;
    float yellowBlueChildSpacing = 0;
    int childBounces = 0;
    int purpleChildBounces = 0;
    float purpleChildPowerMultiplier = 0;
    float purpleChildTurnSpeed = 0;
};

struct PlayerRules {
    TankType startingPrimaryType = TankType::TYPE_GREY;
    TankType startingSecondaryType = TankType::TYPE_GREY;
    float spawnHeight = 0;
    float respawnDelaySeconds = 0;
    float versusRespawnDelaySeconds = 0;
    float comboDecayPerSecond = 0;
    float comboChargeFraction = 0;
    float specialChargeMax = 0;
    float matchingPickupEnergyFraction = 0;
    float otherPickupEnergyFraction = 0;
    int pickupScore = 0;
};

struct CombatRules {
    float healthReturnDivisor = 0;
    float energyReturnDivisor = 0;
    float friendlyResourceThresholdMultiplier = 0;
    float blueSpecialHitPowerBonus = 0;
    float hitComboDivisor = 0;
    int streakThreshold = 0;
    float streakComboDivisor = 0;
    float distanceThreshold = 0;
    float distanceComboDivisor = 0;
    float bounceBonusMaxAgeSeconds = 0;
    float bounceBonusBase = 0;
    float bounceAgeDivisor = 0;
    float killBonus = 0;
    float killComboDivisor = 0;
};

struct InputRules {
    float mouseTurretScale = 0;
    float keyboardTurretRate = 0;
    float keyboardTurboTurretRate = 0;
    int browserPadActivationDeadzone = 0;
};

struct JoystickRules {
    int forwardDeadzone = 0;
    int reverseDeadzone = 0;
    int bodyDeadzone = 0;
    int turretDeadzone = 0;
    int alternateTurretDeadzone = 0;
    float forwardDivisor = 0;
    float bodyDivisor = 0;
    float turretDivisor = 0;
    float chargeDivisor = 0;
    float specialChargeDivisor = 0;
    int fireAxisThreshold = 0;
    int jumpAxisThreshold = 0;
};

struct SpecialRules {
    float muzzleHeight = 0;
    float blueCenterHeight = 0;
    float blueSideOffset = 0;
    int redArcCount = 0;
    float redArcStepDegrees = 0;
    float redArcStartDegrees = 0;
    int redArcBounces = 0;
    int yellowBounces = 0;
    std::vector<float> redForwardAnglesDegrees;
    std::vector<float> purpleAnglesDegrees;
};

struct LandingRules {
    float rewardEnergyFraction = 0;
    float rewardThresholdFraction = 0;
};

struct EnemyBehaviorRules {
    float activationDistance = 0;
    float activationDistancePerLevel = 0;
    float huntEnergyFraction = 0;
    int fearUntilLevelOffset = 0;
    int attackerCountBase = 0;
    float wanderProbeAngleDegrees = 0;
    float wanderProbeFrames = 0;
    float wanderJumpHeight = 0;
    float wanderMaxDrop = 0;
    float wanderTurnDegrees = 0;
    float jumpEnergyFraction = 0;
    float aimToleranceDegrees = 0;
    float fearProbeAngleDegrees = 0;
    float fearProbeFrames = 0;
    float fearJumpHeight = 0;
    float avoidanceTurnDegrees = 0;
    float huntProbeFrames = 0;
    float huntJumpHeight = 0;
    float huntStopDistance = 0;
    float fireEnergyFraction = 0;
    float shotCharge = 0;
    float spawnRotationStepDegrees = 0;
    float overflowSpawnOffset = 0;
};

struct EnemyRules {
    int openingLevelWeakenedIndex = 0;
    float openingLevelAttackScale = 0;
    std::vector<std::pair<TankType, TankType>> openingTypes;
    std::vector<std::pair<TankType, TankType>> regularTypes;
    std::vector<std::pair<TankType, TankType>> advancedTypes;
    std::vector<int> advancedLevelOffsets;
    TankType openingWeakenedPrimary = TankType::TYPE_BLUE;
    TankType openingWeakenedSecondary = TankType::TYPE_GREY;
};

struct DifficultyModifiers {
    float health = 1;
    float healthRegen = 1;
    float energy = 1;
    float energyRegen = 1;
};

struct TankContext {
    bool enemy = false;
    int difficulty = 2; // Easy=0, normal=1, hard=2; unmodified standalone tanks.
    float attackScale = 1;
};

struct ResolvedTankConfig {
    TankTypeConfig weapon;
    TankDefaults tank;
};

struct GameplayDefinitions {
    TankDefaults tank;
    ProjectileRules projectile;
    PlayerRules player;
    CombatRules combat;
    EnemyRules enemy;
    InputRules input;
    JoystickRules joystick;
    SpecialRules special;
    LandingRules landing;
    EnemyBehaviorRules enemyBehavior;
    JoystickRules gameCube;
    std::array<DifficultyModifiers, 3> difficulties;
    std::map<TankType, TankTypeConfig> types;
    std::map<std::pair<TankType, TankType>, TankTypeOverride> combinations;
};

// Pure functions: no application singleton, entity mutation, or file access.
TankTypeConfig ResolveTankType(const GameplayDefinitions& definitions,
                               TankType primary, TankType secondary);
ResolvedTankConfig ResolveTank(const GameplayDefinitions& definitions,
                              TankType primary, TankType secondary, const TankContext& context);
// Throws std::invalid_argument with the offending field on invalid definitions.
void ValidateDefinitions(const GameplayDefinitions& definitions);
GameplayDefinitions BuildBuiltInDefinitions();
const GameplayDefinitions& BuiltInDefinitions();

}
