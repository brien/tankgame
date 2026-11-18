# Gameplay configuration completion evidence — 2026-11-18

Working-tree implementation based on `aa9dfcd`; no new commit is implied.

- Native Release build: passed. Graphics-enabled CTest: **233/233 passed**, no
  skips, using Linux software OpenGL and dummy audio. See [ctest.txt](ctest.txt).
- Emscripten Release build: passed. The existing `src/main.cpp:183` empty-character
  token warning remains unrelated to this migration.
- Browser lifecycle: **31/31 checks passed**, using Electron 38.2.0 on Linux.
  Includes real WebGL frames, emulated standard-gamepad firing/reconnection,
  level advance, audio activation/focus, two context-loss recoveries, resize/DPI,
  fullscreen, shutdown, download retry and failed-recovery shutdown. See
  [assertions](lifecycle/browser-smoke.json) and [gameplay capture](lifecycle/browser-playing.png).
- Multiplayer: **24/24 checks passed**, eight sessions across single-player,
  cooperative and versus modes, fifteen multiplayer reloads, viewport isolation,
  GL checks and clean shutdown. No browser console errors were recorded. See
  [assertions](multiplayer/browser-smoke.json) and [co-op capture](multiplayer/browser-mode-1.png).

The browser drivers are copies of the October release and November multiplayer
harnesses, updated for the current pause flow. Play → pause → menu → shutdown
requires three Escape presses; each session return requires two. The release
harness also explicitly focuses the window/canvas before key injection, matching
the multiplayer driver. Two initial release attempts passed their first 26
checks but failed the obsolete two-Escape shutdown expectation. Inspection of
the application log and `GameTask` confirmed that the game had reached the menu.
No production behavior was changed to satisfy the test. The corrected full run
passes all 31 assertions. Expected Electron GPU diagnostics occur during forced
context loss; the recovered contexts report GL_NO_ERROR.

## Reproduction

```sh
cmake -S . -B build -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
cmake --build build -j4
TANKGAME_TEST_GL=1 LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy ctest --test-dir build --output-on-failure
cmake -S . -B build-browser -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j2
python3 -m http.server 8766 --directory runtime
```

With that server running, run these sequentially using an installed Electron
runtime (the temporary installation used here is outside the repository):

```sh
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-browser-tools/node_modules/electron/dist/electron --no-sandbox documentation/gameplay-configuration-2026-11-18/lifecycle/browser-smoke.cjs
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-browser-tools/node_modules/electron/dist/electron --no-sandbox documentation/gameplay-configuration-2026-11-18/multiplayer/browser-smoke.cjs
```

The drivers focus windows and inject input. Copy them to a temporary directory
before rerunning if the retained evidence should remain unchanged. Release JSON
retains check results; verbose startup console logs were omitted from the saved
report. Captures were inspected for gameplay/HUD and split-screen presentation;
this is not a cross-driver pixel comparison or performance benchmark.

The user reported completed manual testing on 2026-11-18. Platforms and scenarios
were not specified; Windows/macOS runtime, physical controllers and extended soak
are not inferred from that report. Automated native tests cover
configuration-specific pickup, difficulty, special-shot, jump, death/respawn and
level-transition cases; the browser checks provide shared-runtime integration
coverage. Prior manual acceptance of increments 1–4 remains in the progress log.
