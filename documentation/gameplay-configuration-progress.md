# Gameplay configuration implementation progress

The first increment implemented tank-type definition storage, validation, and pure
resolution, plus the associated weapon-cost corrections from the implementation
plan. The second increment adds shared defaults and lifecycle resolution; the third
extracts movement, projectile, combat, and player rules. The fourth isolates and
validates level metadata.
The broader configuration migration remains in progress.

## First increment: configuration contract

`src/gameplay/GameplayDefinitions.h` contains plain C++14 types without entity,
application, renderer, or JSON dependencies. `BuildBuiltInDefinitions()` creates
the authoritative type and combination definitions. `ValidateDefinitions()`
checks complete type coverage, valid references, finite/nonnegative numeric
values, positive energy capacity, and color ranges. Diagnostics identify the
offending type/combination and field.

`ResolveTankType(definitions, primary, secondary)` copies the primary definition
and applies only present fields from the ordered combination. Explicit zero is
different from omission. Invalid type lookup throws without inserting a map
entry. Callers validate a complete candidate definition set before using it.

`fireIntervalSeconds` is elapsed time between shots; attack is projectile input
power; energy regeneration is per second; fire cost is energy per shot;
`specialChargeCost` is charge from the player's separate special meter per use.
Normal firing and special-charge costs are independent.

The existing `TankTypeManager` API is temporarily retained as a facade over one
immutable, function-local built-in definition set. Repeated `Initialize()` calls
no longer rebuild mutable maps. This facade remains for legacy callers; session
ownership and entity injection were subsequently added in the second increment.

## Intentional behavior changes

All normal shots previously cost 10. The following table shows their new resolved
costs; rows are primary type, columns secondary type.

| Primary / secondary | Grey | Red | Blue | Yellow | Purple |
|---|---:|---:|---:|---:|---:|
| Grey | 50 | 50 | 50 | 50 | 50 |
| Red | 20 | 10 | 20 | 12.5 | 10 |
| Blue | 100/1.5 | 100/1.5 | 100 | 100/1.5 | 100/1.5 |
| Yellow | 25 | 25 | 50 | 25 | 50 |
| Purple | 25 | 25 | 50 | 50 | 25 |

- Blue/grey, blue/red, blue/yellow, and blue/purple now inherit the blue
  0.5-second firing interval; previously omitted entries produced zero.
- Purple/grey, purple/red, and purple/blue now inherit 64 ricochets; previously
  omitted entries produced zero.
- Base attack values, all explicit combination overrides, base energy capacity,
  regeneration, movement cost, and colors are retained.
- Special attacks retain their effective cost of 2 charge for every type instead
  of increasing automatically with normal shot costs. HUD ticks, availability,
  and renderer cost thresholds consume the separate cost.
- Applying types preserves current health and energy subject to new capacity
  limits, plus velocity and elapsed firing cooldown. Invalid type resolution
  happens before any tank mutation.

`tests/test_gameplay_definitions.cpp` records the expected attack, interval, cost,
ricochet, and energy values for all 25 ordered pairs. Projectile regressions cover
the exact normal-fire energy boundary and the independent special-charge
boundary. HUD coverage verifies normal cost cannot change special tick counts.

## First increment validation — 2026-11-17

- Native Release configure and build succeeded.
- All 36 focused definition, firing, HUD, and jump tests passed.
- Full `ctest --test-dir build --output-on-failure`: 197 registered tests,
  191 passed, six GL-dependent tests skipped, zero failures.
- Emscripten Release configure and build succeeded using the installed SDK
  cache. The compiler reports an existing empty-character-token warning in
  `src/main.cpp:183`, outside this change.
- `git diff --check` passed.
- Manual testing passed, as reported by the user. Platform and scenarios were
  not specified.
- Automated browser runtime smoke and GL-enabled tests were not run for this
  increment.


## Second increment: defaults, ownership, and lifecycle

`GameplayDefinitions` now includes common tank defaults and easy/normal/hard
multipliers. `ResolveTank()` composes type and combo values with encounter attack
scaling, enemy jump rules, and player difficulty. Definitions are validated before
`GameWorld` accepts them, including composed-value overflow checks.

Each world owns an immutable definition set and passes shared ownership to its
tanks. Constructing `GameWorld(alternateDefinitions)` changes spawned tank stats
and tank colors without edits to gameplay consumers. Standalone tanks retain the
built-in fallback for fixtures and legacy construction. Tank moves preserve the
configuration context, definition ownership, and world reference.

Constructor, `Init()`, and type changes use one stat-application function. Current
health, energy, cooldown, and motion remain separate from immutable definitions.
Public effective-stat fields remain a compatibility layer for existing input,
rendering, and test code; conversion to read-only accessors is still pending.

