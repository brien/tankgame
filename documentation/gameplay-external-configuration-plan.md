# External gameplay configuration loading — implementation draft

Status: approved by the user on 2026-11-19; implementation on
`feature/external-gameplay-configuration`. See the
[implementation progress](gameplay-external-configuration-progress.md).

Subsequent user direction: startup now loads `gameplay.json` automatically and
logs/falls back to built-ins if the default or explicitly selected file cannot be
loaded. The opt-in/fatal-startup decisions below are historical and superseded.

## Objective and scope

Allow a complete external JSON definition file to replace the compiled gameplay
provider at application startup. Preserve existing balance when no file is selected
or when an exported built-in file is selected. Use the immutable ownership,
resolution, validation and entity lifecycles established by the
[configuration contract](gameplay-configuration-contract.md).

Include schema versioning, stable external names, strict decoding, export and
validation commands, native startup integration, browser asset integration, tests
and authoring documentation. Keep level metadata independent.

Exclude live reload, an in-game editor, persisted configuration selection, browser
file upload, remote fetching, per-level gameplay overrides, layered patches,
save-game migration and balance changes. Settings persistence can be a later
feature without changing the loading boundary.

## Approved decisions

- Files are complete snapshots, not patches over current compiled defaults.
  This avoids an old file silently changing behavior when built-in tuning changes.
- No selection means built-in definitions. Native users select a file with
  `--gameplay-config <path>`; relative paths resolve against the launch working
  directory. There is no implicit configuration discovery or environment override.
- An explicitly selected missing, unreadable or invalid file stops startup with a
  useful diagnostic and nonzero exit status. It never silently falls back.
- Configuration is read once before world construction. Menu returns, mode changes,
  respawns and level transitions retain the same immutable definitions. Editing
  the file takes effect only on the next application launch.
- Version 1 is the first external format. Unsupported versions are rejected;
  future migrations must be explicit and tested, rather than guessed now.

## 1. Define the version 1 file contract

Add `documentation/gameplay-configuration-format.md` with an exhaustive field
reference, units, required keys, bounds, examples and compatibility rules.

The root contains integer `schemaVersion: 1` and a `definitions` object. Its
sections match the existing `GameplayDefinitions` groups: `tank`, `types`,
`combinations`, `difficulties`, `projectile`, `player`, `combat`, `enemy`, `input`,
`joystick`, `gameCube`, `special`, `landing`, and `enemyBehavior`.

- Use stable lowercase type names `grey`, `red`, `blue`, `yellow`, `purple`;
  never serialize enum ordinals or C++ enumerator names.
- Key `types` by those names, with all five complete records required.
- Key `difficulties` by `easy`, `normal`, `hard`, requiring all three records.
- Represent `combinations` as an array of records containing `primary`,
  `secondary`, and `overrides`. Pair order matters; duplicate pairs are errors.
  Require the array, but permit it to be empty. An absent pair inherits its
  primary definition. Only the currently supported `TankTypeOverride` fields
  are accepted in `overrides`; omitted fields inherit and explicit zero remains
  present. An empty override object is valid.
- Represent enemy type cycles as arrays of `{primary, secondary}` records.
  Preserve sequence order and repeated entries because they affect selection.
- Require every complete-record field, including profile fields unused by a
  particular device. Do not inherit missing fields from built-in definitions.
- Reject unknown keys at every level, duplicate object keys, `null`, invalid
  references, numeric strings and booleans in numeric fields. Integer fields
  require integer JSON tokens within the destination range. Floats accept
  integer or fractional JSON numbers only if finite and representable.
- Accept exactly one JSON document, without comments or trailing content.
  Bound input size to 1 MiB and nesting to 32 levels, reporting clear errors.
  Existing semantic validation retains its pattern/cycle limits.

Publish a JSON Schema alongside the reference for editor assistance. The C++
decoder and `ValidateDefinitions()` remain authoritative for cross-field and
composed-value constraints. Test schema examples against the decoder.

## 2. Implement an isolated codec and loader

Add `GameplayDefinitionJson.h/.cpp` and `GameplayDefinitionLoader.h/.cpp` under
`src/gameplay/`, with naming adjusted to repository conventions during implementation.
Reuse the existing nlohmann_json dependency; add no new runtime parser dependency.

Expose text-to-definitions and definitions-to-text operations using standard C++
types. Keep nlohmann types private to the codec implementation. Keep file I/O in
the loader and diagnostics/logging policy at the application boundary.

Decode into a fresh candidate, enforce the version and structural contract, then
call `ValidateDefinitions()` before returning it. Never mutate an active world.
Preserve validation at `GameWorld` construction as protection for every provider.
Detect duplicate keys during parsing, before a DOM can discard them; enforce
size/depth limits before excessive allocation or recursive traversal.

Diagnostics identify the source path, JSON field path and reason, with byte or
line location for syntax errors where available. Distinguish I/O, syntax, schema
version, field-type and semantic errors. Use `Logger::Get().Write(...)` for game
startup failures; avoid dumping the full document into logs.

