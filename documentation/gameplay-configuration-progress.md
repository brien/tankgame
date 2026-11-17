# Gameplay configuration implementation — first increment

This increment implements tank-type definition storage, validation, and pure
resolution, plus the associated weapon-cost corrections from the implementation
plan. It does not complete the broader configuration migration.

## Configuration contract

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
no longer rebuild mutable maps. Session ownership and injection into entity
creation remain a later increment. Tests can already construct and resolve an
alternate definition set without changing the global provider.

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
- Applying types preserves current health, energy, velocity, and elapsed firing
  cooldown. Invalid type resolution happens before any tank mutation.

`tests/test_gameplay_definitions.cpp` records the expected attack, interval, cost,
ricochet, and energy values for all 25 ordered pairs. Projectile regressions cover
the exact normal-fire energy boundary and the independent special-charge
boundary. HUD coverage verifies normal cost cannot change special tick counts.

## Remaining work

- Consolidate constructor/`Init()`/`SetType()` health, movement, and action
  defaults; separate resolved stats from mutable entity state.
- Introduce session-owned definitions and apply role/encounter/difficulty
  modifiers through the resolver.
- Make difficulty application idempotent and consistent across spawning,
  respawning, level transitions, and pickups; fix pickup match ordering.
- Extract remaining movement, projectile, combat, player, and respawn constants.
- Isolate and validate level metadata and remove unused settings.
- Complete the full lifecycle behavior matrix, manual gameplay review, and
  final loader-ready contract. No gameplay JSON loader is introduced here.

## Validation — 2026-11-17

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
