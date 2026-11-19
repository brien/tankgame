# External gameplay configuration evidence — 2026-11-19

The later [default-loading follow-up](default-loading/README.md) supersedes the
opt-in and fatal-on-invalid startup behavior recorded below. The original
reports remain historical evidence.

Working-tree implementation on `feature/external-gameplay-configuration`.
No commit or PR is implied.

- [Native CTest](ctest.txt): 248/248 passed with software OpenGL and dummy audio.
- Default browser: [31 lifecycle checks](lifecycle.json) and
  [24 multiplayer checks](multiplayer.json).
- Packaged exported defaults: [startup and file identity](packaged-builtin.json).
- Packaged alternate rules: [startup and file identity](packaged-alternate.json),
  [31 lifecycle checks](selected-lifecycle.json),
  [24 multiplayer checks](selected-multiplayer.json), and
  [gameplay capture](alternate-gameplay.png).
  The capture was inspected: the player tank is magenta and health is 360/360,
  reflecting configured base health 240 with the normal difficulty multiplier 1.5.

- [Unsupported-version startup failure](unsupported-version.json): visible retry
  screen, configuration diagnostic, no frame loop.
- [Malformed JSON startup failure](malformed-json.json): the same safe failure path.
- [Automatic rebuild after source edit](browser-source-edit-build.txt) and
  [missing-source configuration failure](browser-missing-source.txt).
- [Native build](native-build.txt) and [default browser build](browser-default-build.txt).
  The browser compiler retains the existing empty-character-token warning in the
  JavaScript audio-status string embedded in `src/main.cpp`.

The browser cache was cleared back to built-in configuration after testing; the
[restored build](browser-restored-build.txt) passed.

## Native reproduction

```sh
cmake -S . -B build -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
cmake --build build -j4
TANKGAME_TEST_GL=1 LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy ctest --test-dir build --output-on-failure
./runtime/tankgame-linux --export-gameplay-config /tmp/gameplay.json
./runtime/tankgame-linux --validate-gameplay-config /tmp/gameplay.json
```

Use a new export path; exports intentionally refuse existing files. The standalone
schema was checked with Python `jsonschema` Draft 2020-12 tooling outside the
repository. Both exported defaults and the alternate snapshot passed. This is a
validation tool, not an added application/build dependency.

## Browser reproduction

Use the repository's configured Emscripten build and installed SDK cache. Serve
`runtime` on port 8766. The existing lifecycle and multiplayer drivers are in
`documentation/gameplay-configuration-2026-11-18/`; copy them to temporary output
directories before running to avoid replacing historical evidence. Run drivers
sequentially because they focus windows and inject input.

For a packaged snapshot:

```sh
cmake -S . -B build-browser -DTANKGAME_GAMEPLAY_CONFIG=/tmp/gameplay.json
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j2
python3 -m http.server 8766 --directory runtime
```

With Electron installed outside the repository, run the retained configuration
smoke driver:

```sh
CONFIG_EXPECTED=/tmp/gameplay.json CONFIG_OUTPUT=/tmp/config-smoke \
  env -u ELECTRON_RUN_AS_NODE /path/to/electron --no-sandbox \
  documentation/gameplay-external-configuration-2026-11-19/configuration-smoke.cjs
```

The alternate snapshot changes `definitions.tank.maxHealth` to 240,
`definitions.projectile.speed` to 20, `definitions.types.grey` RGB to 1/0/1,
and both `definitions.player.startingPrimaryType` and `startingSecondaryType`
to `grey`. Modify the same selected source file, then build without an explicit
configure step. CMake should automatically restage and relink it. The smoke driver
checks exact packaged file bytes as well as successful gameplay startup and WebGL.

For invalid-startup checks, replace the selected file with malformed JSON or an
unsupported version, rebuild, then run the same driver with `CONFIG_MODE=invalid`
and a fresh `CONFIG_OUTPUT` directory. It checks the visible failure/retry state,
absence of a frame loop, and a configuration-specific diagnostic in the log.

Restore normal browser packaging with `-DTANKGAME_GAMEPLAY_CONFIG=` and rebuild.

These are automated Linux software-graphics checks and an inspected capture.
No Windows/macOS runtime, physical controller, manual user acceptance or extended
soak testing is inferred.
