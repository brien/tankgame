#include "GameplayDefinitions.h"

namespace Gameplay {
GameplayDefinitions BuildBuiltInDefinitions()
{
    GameplayDefinitions definitions;

    definitions.input.mouseTurretScale = 2;
    definitions.input.keyboardTurretRate = 200;
    definitions.input.keyboardTurboTurretRate = 300;
    definitions.input.browserPadActivationDeadzone = 5000;
    definitions.joystick.forwardDeadzone = 5000;
    definitions.joystick.reverseDeadzone = 8000;
    definitions.joystick.bodyDeadzone = 8000;
    definitions.joystick.turretDeadzone = 6000;
    definitions.joystick.alternateTurretDeadzone = 5000;
    definitions.joystick.forwardDivisor = 20000;
    definitions.joystick.bodyDivisor = 32000;
    definitions.joystick.turretDivisor = 3200;
    definitions.joystick.chargeDivisor = 3200;
    definitions.joystick.specialChargeDivisor = 3200;
    definitions.joystick.fireAxisThreshold = -5000;
    definitions.joystick.jumpAxisThreshold = 5000;
    definitions.special.muzzleHeight = .25f;
    definitions.special.blueCenterHeight = .5f;
    definitions.special.blueSideOffset = .2f;
    definitions.special.redArcCount = 10;
    definitions.special.redArcStepDegrees = 27;
    definitions.special.redArcStartDegrees = 52;
    definitions.special.redArcBounces = 0;
    definitions.special.yellowBounces = 4;
    definitions.landing.rewardEnergyFraction = 1.0f / 3.0f;
    definitions.landing.rewardThresholdFraction = 2.0f / 3.0f;
    definitions.enemyBehavior.activationDistance = 15;
    definitions.enemyBehavior.activationDistancePerLevel = 3;
    definitions.enemyBehavior.huntEnergyFraction = .5f;
    definitions.enemyBehavior.fearUntilLevelOffset = 2;
    definitions.enemyBehavior.attackerCountBase = 1;
    definitions.enemyBehavior.wanderProbeAngleDegrees = 20;
    definitions.enemyBehavior.wanderProbeFrames = 15;
    definitions.enemyBehavior.wanderJumpHeight = 3;
    definitions.enemyBehavior.wanderMaxDrop = 7;
    definitions.enemyBehavior.wanderTurnDegrees = 90;
    definitions.enemyBehavior.jumpEnergyFraction = .25f;
    definitions.enemyBehavior.aimToleranceDegrees = 5;
    definitions.enemyBehavior.fearProbeAngleDegrees = 30;
    definitions.enemyBehavior.fearProbeFrames = 20;
    definitions.enemyBehavior.fearJumpHeight = 3;
    definitions.enemyBehavior.avoidanceTurnDegrees = 30;
    definitions.enemyBehavior.huntProbeFrames = 10;
    definitions.enemyBehavior.huntJumpHeight = 5;
    definitions.enemyBehavior.huntStopDistance = 10;
    definitions.enemyBehavior.fireEnergyFraction = .5f;
    definitions.enemyBehavior.shotCharge = 1;
    definitions.enemyBehavior.spawnRotationStepDegrees = 45;
    definitions.enemyBehavior.overflowSpawnOffset = 1;
    definitions.gameCube = definitions.joystick;
    definitions.gameCube.forwardDeadzone = 4000;
    definitions.gameCube.forwardDivisor = 18000;
    definitions.gameCube.bodyDivisor = 28000;
    definitions.gameCube.turretDivisor = 2500;
    definitions.gameCube.chargeDivisor = 6400;
    definitions.gameCube.specialChargeDivisor = 6000;
    definitions.special.redForwardAnglesDegrees = {0, -10, 20};
    definitions.special.purpleAnglesDegrees = {0, -90, 180, 90};
    definitions.enemy.openingTypes = {
        {TankType::TYPE_RED, TankType::TYPE_GREY},
        {TankType::TYPE_YELLOW, TankType::TYPE_GREY},
        {TankType::TYPE_YELLOW, TankType::TYPE_GREY}};
    for (int index = 0; index < 3; ++index)
        definitions.enemy.regularTypes.push_back({static_cast<TankType>(1 + index % 3), static_cast<TankType>(index % 3)});
    for (int index = 0; index < 12; ++index)
        definitions.enemy.advancedTypes.push_back({static_cast<TankType>(1 + index % 4), static_cast<TankType>(1 + index % 3)});
    definitions.enemy.advancedLevelOffsets = {2, 5, 12};

    definitions.tank.maxHealth = 100;
    definitions.tank.healthRegenPerSecond = 10;
    definitions.tank.moveSpeed = 5;
    definitions.tank.rotationDegreesPerSecond = 150;
    definitions.tank.collisionRadius = .2f;
    definitions.tank.jumpEnergyPerSecond = 250;
    definitions.tank.jumpLaunchSpeed = 4;
    definitions.tank.jumpThrust = 18;
    definitions.tank.gravity = 16;
    definitions.tank.maxRiseSpeed = 10;
    definitions.tank.maxFallSpeed = 15;
    definitions.tank.turboEnergyPerSecond = 250;
    definitions.tank.turretInputScale = 10;
    definitions.tank.turretMaxStepDegrees = 2.5f;
    definitions.tank.maxForwardInput = 1.25f;

    definitions.projectile.speed = 33;
    definitions.projectile.lifetimeSeconds = 100;
    definitions.projectile.collisionRadius = .1f;
    definitions.projectile.registrationRadius = .3f;
    definitions.projectile.selfHitDelaySeconds = .5f;
    definitions.projectile.purplePrimaryTurnScale = 320;
    definitions.projectile.purpleSecondaryTurnScale = 160;
    definitions.projectile.purpleSecondaryPowerScale = .5f;
    definitions.projectile.purpleSecondaryChargeDivisor = 2;
    definitions.projectile.purpleMinPower = 100;
    definitions.projectile.purpleMaxPower = 1000;
    definitions.projectile.purpleSpecialTurnAcceleration = 1000;
    definitions.projectile.firstBouncePowerMultiplier = 1.5f;
    definitions.projectile.yellowFirstBouncePowerMultiplier = 2;
    definitions.projectile.yellowRedChildCount = 20;
    definitions.projectile.yellowBlueChildCount = 2;
    definitions.projectile.yellowBlueChildSpacing = .2f;
    definitions.projectile.childBounces = 4;
    definitions.projectile.purpleChildBounces = 16;
    definitions.projectile.purpleChildPowerMultiplier = 2;
    definitions.projectile.purpleChildTurnSpeed = 10;

    definitions.player.startingPrimaryType = TankType::TYPE_RED;
    definitions.player.startingSecondaryType = TankType::TYPE_BLUE;
    definitions.player.spawnHeight = 24;
    definitions.player.respawnDelaySeconds = .5f;
    definitions.player.versusRespawnDelaySeconds = 1.5f;
    definitions.player.comboDecayPerSecond = 1;
    definitions.player.comboChargeFraction = .1f;
    definitions.player.specialChargeMax = 100;
    definitions.player.matchingPickupEnergyFraction = 1;
    definitions.player.otherPickupEnergyFraction = .5f;
    definitions.player.pickupScore = 100;

    definitions.combat.healthReturnDivisor = 2;
    definitions.combat.energyReturnDivisor = 8;
    definitions.combat.friendlyResourceThresholdMultiplier = 2;
    definitions.combat.blueSpecialHitPowerBonus = 100;
    definitions.combat.hitComboDivisor = 10;
    definitions.combat.streakThreshold = 9;
    definitions.combat.streakComboDivisor = 2;
    definitions.combat.distanceThreshold = 20;
    definitions.combat.distanceComboDivisor = 10;
    definitions.combat.bounceBonusMaxAgeSeconds = 10;
    definitions.combat.bounceBonusBase = 1;
    definitions.combat.bounceAgeDivisor = 2;
    definitions.combat.killBonus = 10;
    definitions.combat.killComboDivisor = 10;

    definitions.enemy.openingLevelWeakenedIndex = 4;
    definitions.enemy.openingLevelAttackScale = 1.0f / 3.0f;

    definitions.difficulties[0] = {3, 2, 1.5f, 1.5f};
    definitions.difficulties[1] = {1.5f, 1.5f, 1.5f, 1.5f};
    definitions.difficulties[2] = {1, 1, 1, 1};

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
    // Built-in fallback for standalone entities and fixtures.
    // Construct once on first use; no mutable maps or translation-unit initializer.
    static const GameplayDefinitions definitions = BuildBuiltInDefinitions();
    return definitions;
}
}
