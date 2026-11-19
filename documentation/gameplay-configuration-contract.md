# Gameplay configuration contract

Status: final implementation, 2026-11-18. Validation results are recorded in
[the progress report](gameplay-configuration-progress.md).

## Definition provider and ownership

`Gameplay::BuildBuiltInDefinitions()` is the authoritative compiled tuning
provider. `GameWorld(GameplayDefinitions)` validates a complete candidate before
copying/moving it into shared immutable ownership. A rejected candidate never
changes an existing world. Tanks and projectiles retain this ownership across
moves and projectile copies; an old projectile keeps its original rules even
if its firing tank changes types. Standalone entities use `BuiltInDefinitions()`
when no definitions are supplied. Callers supplying shared definitions directly
to standalone entities must validate them first and retain immutable ownership;
normal game creation goes through the validating world boundary.

The external JSON loader builds a complete `GameplayDefinitions`, validates
it, and passes it to world construction. See the
[version 1 file format](gameplay-configuration-format.md) and
[implementation progress](gameplay-external-configuration-progress.md). Settings
persistence and live reload remain separate work. Startup loads `gameplay.json`
from the working directory by default; `--gameplay-config` selects another file.
Loading failures log the path/reason and use built-ins. Browser builds package
`runtime/gameplay.json` by default at `/gameplay.json`.
No JSON/parser types appear in entities, resolution or combat APIs. Existing
level metadata JSON remains an independent, validated format.

## Resolution and state application

Resolution order is common defaults → primary type → ordered combination
(overrides only fields marked present) → role/encounter → player difficulty.
An explicit zero override is different from omission. Enemies receive free
jumping and no player difficulty multipliers. Attack scaling precedes the
historical conversion to integer projectile-input power at application; e.g.
the opening blue/grey enemy's 100/3 resolves to effective input power 33.

`Tank::GetStats()` exposes a const reference to the single effective-stat object.
Types are read through `GetPrimaryType()`/`GetSecondaryType()` and changed together
through `SetType()`. `ConfigureStats()` and `SetType()` resolve before changing
live state. Writable stat aliases and `TankTypeManager` have been removed.
Health, energy, elapsed cooldown, position, velocity and alive/dead state remain
mutable entity state. Debug resource cheats change that state, not definitions.

| Event | Resources and transient state | Regression evidence |
| --- | --- | --- |
| Constructor / `Init()` | Same resolved defaults; refill health/energy; reset cooldown and motion | `InitUsesSameDefaultsAndResetsTransientPhysics` and jump regressions |
| Player fresh spawn / respawn | Session starting types on first spawn; remember both types thereafter; apply current difficulty; full resolved resources | `EveryPairAndDifficultySurvivesRetuningAndFreshRespawn`, player-manager/game-task tests |
| Enemy spawn | Select encounter types and role before `Init()` refills; apply position afterward | `AlternateStartingTypesAndEnemyCapacityApplyBeforeRefill`, `EnemyCyclesAndEncounterModifierUseSessionRules` |
| Pickup | Resolve new ordered pair with current difficulty; preserve health/motion/cooldown, clamp resources, then reward based on previous primary type | Existing pickup/lifecycle and one-pickup-per-frame tests |
| Difficulty change / repeated application | Recompute from original definitions; preserve absolute resources and clamp; no healing, resurrection or accumulating multipliers | Lifecycle tests, all 75 pair/difficulty combinations |
| Level/mode transition | Existing deferred session lifecycle; each new entity follows configured spawn path | Game-task tests across all nine mode transitions and remembered-type level reloads |
| Invalid type/context | No partial type, context or stat mutation | `EffectiveStatsAreReadOnlyAndTypesRejectPartialChanges` |
| Tank type change after firing | In-flight power, types and rules remain stable | `SpecialLayoutsAndNormalMuzzleUseSessionDefinitions` and projectile-copy tests |

Readiness retains elapsed time and uses the new interval with the existing strict
`elapsed > interval` condition. Normal shots consume energy; special attacks
consume the player's separate charge meter. Built-in special cost remains 2.

## Tuning inventory and units

All values below live in `GameplayDefinitions.h`, with authoritative values in
`BuiltInGameplayDefinitions.cpp`. `ValidateDefinitions()` rejects invalid types,
non-finite numbers, invalid ranges, empty required cycles, invalid divisors and
composed stat overflow before world construction.

