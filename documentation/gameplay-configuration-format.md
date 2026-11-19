# External gameplay configuration format

Version 1 stores a complete immutable gameplay snapshot. An editable starting
snapshot is included at [`runtime/gameplay.json`](../runtime/gameplay.json).
Edit it or copy it, validate it, then select it at startup. You can also export
a fresh snapshot from the built-ins:

```sh
./runtime/tankgame-linux --export-gameplay-config /tmp/gameplay.json
./runtime/tankgame-linux --validate-gameplay-config /tmp/gameplay.json
# Launch from runtime so the game's existing assets resolve correctly.
cd runtime
./tankgame-linux --gameplay-config /tmp/gameplay.json
```

Use `tankgame-mac` on macOS. All relative configuration paths resolve against the
launch working directory. Startup loads `gameplay.json` by default;
`--gameplay-config <path>` selects an alternative. Selection is not persisted.
Exports refuse existing destinations. Validation and export run without SDL or a
window. Exit status is 0 on success and 1 on argument, I/O, validation or startup
failure. During normal startup, any configuration loading failure is logged with
the path/reason and falls back to built-ins, including explicitly selected files.
The validation command still returns failure for invalid or unreadable files.

The root has exactly `schemaVersion` (integer 1) and `definitions`. The
[JSON Schema](gameplay-configuration.schema.json) provides every field and basic
bound for editor assistance. The shipped snapshot was generated from the built-in
provider using the export command. Built-ins remain the fallback when loading
the default or selected file fails. Maintainers
should refresh the shipped snapshot via export when built-in tuning changes.

## File rules

- All sections and complete-record fields are required. Unknown keys, duplicate
  object keys, null values, comments and trailing documents are rejected.
- Type names are `grey`, `red`, `blue`, `yellow`, `purple`. Difficulties are keyed
  by `easy`, `normal`, `hard`; numeric enum values are never accepted.
- `types` contains all five type records. `difficulties` contains all three records.
- `combinations` is a required array of `{primary, secondary, overrides}` records.
  It may be empty. Pairs are ordered and unique. Missing pairs inherit the primary
  type. Within `overrides`, only `fireIntervalSeconds`, `attack`, `fireCost`,
  `specialChargeCost` and `bounces` are permitted. Each is optional: omission
  inherits, whereas explicit zero overrides. `{}` is a valid override object.
- Enemy type cycles contain ordered `{primary, secondary}` records. Repeated
  entries are meaningful and preserved, unlike duplicate combination definitions.
- Integers require integer tokens (`1`, not `1.0`), within signed 32-bit range and
  the field bounds. Floats accept integer/fractional tokens, require finite float
  range, and reject nonzero values that underflow to zero. Decimal values undergo
  normal float rounding. Numeric strings and booleans are not numbers.
- Documents are limited to 1 MiB and parser depth 32. Arrays are limited to 1,024
  entries; combinations to 25. Limits apply before world construction.
- The C++ decoder and `ValidateDefinitions()` are authoritative: JSON Schema alone
  cannot enforce duplicate keys, integer token spelling, unique ordered pairs,
  float representability, min/max relationships or composed-stat overflow.

Loading resolves no entities and changes no active world. The application reads
once before constructing `GameTask`/`GameWorld`. Restart to apply edits. Existing
level metadata remains a separate format. Input mappings and behavior algorithms
remain C++; files supply only the fields described here.

## Browser deployment

Configure the existing Emscripten build with an optional absolute file path:

```sh
cmake -S . -B build-browser -DTANKGAME_GAMEPLAY_CONFIG=/absolute/path/gameplay.json
cmake --build build-browser -j2
```

By default the browser build packages `runtime/gameplay.json`. The selected
snapshot is staged and preloaded at `/gameplay.json`. Changes
to the source file trigger restaging and relinking. Clear the cache option with
`-DTANKGAME_GAMEPLAY_CONFIG=` to restore the shipped default file. Validate with
the native command before deployment. A missing explicitly selected source file
fails build configuration; a missing default file is omitted from the package.
Missing or invalid files at runtime log the reason and use built-in defaults. This feature
performs no runtime fetching, local upload or persisted browser selection.

## Version policy

Adding required fields or changing field meaning requires a new schema version.
Version 1 is the only supported external version. Unknown versions fail with the
supported version in the diagnostic. Future versions must explicitly decode and
migrate any supported older format into the current model, then validate it. No
version 0, implicit default merging or speculative migration is supported.

## Field reference

Fields use the names below within their corresponding section. The schema lists
numeric bounds. The [configuration contract](gameplay-configuration-contract.md)
describes resolution order, lifecycle rules and preserved behavior. `TankTypeConfig`
is used under `types`; `DifficultyModifiers` under `difficulties`; `JoystickRules`
is used by both `joystick` and `gameCube`.