| Value | Shared default before difficulty |
|---|---:|
| Health / health regeneration per second | 100 / 10 |
| Movement / rotation degrees per second | 5 / 150 |
| Collision radius | 0.2 |
| Jump energy per second / launch speed / thrust | 250 / 4 / 18 |
| Gravity / maximum rising speed / maximum falling speed | 16 / 10 / 15 |

These preserve the previous normal `Init()` → `SetType()` path. Direct constructor
and `Init()`-only users now receive the same defaults rather than their previous
alternate values. `Init()` also resets velocity, jump time, and firing cooldown.

| Difficulty | Health | Health regen/s | Energy capacity | Energy regen multiplier |
|---|---:|---:|---:|---:|
| Easy | 300 | 20 | 150 | 1.5 |
| Normal | 150 | 15 | 150 | 1.5 |
| Hard | 100 | 10 | 100 | 1 |

- Difficulty is remembered even before the first tank exists. Every player spawn
  and respawn applies it, then starts at full resolved health and energy.
- Reapplying difficulty recomputes from definitions instead of multiplying current
  values. Live changes preserve absolute resources and clamp to new maxima;
  increasing difficulty bonuses does not heal or refill a living tank.
- Type changes retain difficulty. They preserve motion and cooldown progress.
- Pickups compare against the previous primary type: matching colors restore a
  full resolved energy capacity, nonmatching colors restore half, both capped.
- The main update loop now collects through `Player::Update()` once per player
  per frame. The old level entry point delegates to the same player-owned logic
  for legacy callers, rather than maintaining a second reward implementation.
- Enemy spawning uses the resolver for free jumps and the first-level blue attack
  reduction. Player difficulty does not affect enemy stats. Releasing a tank
  clears player difficulty modifiers; taking control applies the new player's
  difficulty without refilling resources.

New regressions cover difficulty before spawning, repeated application, easy →
hard changes, resource clamping without resurrection, color changes, respawns,
both pickup entry points, one pickup per frame, shared constructor/Init defaults,
enemy rules, alternate session definitions, and moved tanks. Existing mode and
level-transition tests now also verify resolved health/energy/regen.

## Second increment validation — 2026-11-17

- Native Release and Emscripten Release builds succeeded.
- All 38 focused definition, lifecycle, firing, jump, and pickup-frame tests passed.
- Full `ctest --test-dir build --output-on-failure`: 205 registered tests,
  199 passed, six GL-dependent skips, zero failures.
- `git diff --check` passed.
- Manual testing completed and passed, as reported by the user for this second
  increment. Platform and scenarios were not specified.
- Automated browser runtime smoke and GL-enabled tests have not been run for
  this increment.

## Third increment: gameplay rule extraction

This increment preserves the built-in balance values while moving additional
parameters into the immutable session definition set. It adds these rule groups:

| Group | Parameters now supplied by definitions |
|---|---|
| Movement | Turbo energy use, turret input scaling/per-call limit, forward input cap |
| Projectiles | Speed, lifetime, query/registration radii, self-hit grace period, purple charge/turn rules, first-bounce power multipliers, yellow child counts/spacing/bounces/power/turn speed |
| Combat | Self/friendly healing and energy-return divisors, resource thresholds, blue special hit increment, streak/distance/bounce/kill combo formulas |
| Players | Spawn height, cooperative/versus respawn delays, combo decay and special-charge conversion/capacity, pickup score and energy fractions |
| Enemy encounter | Opening-level weakened enemy index and attack scale |

The built-in projectile speed/lifetime remain 33 units/s and 100 seconds. Purple
power remains clamped to 100–1,000. First-bounce multipliers remain 1.5, or 2 for
yellow/yellow. Health/energy return divisors remain 2/8. Spawn height remains 24,
respawn delays 0.5/1.5 seconds, special-charge capacity 100, and pickup score 100.

Projectiles retain shared ownership of their original definitions. Fresh shots
apply charge modifiers once; copied projectiles preserve already resolved power,
state, and rules. Ricochet children start with their parent's session defaults
without passing through fresh-shot charge calculations. Combat uses the bullet's
rules, while player combo-to-charge conversion uses the owning player's session.

Turbo consumption now has a separate tuning parameter from jump consumption;
its built-in value remains 250 energy/s, with the existing enemy exemption.
Controller bindings, repeated turbo actions, and consumption order are unchanged.
The turret limit remains a per-call limit; this extraction does not redesign its
frame-time behavior. HUD special-meter geometry and color use the player's
configured capacity rather than assuming 100.