| Group | Meaning / units |
| --- | --- |
| `tank` | Health capacity and regeneration per second; movement units/s, rotation degrees/s; collision radius in world units; jump/turbo energy per second, launch/rise/fall speeds in units/s, thrust/gravity in units/s²; turret input multiplier, per-call degree limit and forward-input cap |
| `types`, `combinations` | Projectile-input attack power; seconds between shots; energy capacity, regeneration/s and shot cost; movement energy/s at unit input; separate special charge/use; integral ricochet budget; RGB channels [0,1] |
| `difficulties` | Dimensionless health, health-regeneration, energy and energy-regeneration multipliers; indices easy=0, normal=1, hard=2 |
| `projectile` | Movement units/s; lifetime and self-hit delay in seconds; world-space radii; purple charge/power and angular rules; bounce power multipliers, child counts, spacing, bounce budgets and turn speed |
| `player` | Starting ordered types; spawn height in world units; respawn delays in seconds; combo decay/s and combo-to-charge fraction; charge capacity, pickup score and capacity-based energy reward fractions |
| `combat` | Damage/healing/energy divisors; resource thresholds as capacity multipliers; streak/distance/bounce/kill score formulas; age thresholds in seconds and distances in world units |
| `input` | Mouse-delta multiplier; keyboard turret input/s before tank turret scaling; browser pad activation threshold in raw SDL axis units |
| `joystick`, `gameCube` | Raw SDL axis deadzones (0–32767), signed trigger thresholds (-32768–32767), positive raw-axis divisors for body/movement/turret/charged shots; unused profile fields retain valid defaults |
| `special` | World-space muzzle heights/lateral offsets; red arc count, start/step degrees and bounce budget; signed heading arrays for red forward and purple shots; yellow bounce budget |
| `landing` | Energy reward and strict reward threshold as fractions of resolved capacity; reward only on transition to grounded |
| `enemy` | Ordered type cycles for opening/regular/advanced encounters; advanced ASCII-derived level offsets; opening weakened enemy index, types and attack multiplier |
| `enemyBehavior` | Activation/stop/probe heights/distances in world units; per-level activation growth; hunt/fire/jump capacity fractions; fear cutoff and attacker base count; probe lookahead in frame multiples of `dT`; steering angles in degrees; spawn rotation and overflow spacing |

Special-angle arrays, red arc count, enemy type cycles and projectile child counts
are bounded at 1,024 entries/shots per configured component. This bounds individual
pattern work; it is not a global live-projectile cap or a guarantee that all valid
extreme configurations are performant. Normal and special muzzle advance both use
the configured projectile speed, rather than a duplicate built-in speed constant.

## Preserved behavior and intentionally excluded literals

The audit covered Tank, TankHandler, Player, PlayerManager, Bullet, CombatSystem,
all three input handlers and their rendering-data consumers. Structural rules
remain C++, not a generic behavior interpreter:

- Red specials retain a ten-shot arc at 52 + 27×index degrees and forward shots
  at 0/-10/+20. Purple retains 0/-90/+180/+90. Blue/red omits the central shot;
  yellow/non-yellow retains its two overlapping shots. Grey has no special
  projectile pattern. Red arc colors remain an intentional visual style.
- Axis/button bindings, device detection, camera presets, turbo's repeated-action
  and consumption order, and turret's per-call clamp are unchanged. Camera
  sensitivity is presentation, outside gameplay tuning.
- AI nearest-player selection, shared state variable, attacker accounting, integer
  height comparisons and existing local/member distance usage are preserved.
  Numeric thresholds are configurable; this is not an AI correctness redesign.
- The ASCII level progression (including the 9→A gap), map dimensions (128),
  fallback world positions, world-height limits, exit/death tile sentinels,
  authored spawn-slot count and collision sampling/separation tolerances remain
  level-format/geometry algorithms. They are not advertised as configurable.
- Angle conversions, quarter/half turns used to compute perpendicular/opposite
  directions, collision subdivision, array indices and entity/player identities
  are structural constants. UI/FX colors, offsets, animation timing, audio
  positioning/volume, diagnostic cadence and debug-cheat quantities remain
  presentation/debug constants.
- Health/energy overcharge, enemy free-jump eligibility, explicit uncapped
  `SetSpecialCharge()` and existing kill-combo charge behavior are preserved.
  Landing reward fractions can change floating-point rounding at the last bit;
  the default threshold/reward remain two-thirds/one-third of capacity.

No shipped balance values change in this final structural increment. Two alternate
provider inconsistencies are corrected: muzzle advance honors projectile speed,
and enemy spawn refills after applying its selected type capacity. Earlier
intentional firing-cost, combo-inheritance, difficulty, pickup and metadata
corrections remain documented separately in the progress report.