### TankDefaults

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `maxHealth` | `float` | Health units |
| `healthRegenPerSecond` | `float` | Health units per second |
| `moveSpeed` | `float` | World units per second |
| `rotationDegreesPerSecond` | `float` | Degrees per second |
| `collisionRadius` | `float` | World units |
| `jumpEnergyPerSecond` | `float` | Energy units per second |
| `jumpLaunchSpeed` | `float` | World units per second |
| `jumpThrust` | `float` | World units per second squared |
| `gravity` | `float` | World units per second squared |
| `maxRiseSpeed` | `float` | World units per second |
| `maxFallSpeed` | `float` | World units per second |
| `turboEnergyPerSecond` | `float` | Energy units per second |
| `turretInputScale` | `float` | Dimensionless multiplier or fraction |
| `turretMaxStepDegrees` | `float` | Degrees per input call |
| `maxForwardInput` | `float` | Dimensionless forward-input cap |

### ProjectileRules

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `speed` | `float` | World units per second |
| `lifetimeSeconds` | `float` | Seconds |
| `collisionRadius` | `float` | World units |
| `registrationRadius` | `float` | World units |
| `selfHitDelaySeconds` | `float` | Seconds |
| `purplePrimaryTurnScale` | `float` | Degrees per second per second of shot charge |
| `purpleSecondaryTurnScale` | `float` | Degrees per second per second of shot charge |
| `purpleSecondaryPowerScale` | `float` | Dimensionless multiplier or fraction |
| `purpleSecondaryChargeDivisor` | `float` | Charge duration divisor in seconds |
| `purpleMinPower` | `float` | Projectile input power |
| `purpleMaxPower` | `float` | Projectile input power |
| `purpleSpecialTurnAcceleration` | `float` | Degrees per second squared |
| `firstBouncePowerMultiplier` | `float` | Dimensionless multiplier or fraction |
| `yellowFirstBouncePowerMultiplier` | `float` | Dimensionless multiplier or fraction |
| `yellowRedChildCount` | `int` | Integral count or level offset |
| `yellowBlueChildCount` | `int` | Integral count or level offset |
| `yellowBlueChildSpacing` | `float` | World units |
| `childBounces` | `int` | Integral count or level offset |
| `purpleChildBounces` | `int` | Integral count or level offset |
| `purpleChildPowerMultiplier` | `float` | Dimensionless multiplier or fraction |
| `purpleChildTurnSpeed` | `float` | Degrees per second |

### PlayerRules

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `startingPrimaryType` | `TankType` | Stable tank type name or ordered type pairs |
| `startingSecondaryType` | `TankType` | Stable tank type name or ordered type pairs |
| `spawnHeight` | `float` | World units |
| `respawnDelaySeconds` | `float` | Seconds |
| `versusRespawnDelaySeconds` | `float` | Seconds |
| `comboDecayPerSecond` | `float` | Combo units per second |
| `comboChargeFraction` | `float` | Dimensionless multiplier or fraction |
| `specialChargeMax` | `float` | Special charge units |
| `matchingPickupEnergyFraction` | `float` | Dimensionless multiplier or fraction |
| `otherPickupEnergyFraction` | `float` | Dimensionless multiplier or fraction |
| `pickupScore` | `int` | Score points |

### CombatRules

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `healthReturnDivisor` | `float` | Damage-to-health divisor |
| `energyReturnDivisor` | `float` | Damage-to-energy divisor |
| `friendlyResourceThresholdMultiplier` | `float` | Dimensionless multiplier or fraction |
| `blueSpecialHitPowerBonus` | `float` | Projectile input power |
| `hitComboDivisor` | `float` | Hit-count divisor for combo contribution |
| `streakThreshold` | `int` | Integral count or level offset |
| `streakComboDivisor` | `float` | Streak-count divisor for combo contribution |
| `distanceThreshold` | `float` | World units |
| `distanceComboDivisor` | `float` | World units per combo unit |
| `bounceBonusMaxAgeSeconds` | `float` | Seconds |
| `bounceBonusBase` | `float` | Combo units |
| `bounceAgeDivisor` | `float` | Seconds per combo unit |
| `killBonus` | `float` | Combo units before diminishing returns |
| `killComboDivisor` | `float` | Combo units in diminishing-return divisor |

### EnemyRules

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `openingLevelWeakenedIndex` | `int` | Integral count or level offset |
| `openingLevelAttackScale` | `float` | Dimensionless multiplier or fraction |
| `openingTypes` | `std::vector<std::pair<TankType, TankType>>` | Stable tank type name or ordered type pairs |
| `regularTypes` | `std::vector<std::pair<TankType, TankType>>` | Stable tank type name or ordered type pairs |
| `advancedTypes` | `std::vector<std::pair<TankType, TankType>>` | Stable tank type name or ordered type pairs |
| `advancedLevelOffsets` | `std::vector<int>` | Integral count or level offset |
| `openingWeakenedPrimary` | `TankType` | Stable tank type name or ordered type pairs |
| `openingWeakenedSecondary` | `TankType` | Stable tank type name or ordered type pairs |

