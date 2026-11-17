# Gameplay configuration consolidation plan

## Objective

Give gameplay tuning values one authoritative definition and a predictable application lifecycle. Keep the definitions compiled into C++ for this work. A later change should be able to replace the built-in definition provider with a file loader without rewriting tank, player, projectile, or combat behavior.

Separate structural refactoring from intentional gameplay corrections in commits and validation. Existing tables do not always describe current behavior, so neither the tables nor the runtime behavior should silently become the specification for every field.

## Scope and decisions

- Consolidate tank, weapon, movement, projectile, difficulty, pickup, and player-rule tuning. Move numeric parameters of existing behaviors; retain the behaviors themselves in C++.
- Preserve ordered primary/secondary tank combinations.
- Keep health independent of color unless explicitly configured otherwise in future work.
- Fix ignored firing costs, implicit zero combo overrides, difficulty application, and stale level metadata.
- Keep existing level text maps and JSON sidecars. Do not introduce a new gameplay file format, live reload, save format, scripting, or a general-purpose effects system.
- Preserve existing rendering, sound, input bindings, and mode transitions. Only move input-side balance constants when they affect movement or energy consumption.
- Do not bundle enemy progression redesign or broad balance changes into this work.

## 1. Establish the behavior baseline

Inventory authoritative definitions and all writes to tank tuning fields in `TankTypeManager`, `Tank`, `Player`, `PlayerManager`, `TankHandler`, `Bullet`, `CombatSystem`, and input handlers. Record values, units, consumers, and whether each field is active, ignored, duplicated, or derived.

Capture resolved values for every supported ordered color pair and for player/enemy roles at each difficulty. Trace actual creation, respawn, level transition, pickup, and difficulty-change paths; do not infer effective difficulty from `ApplyDifficultySettings()` alone. It currently multiplies mutable stats, while some calls occur before a player owns a tank.

Extend existing jump, projectile, and player regression coverage around observable behavior. Keep baseline tests separate from tests specifying intentional corrections. Identify tests that construct an unconfigured `Tank` and give those fixtures explicit setup rather than preserving contradictory constructor defaults for their sake.

Deliverable: a compact before/after behavior matrix accompanying the implementation, including all changes listed in phase 4.

## 2. Introduce typed definitions and a pure resolver

Add a small `src/gameplay/` module with proposed responsibilities:

| Component | Responsibility |
|---|---|
| `GameplayDefinitions` | Immutable collection of tank defaults, type definitions, combo overrides, difficulty and role modifiers, projectile rules, and player rules |
| `BuiltInGameplayDefinitions` | The single source of compiled tuning values |
| `GameplayConfigResolver` | Pure composition of definitions into complete effective tank/weapon stats |
| `GameplayConfigValidation` | Validate definitions before gameplay uses them |

Use plain C++14 structs and explicit ownership. Prefer unit-bearing names such as `fireIntervalSeconds`, `jumpEnergyPerSecond`, `jumpLaunchSpeed`, and `healthRegenPerSecond`; `fireRate` currently means an interval, not shots per second.

Distinguish complete resolved stats from sparse combo overrides. Use an explicit C++14-compatible presence flag/value wrapper or equivalent typed representation. Missing means inherit; an explicit zero remains a valid override where allowed. Never use zero as the missing-value sentinel. Validate tank-type keys and use checked lookups rather than map indexing that silently inserts defaults.

Resolve in this order:

1. Common tank defaults.
2. Primary type definition.
3. Ordered primary/secondary combo overrides.
4. Role and encounter modifiers, including free enemy jumping and the existing first-level attack exception.
5. Player difficulty modifiers, applied only to player stats.

Return a complete value object without touching a live tank, querying application singletons, or performing I/O. Tests can provide an alternate definition set without mutating global maps.

Construct and validate the built-in definitions explicitly during game setup, before entities are created. Remove the translation-unit static initializer and repeated map initialization. The game session owns the immutable definitions and exposes them to creation/resolution paths with a lifetime covering all consumers.

Validation covers complete type coverage, finite numbers, positive capacities and projectile lifetimes, nonnegative costs/regen/intervals, integral nonnegative bounce limits, valid references, and consistent min/max bounds. Permit explicit zero costs and intervals deliberately; reject invalid values with a field-specific diagnostic. A failed definition build must not partially replace the active definitions.

## 3. Make stat application and entity state separate

Make one initialization path responsible for configured tanks. The constructor supplies safe structural state; it must not contain an alternate balance table. Replace overlapping tuning assignments in `Tank::Init()` and `SetType()` with resolver results. Retain temporary compatibility wrappers only while migrating call sites, then remove them.

Keep current health, energy, position, velocity, cooldown progress, identity, and alive/dead status separate from resolved maxima and rates. Prefer a single effective-stat member with read accessors over copying the same configuration into multiple independently writable fields. Migrate HUD extraction and gameplay consumers accordingly; debug cheats should modify runtime state or use explicit overrides.

Define application semantics explicitly:

| Event | Stat and state policy |
|---|---|
| Fresh spawn / respawn | Resolve from types, role, encounter, and current difficulty; initialize health/energy to resolved maxima and reset transient state |
| Color pickup | Resolve again with current difficulty; preserve health and motion, clamp resources to new maxima, then apply the pickup reward |
| Difficulty change | Resolve from original definitions, never multiply current stats; preserve absolute health/energy and clamp to new maxima without free healing |
| Reapplying the same configuration | No multiplier accumulation and no refill/reset of transient state |
| Level transition | Follow existing entity lifecycle; every newly created tank uses the same configured spawn path |

If applying a changed fire interval to a live tank, retain elapsed cooldown time; evaluate readiness against the new interval. Keep existing in-flight projectile stats stable when the firing tank changes type.

