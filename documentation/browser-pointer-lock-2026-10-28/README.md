# Browser mouse capture — 2026-10-28

The next R6 slice adds an explicit Capture mouse button to the browser adapter.
Mouse capture is optional: keyboard play continues without permission. The button
supports normal keyboard activation and calls the Pointer Lock API synchronously
inside its activation gesture. It reports capture and the browser's Escape release
control, and offers retry on synchronous errors or rejected promises. The cue is
hidden while the window is unfocused/hidden and uses a polite live announcement.

Independent pointer-lock listeners preserve SDL's existing handlers. SDL 2.28.4's
Emscripten mouse callback uses its observed `has_pointer_lock` state to forward
relative movement to C++; setting SDL relative mode is intentionally unnecessary
and would enable SDL's automatic recapture behavior. Each lock transition marks
the existing activity boundary dirty, clearing key/button snapshots, suppressing
held controls until release, draining relative motion and resetting simulation
time before the next tick. The capture button itself is outside the SDL canvas.
There is no automatic capture on startup or after loss. Shutdown removes all added
listeners and controls and exits any lock owned by this canvas; late rejection
callbacks cannot update removed UI.

Native input and renderer defaults are unchanged. Audio, gamepad lifecycle,
fullscreen, context recovery, packaging, other browsers and deferred desktop
runtime qualification remain separate work.

## Validation

- Native and Emscripten Release builds pass. All 145 native tests pass with
  `TANKGAME_TEST_GL=1`, including the existing input/timer reset coverage.
- The checked-in Chromium/Electron harness checks real gesture capture, sends
  relative movement, releases with `document.exitPointerLock()`, recaptures,
  transfers window focus, injects a rejected request, and retries successfully.
  It then checks normal Escape menu/shutdown behavior after releasing capture and
  verifies that activity listeners and the capture control are removed.
- All nine WebGL probes report zero errors/no context loss and four stable buffer
  uploads. Frames stop while blurred. Capture clicks leave the energy meter full.
  The report and screenshots are adjacent to this file.
- Physical Escape release while captured and physical mouse feel still need
  manual validation; Electron input injection is not browser-chrome input.
  Relative-motion injection is a smoke check, not a measured turret-angle test.
- An initial harness run tried to serialize a restored JavaScript function across
  Electron IPC; the harness now discards that assignment result. A run overlapping
  relinking was discarded; final evidence uses a completed build and cleared cache.

## Reproduction

From the repository root (SDK cache and display access required):

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
```

In another terminal:

```sh
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox documentation/browser-pointer-lock-2026-10-28/browser-smoke.cjs
```

For manual validation, enter gameplay, activate Capture mouse, move/aim and fire,
press Escape to release, then release held controls before recapturing. Verify
that denial leaves keyboard play usable and a new capture attempt requires a new
gesture. Repeat after a focus round trip.
