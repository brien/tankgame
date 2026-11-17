#pragma once

#include "TankType.h"
#include <map>
#include <array>

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

struct EnemyRules {
    int openingLevelWeakenedIndex = 0;
    float openingLevelAttackScale = 0;
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
