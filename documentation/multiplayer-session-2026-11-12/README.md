# Multiplayer session and level transitions — 2026-11-12

Starting Single player, Co-op or Versus now creates a fresh session with the
selected player count and matching game/player-manager mode flags. Previous
players, scores, wins, combo/charge state and world entities do not carry over.
Returning to the menu still freezes and displays the previous world until a new
session starts. The `2` menu shortcut selects Co-op without modifying that world.

Level changes requested by input, exit tiles, debug controls or death timers are
applied at the end of the gameplay update. Requests from both players coalesce;
a retry takes priority over advancement. Both players release their tank pointers
before world destruction, queued old-world events are discarded, and all active
players receive new tanks after loading. Level changes preserve player statistics;
new sessions reset them.

Versus mode now reaches the player manager, uses the requested spawn coordinates
(after tank initialization), and awards the surviving player one win per round.
Simultaneous deaths award neither player a win. The old tank-level award incorrectly
credited the dead player, while the manager awarded the survivor every frame.
Player physics and combo decay now run once per frame; death timers and reloads
have one owner. Death effects run once rather than every waiting frame.

The level filename is now a string. The previous 18-byte array could not hold
`levels/level0@@.txt` plus its terminating null, so ordinary level loading wrote
past the buffer.

## Validation

- Native and Emscripten builds succeed.
- 167 CTest cases pass with graphics tests enabled and software OpenGL.
- Eleven focused session/event tests pass under AddressSanitizer and
  UndefinedBehaviorSanitizer, with leak checking disabled for this lifetime check.
- Regression tests cover all nine mode transitions, repeated co-op deaths for
  either player, pending-event disposal, deferred input-triggered reloads,
  repeated versus rounds, simultaneous deaths and authored versus spawn positions.
- Linux native smoke exercises play/restart/co-op/single-player and exits 0.
  The retained single-player-after-co-op capture has one full-screen viewport.
- The Chromium/Electron harness exercises eight sessions and fifteen multiplayer
  level-advance requests, checks viewport layout and GL errors, and checks clean
  shutdown. It explicitly focuses the window/canvas before injected keys. Earlier harness
  runs missed menu transitions (no new session-start log), without a runtime trap. See the
  adjacent JSON for the final run's results.

## Reproduction

```sh
cmake --build build --parallel 4
TANKGAME_TEST_GL=1 LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy ctest --test-dir build --output-on-failure
cmake --build build-browser --parallel 4
python3 -m http.server 8766 --directory runtime
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox documentation/multiplayer-session-2026-11-12/browser-smoke.cjs
```

The adjacent `native-smoke.py` adapts the November 7 driver with an explicit
focus-settling delay before injected keys. Only representative native
screenshots are retained; the complete native transcript is compressed.

This fixes the reproduced session-state failure and identified unsafe reload
paths. It does not establish exhaustive long-session, physical two-controller or
Windows/macOS runtime coverage. Level-map spawn design remains unchanged: maps
without explicit versus spawn markers retain their existing fallback coordinates.
No new match-results UI or automatic match-ending rule is introduced.