Serialize every definition field deterministically, with stable key/pair ordering
and sufficient float precision for round-trip equality. Preserve override presence
flags, including explicit zero. Validate before exporting. Exported built-ins are
generated from `BuildBuiltInDefinitions()`, not maintained as a second balance table.

## 3. Add native selection and authoring commands

Parse options before SDL/task initialization and before allocating application
tasks. Define mutually exclusive modes:

- Normal startup, optionally with `--gameplay-config <path>`.
- `--validate-gameplay-config <path>`: parse and validate, report success/failure,
  and exit without creating a window or world.
- `--export-gameplay-config <path>`: write the complete built-in version 1 file
  and exit without creating a window or world. Refuse an existing destination;
  avoid leaving a partial final file on write failure.

Reject conflicting modes and missing option values; document exit codes and help
text. Preserve any existing platform arguments discovered during integration.

Add a `GameTask` constructor accepting `GameplayDefinitions` and initialize its
`gameWorld` member from that value. Keep the default constructor delegating to
built-ins for existing fixtures/callers. `App::Initialize()` supplies the selected,
validated definitions before any entity creation. Keep parsing out of `GameWorld`,
entities, input, rendering and combat.

Propagate initialization failures through native `App::Run()`/`main()` so invalid
configuration returns failure; native `main()` currently returns zero after Run.
Handle errors before partial task allocation where possible and test cleanup.

## 4. Integrate browser packaging

Support deployment-selected configuration through an optional CMake cache file
path, proposed name `TANKGAME_GAMEPLAY_CONFIG`. With no setting, use built-ins.
When set, stage that file into the Emscripten virtual filesystem at a fixed path,
and select it before browser world construction using the same loader.

Inspect and extend the existing `src/CMakeLists.txt` asset/preload workflow. Track
the selected file as a build dependency so edits update the packaged artifact.
Report a missing source file during configuration/build and invalid contents at
startup. Do not require executing a target binary while cross-compiling; use the
native validation command in release checks.

Route load failures through the existing browser initialization failure path and
verify the error is visible in the browser shell. No frame loop starts on failure.
The initial browser feature uses packaged files; it does not fetch configuration
asynchronously or grant web pages arbitrary local filesystem access.

## 5. Add focused regression coverage

Add `tests/test_gameplay_definition_json.cpp` and loader/startup tests, registered
through the existing CMake test setup.

- Full built-in round trip: compare every field, all type/difficulty entries,
  ordered pairs, vectors and override presence. Compare all 75 resolved
  pair/difficulty combinations and enemy encounter modifiers.
- Alternate-provider round trip: vary every serializable field with valid values,
  verifying mappings do not omit fields or accidentally use built-in values.
- Strict parsing matrix: omitted/unknown/duplicate fields, duplicate pairs,
  unknown names, wrong types, nulls, unsupported/missing/noninteger versions,
  numeric overflow, invalid semantic bounds, trailing content, size/depth limits.
- Explicit-zero versus omitted combo overrides; empty combination table;
  ordered type pairs and ordered/repeated enemy-cycle entries.
- File errors, startup selection, working-directory-relative paths, conflicting
  options, headless validation/export, existing export destination and failure exit
  codes. Use temporary files independent of shipped assets.
- Load an alternate file through the actual application configuration boundary
  and verify configured spawning, firing, pickup, difficulty, respawn and level/
  mode transitions. Reuse existing lifecycle fixtures where practical.
- Confirm editing a source file does not mutate a running world's definitions or
  in-flight projectile rules; a subsequent startup loads the changed file.
- Browser smoke with built-ins, exported built-ins, a visibly changed alternate
  configuration, and malformed/unsupported configuration. Verify asset rebuilds
  pick up edits and startup failure is surfaced.

## 6. Validate and document delivery

Run focused codec/loader/startup tests, then native configure/build and full
`ctest --test-dir build --output-on-failure`, using the graphics-enabled test
environment where available. Build Emscripten and run existing lifecycle and
multiplayer smoke checks with default and selected configurations. Record results,
skips and platform limitations without inferring coverage.

Manually exercise firing/specials, jumping, pickups, difficulty, death/respawn and
level transitions with an alternate configuration. Retain logs and captures for
visible changes. Run `git diff --check`.

Update the configuration contract and add authoring instructions for export,
edit, validation, native launch and browser packaging. State explicitly that
selection is not persisted and files are read only at startup. Document future
version policy: retain a decoder/migration for supported older versions, migrate
to the current in-memory model, then validate; reject unsupported versions with
an actionable diagnostic. Do not invent a version 0 migration.

## Suggested delivery sequence and acceptance criteria

1. Format contract, strict codec, deterministic export and round-trip tests.
2. File loader, headless commands, startup injection and failure handling.
3. Browser packaging, runtime smoke coverage and authoring documentation.
4. Full regression validation and evidence closeout.

Complete when an exported built-in file reproduces default definitions and
behavior, an edited valid file changes gameplay through the existing provider
boundary on both targets, invalid selected files fail atomically and clearly,
all configuration groups round-trip, and no entity/combat consumer depends on
JSON or file paths. No shipped tuning change is part of this implementation.
