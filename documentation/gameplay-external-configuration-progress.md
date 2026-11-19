# External gameplay configuration implementation progress

Implementation on `feature/external-gameplay-configuration`, following the
[approved plan](gameplay-external-configuration-plan.md), 2026-11-19.

## Implemented

- Version 1 complete JSON snapshots with stable type/difficulty names, all
  definition groups, ordered combinations, explicit-zero overrides and preserved
  cycle order. Strict decoding rejects unknown/missing fields, duplicate keys or
  pairs, incorrect types, unsupported versions, out-of-range numbers and oversized
  documents/arrays/depth. Existing semantic validation remains authoritative.
- Deterministic export from built-ins, bounded file loading and source-aware
  diagnostics. JSON types remain private to the codec; gameplay consumers and
  resolver interfaces are unchanged.
- Native `--gameplay-config`, `--validate-gameplay-config`,
  `--export-gameplay-config` and `--help`. Headless commands precede SDL/task
  allocation. Export refuses existing destinations and removes failed output.
  Startup loads `gameplay.json` by default. Missing or invalid default/alternative
  files log the reason and fall back to built-ins; validation commands remain strict.
- `GameTask` accepts definitions before constructing its world. Files are read once;
  all mode/level/entity lifecycles retain immutable ownership. The startup log identifies the provider.
- Optional browser packaging through `TANKGAME_GAMEPLAY_CONFIG`, staged at
  `/gameplay.json`, with build dependency tracking. Without an override the
  browser packages `runtime/gameplay.json`.
- [Authoring guide and field reference](gameplay-configuration-format.md),
  [editor schema](gameplay-configuration.schema.json), and regression coverage.

## Original validation (before default-loading follow-up)

- Native configure/build passed. Graphics-enabled CTest: **248/248 passed**, zero
  skips/failures, using software OpenGL and dummy audio.
- New tests compare every field, alternate numeric values and all 75 resolved
  player pair/difficulty combinations, plus enemy modifiers. They cover strict
  parsing, file failures, exclusive export, command selection and exit status.
- File-loaded rules are exercised through firing, pickups, difficulty changes,
  respawn, level reloads and single-player/cooperative/versus transitions.
  Editing a file leaves an existing world unchanged and affects the next startup.
- Emscripten default build passed. Existing browser lifecycle **31/31** and
  multiplayer **24/24** checks passed.
- JSON Schema validated with Draft 2020-12 tooling; exported built-ins and an
  alternate configuration both conform.
- Packaged exported defaults and alternate snapshots passed byte-identity,
  startup, gameplay and WebGL checks. The alternate snapshot also passed all
  **31 lifecycle** and **24 multiplayer** checks. Its capture was inspected.
- Editing the selected source file automatically restaged/relinked the browser
  build. Missing source files failed CMake configuration. Unsupported versions
  and malformed JSON produced a visible error/retry screen without a frame loop.
- The browser cache was restored to built-in defaults and rebuilt successfully.
- `git diff --check` passed.
- [Retained logs, assertions, capture and reproduction](gameplay-external-configuration-2026-11-19/README.md).

Implementation and automated validation are complete; manual acceptance remains
outstanding.

No manual user acceptance or Windows/macOS/physical-controller qualification is
claimed. Live reload, an editor, remote fetching, persisted selection and balance
changes remain outside this workstream.

## Default-loading follow-up

The user requested automatic `gameplay.json` loading and logged fallback to
built-ins when loading fails. This supersedes the original plan's opt-in and
fatal-on-invalid startup policy, including the historical browser error-screen
checks above. Alternative files remain selectable with `--gameplay-config`.
Validation/export commands retain their original strict behavior.

Follow-up verification passed: native and Emscripten builds, **249/249**
graphics-enabled native tests, **25/25** browser startup/gameplay checks, and
`git diff --check`. Browser cases cover the default file, a missing default,
invalid default, valid alternative, and invalid alternative. Every case reaches
gameplay with the expected provider diagnostic and fallback policy.
See [follow-up evidence](gameplay-external-configuration-2026-11-19/default-loading/README.md).