### InputRules

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `mouseTurretScale` | `float` | Turret input per mouse delta unit |
| `keyboardTurretRate` | `float` | Turret input units per second |
| `keyboardTurboTurretRate` | `float` | Turret input units per second |
| `browserPadActivationDeadzone` | `int` | Raw SDL axis units |

### JoystickRules

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `forwardDeadzone` | `int` | Raw SDL axis units |
| `reverseDeadzone` | `int` | Raw SDL axis units |
| `bodyDeadzone` | `int` | Raw SDL axis units |
| `turretDeadzone` | `int` | Raw SDL axis units |
| `alternateTurretDeadzone` | `int` | Raw SDL axis units |
| `forwardDivisor` | `float` | Raw SDL axis normalization divisor |
| `bodyDivisor` | `float` | Raw SDL axis normalization divisor |
| `turretDivisor` | `float` | Raw SDL axis normalization divisor |
| `chargeDivisor` | `float` | Raw SDL axis normalization divisor |
| `specialChargeDivisor` | `float` | Raw SDL axis normalization divisor |
| `fireAxisThreshold` | `int` | Raw SDL axis units |
| `jumpAxisThreshold` | `int` | Raw SDL axis units |

### SpecialRules

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `muzzleHeight` | `float` | World units |
| `blueCenterHeight` | `float` | World units |
| `blueSideOffset` | `float` | World units |
| `redArcCount` | `int` | Integral count or level offset |
| `redArcStepDegrees` | `float` | Degrees |
| `redArcStartDegrees` | `float` | Degrees |
| `redArcBounces` | `int` | Integral count or level offset |
| `yellowBounces` | `int` | Integral count or level offset |
| `redForwardAnglesDegrees` | `std::vector<float>` | Degrees |
| `purpleAnglesDegrees` | `std::vector<float>` | Degrees |

### LandingRules

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `rewardEnergyFraction` | `float` | Dimensionless multiplier or fraction |
| `rewardThresholdFraction` | `float` | Dimensionless multiplier or fraction |

### EnemyBehaviorRules

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `activationDistance` | `float` | World units |
| `activationDistancePerLevel` | `float` | World units per level |
| `huntEnergyFraction` | `float` | Dimensionless multiplier or fraction |
| `fearUntilLevelOffset` | `int` | Integral count or level offset |
| `attackerCountBase` | `int` | Integral count or level offset |
| `wanderProbeAngleDegrees` | `float` | Degrees |
| `wanderProbeFrames` | `float` | Frame delta multiples |
| `wanderJumpHeight` | `float` | World units |
| `wanderMaxDrop` | `float` | World units |
| `wanderTurnDegrees` | `float` | Degrees |
| `jumpEnergyFraction` | `float` | Dimensionless multiplier or fraction |
| `aimToleranceDegrees` | `float` | Degrees |
| `fearProbeAngleDegrees` | `float` | Degrees |
| `fearProbeFrames` | `float` | Frame delta multiples |
| `fearJumpHeight` | `float` | World units |
| `avoidanceTurnDegrees` | `float` | Degrees |
| `huntProbeFrames` | `float` | Frame delta multiples |
| `huntJumpHeight` | `float` | World units |
| `huntStopDistance` | `float` | World units |
| `fireEnergyFraction` | `float` | Dimensionless multiplier or fraction |
| `shotCharge` | `float` | Charge duration in seconds |
| `spawnRotationStepDegrees` | `float` | Degrees |
| `overflowSpawnOffset` | `float` | World units |

### TankTypeConfig

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `fireIntervalSeconds` | `float` | Seconds |
| `attack` | `float` | Projectile input power |
| `maxEnergy` | `float` | Energy units |
| `energyRegen` | `float` | Energy units per second |
| `moveCost` | `float` | Energy units per second at unit movement input |
| `fireCost` | `float` | Energy units |
| `specialChargeCost` | `float` | Special charge units |
| `bounces` | `int` | Integral count or level offset |
| `r` | `float` | RGB channel [0,1] |
| `g` | `float` | RGB channel [0,1] |
| `b` | `float` | RGB channel [0,1] |

### DifficultyModifiers

| Field | C++ value type | Units / meaning |
| --- | --- | --- |
| `health` | `float` | Dimensionless multiplier or fraction |
| `healthRegen` | `float` | Dimensionless multiplier or fraction |
| `energy` | `float` | Dimensionless multiplier or fraction |
| `energyRegen` | `float` | Dimensionless energy regeneration multiplier |

