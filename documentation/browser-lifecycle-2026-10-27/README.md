# Browser focus and timing slice — 2026-10-28

The browser adapter pauses the complete task tick while `document.hidden` is true
or `document.hasFocus()` is false. Independent DOM listeners preserve activity
changes even if blur and focus both happen between animation callbacks, without
replacing SDL's handlers. A status message appears while unfocused. Returning
clears keyboard/mouse snapshots and relative motion and resets `GlobalTimer`.
Held keys/buttons are filtered until release, avoiding stale movement/fire or
synthetic release edges. Input teardown now releases both allocated key snapshots.

The same reset occurs after callback gaps over 250 ms, deliberately discarding
that elapsed time rather than feeding it to simulation. Normal active frame
timing is unchanged. Performance interval counters reset with simulation timing,
so suspension time is not reported as an active rendering slowdown. Shutdown
removes the DOM listeners and status element before task teardown.

This starts the independent R6 lifecycle work while retaining the native
compatibility default and R5 visual-approval/retirement gates. Native scheduling
does not use this policy. Audio is still disabled in browser builds; pointer-lock
activation/loss, controllers, context recovery and release/browser qualification
remain follow-ups. There is no new macOS/Windows runtime or multiplayer claim.

## Validation

- Native and Emscripten Release builds pass. All 145 native tests pass with GL
  enabled; the three modern startup tests also pass. New tests cover hidden and
  unfocused scheduling, first/resumed ticks, focus round trips between frames,
  long-gap resets, cleared key/mouse edges and timer reset. See build/test logs.
- Chromium/Electron smoke enters gameplay, holds movement/fire, transfers focus
  to another window, then hides the game window. Frame submissions stay at 69
  throughout both pauses. Returning resumes submission. Energy recovers from
  0.394 to 0.400 with the stale mouse press, then drops to 0.180 after a fresh
  press. All ten probes have zero GL errors/no context loss and four unchanged
  buffer uploads. A deliberate 600 ms main-thread stall recovers, and normal
  Escape shutdown removes activity state/listeners. These values are evidence
  from this run, not fixed performance expectations.
- Screenshots show the pause cue and resumed gameplay. `browser-smoke.json`
  contains observations and console output. Host DBus/GTK warnings are retained
  in `browser-run.txt`.
- Initial browser attempts exposed a cached JavaScript/WASM mismatch; the harness
  now clears its HTTP cache. The previous smoke harness disabled background
  throttling, which also prevents Electron from setting `document.hidden`; this
  harness uses normal window visibility behavior. Sandboxed GL tests required
  display access and the browser linker required writable SDK-cache access.

## Reproduction

From the repository root, with display and SDK-cache access:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
```

In another terminal:

```sh
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox documentation/browser-lifecycle-2026-10-27/browser-smoke.cjs
```

From `runtime/`:

```sh
TANKGAME_RENDERER=modern TANKGAME_TEST_GL=1 ./tankgame_tests --gtest_filter=VideoStartupTest.*
```