The former Player static tuning constants and duplicate local respawn constants
are removed. `Player::GetSpawnHeight()` supplies the session value to spawn and
respawn paths. `Player::AddSpecialCharge()` uses the configured cap. Explicit
`SetSpecialCharge()` calls retain their existing uncapped behavior, including the
combat kill-combo award; this increment does not silently rebalance overcharge.

Validation requires finite/nonnegative tuning, positive divisors/capacities/
lifetimes/radii, ordered purple power bounds, nonnegative bounce counts, and child
counts from 1 to 1,024 to bound per-impact spawning. It also validates the composed
opening-level attack scale before accepting a session.

Tests supply alternate definitions to exercise projectile motion/expiry, copy
semantics, purple power and turn behavior, ricochet child spawning, healing,
blue power increments, combo conversion, pickup rewards, spawn height, movement,
special-meter scaling, and mode-specific respawn scheduling. Existing default
weapon-pattern and lifecycle tests remain the compatibility baseline.

## Third increment validation — 2026-11-17

- Native Release and Emscripten Release builds succeeded. The browser compiler
  still reports the pre-existing token warning in `src/main.cpp:183`.
- All 32 focused definition, session-rule, projectile, and HUD tests passed,
  including configured cooperative and versus respawn scheduling.
- Full `ctest --test-dir build --output-on-failure`: 214 registered tests,
  208 passed, six GL-dependent skips, zero failures.
- `git diff --check` passed.
- Manual testing completed and passed, as reported by the user for the third
  increment. Platform and scenarios were not specified.
- Automated browser runtime smoke and GL-enabled tests were not run for this
  increment.

## Fourth increment: independent level metadata loading

Every `LevelHandler::Load()` begins with a fresh `LevelMetadata` default object,
including empty-world and missing-map requests. A sidecar is parsed into a separate
candidate and committed only after all fields validate. Missing or invalid JSON
leaves complete defaults; neither previous-level values nor partially parsed
fields survive. Metadata failure does not prevent an otherwise valid terrain map
from loading. `GetMetadata()` exposes the effective configuration read-only.

The parser accepts optional fields and ignores unknown keys. It validates:

- An object root and object-valued `theme`/`gameplay` sections.
- String names/authors, boolean special-coloring, and integer ratings/IDs/counts.
- Nonnegative descriptive difficulty and base enemy count, representable as `int`.
- Theme IDs from -1 (the title-screen value) through the representable integer range.
- Exactly three finite color channels, each within [0, 1].
- Finite nonnegative enemy multipliers representable as `float`.
- Complete JSON input, rejecting trailing garbage or a second JSON document.

`difficulty` is explicitly a descriptive level rating; it does not change player
health/energy difficulty modifiers. The unused `itemSpawnRate` member and entries
in all nine shipped sidecars are removed. Older files containing that key remain
compatible: it is ignored, including its value, just like other unknown keys.

Enemy-count arithmetic retains the existing ASCII-based level progression and
normal float rounding. Subtraction/multiplication no longer overflow integers,
and results outside the integer range saturate before conversion. Counts remain
at least the configured base count. This is numeric range handling, not a new
operational spawn-count limit or a redesign of the 9 → A progression gap.

The intended gameplay difference is the removal of inherited metadata. For
example, level 8 now uses base count 5 and multiplier 1 (29 enemies), rather than
inheriting level 7's 12/2.6 (74 enemies), or the title screen's 0/0 (zero enemies).
Shipped levels 0–7 retain their existing counts. Absent theme/name fields likewise
use defaults rather than previous-level values.

Regression coverage exercises missing sidecars, omitted fields, sequential loads,
malformed input, invalid types/ranges/colors, atomic fallback after partially
valid input, empty/missing maps, ignored legacy keys, extreme count arithmetic,
legacy float rounding, descriptive difficulty, and the shipped sidecars/maps.

## Fourth increment validation — 2026-11-17

- Native Release and Emscripten Release configure/build succeeded.
- All eight metadata regressions passed, including the invalid-input matrix.
- Full `ctest --test-dir build --output-on-failure`: 222 registered tests,
  216 passed, six GL-dependent skips, zero failures.
- All nine staged browser metadata sidecars match the runtime files and omit
  `itemSpawnRate`.
- `git diff --check` passed.
- Manual testing completed and passed, as reported by the user for the fourth
  increment. Platform and scenarios were not specified.
- Automated browser runtime smoke and GL-enabled tests were not run for this
  increment.

## Remaining work

- Complete migration from public writable effective stats to read-only accessors.
- Extract remaining input sensitivity/deadzones, special firing-pattern layouts
  in `Tank::Special()`, landing energy rewards, and AI/encounter selection rules.
  The numeric opening-level attack modifier is now in session definitions.
- Complete the full lifecycle behavior matrix and final loader-ready contract.
  No gameplay JSON loader is added.