## 4. Correct contradictions in separate behavior commits

These are proposed target behaviors, not incidental consequences of the refactor:

| Issue | Target behavior and validation |
|---|---|
| `SetType()` ignores configured firing cost | Apply the resolved `fireCost`. Treat this as a balance correction: verify available shots, energy consumption, and recharge for every ordered pair. Document the increase from the current universal cost of 10 where applicable. |
| Missing combo fields overwrite base values with zero | Inherit missing values. For example, blue/grey inherits a 0.5-second interval. Test explicit zero separately from omission. |
| Difficulty bonuses stack, disappear on pickups, or miss newly created tanks | Resolve difficulty at spawn and each stat refresh. Test repeated changes, easy → hard, respawn, level changes, and pickups. |
| Stored `specialCost` does not control specials | Remove the unused tank energy-cost field. Introduce a distinct weapon `specialChargeCost`; initialize it to 2 to preserve the currently effective cost from `fireCost / 5` with firing cost 10. Changing normal firing costs must not silently rebalance specials. |
| Pickup match check runs after changing the primary type | Capture the previous primary type first, then award full energy for a match and half for a mismatch, capped at the resolved maximum. Update both pickup paths or consolidate them into the active player-owned path. |
| Conflicting respawn and special-charge constants | Keep the values used by the active lifecycle code in the definition set; remove unused duplicates after checking references. |

Keep enemy free jumps and encounter-specific attack reductions as named modifiers rather than post-initialization mutations. Preserve their current behavior while making their provenance explicit.

## 5. Consolidate remaining behavior parameters

Move launch speed, thrust, gravity, vertical speed limits, movement/turbo costs, and rotation parameters into movement definitions. Preserve integration order and frame-time behavior. Run existing jump tests at their supported frame rates, including partial-energy thrust and enemy zero-cost jumping.

Move projectile speed/lifetime, purple charge bounds/scaling, ricochet power multipliers, and blue special hit increments into projectile/weapon definitions. Preserve the distinction between constructing a fresh projectile and copying an already configured projectile so damage modifiers are never applied twice.

Move healing fractions, friendly-fire numeric rules, pickup rewards, combo decay/charge conversion, special-charge maximum, and active respawn delays into small domain-specific rule structs. Keep type-specific branching in existing systems. Extract numeric tuning without introducing a generic behavior interpreter.

Search for remaining gameplay numeric literals and direct stat assignments. Classify intentional algorithm constants, geometry tolerances, debug values, and any deferred parameters in the inventory; avoid claiming configuration coverage that has not been audited.

## 6. Make existing level configuration reliable

Parse each level's metadata into a fresh default-initialized temporary object. Validate it and commit only after successful parsing. Missing or invalid metadata uses a complete default object with a clear log; missing fields in valid metadata inherit defaults, never the previous level's values. Retain existing map-load behavior independently.

Keep `baseEnemyCount` and `enemyMultiplier` active and validate their ranges and safe count conversion. Preserve the existing progression formula during this work, but document that it uses ASCII-based level numbers and therefore jumps at the transition from 9 to A; correcting progression is a separate behavior change.

Mark level `difficulty` as descriptive metadata, distinct from selected player difficulty. Remove the unused `itemSpawnRate` member and entries from shipped sidecars, documenting it as an ignored legacy key for older files. Do not invent item spawning behavior to justify an unused setting.

Test missing files, omitted fields, malformed JSON, invalid numeric values, and sequential level loads that previously retained stale metadata.

## 7. Verify and leave a clean loader boundary

Run focused resolver/validation tests plus existing `test_tank_jump`, `test_bullet_special`, `test_player`, and `test_player_manager_regression` coverage as appropriate. Then build and run `ctest --test-dir build --output-on-failure`. Use the repository's browser build/smoke workflow to check shared gameplay on that target. Manually exercise firing, charged/special shots, jumping, pickups, difficulty changes, death/respawn, and level transitions in single-player, cooperative, and versus modes. Capture evidence for changed visible behavior.

Finish documentation with field units, resolution precedence, application semantics, intentional balance changes, and the remaining deferred parameters. Demonstrate that supplying a second in-memory definition set changes resolved behavior without edits to gameplay consumers.

The later loader should only need to parse a versioned file into `GameplayDefinitions`, validate it, and supply it at the existing initialization boundary. Stable external type names and schema migration belong to that follow-up. No parser-specific types should leak into entities or combat systems. Live reload is a separate lifecycle feature, not implied by file loading.

## Suggested delivery sequence

1. Baseline inventory and focused characterization coverage.
2. Typed definitions, validation, resolver, and explicit ownership.
3. Unified entity initialization and state application, preserving baseline behavior where feasible.
4. Explicit combo, firing-cost, special-charge, difficulty, and pickup corrections with before/after evidence.
5. Remaining movement, projectile, combat, and player-rule parameter extraction.
6. Level metadata isolation, validation, and unused-field cleanup.
7. Full verification and final configuration contract documentation.

Where a structural migration requires a behavior correction, include a named regression test and identify the change in that commit rather than disguising it as behavior-preserving cleanup.

## Completion criteria

- Each in-scope tuning value has one authoritative built-in definition.
- No spawn, pickup, or difficulty path silently replaces resolved stats with unrelated defaults.
- Resolver behavior is deterministic; omissions, explicit zeroes, and modifier precedence are tested.
- Difficulty application is idempotent and survives entity lifecycle changes.
- Entity state and configuration have explicit, independently testable lifecycles.
- Existing level metadata cannot leak across loads or partially apply after parse failure.
- Intentional balance changes are documented and validated independently of structural changes.
- An alternate definition provider can be added without changing gameplay consumers.
