# R5 player status text — 2026-10-27

Modern native and browser HUDs now show health and energy with their maxima,
combo count and special charge. The existing HUD snapshot supplies all values,
colours and visibility flags; FPS visibility is independent. Four fixed rows at
the lower left of each viewport keep labels stable as flags change. Values round
to whole units, retain overfilled values and display `?` for nonfinite floats.
Special is the raw charge value, not a percentage or number of available shots.

Text uses the existing catalogue-owned atlas, glyph geometry and shared program.
No new textures, buffers or gameplay state are introduced. The text pass now
clips to its viewport intersected with the incoming scissor, restoring scissor
state afterward. Oversized labels and very small views are clipped rather than
allowed to draw into another player view.

Compatibility remains the native default and retains its existing text policy.
This implements the next additional-text slice within R5; it does not close other
UI coverage, retire compatibility, or change the Windows/macOS runtime and
multiplayer robustness deferrals.

## Validation

- Native and Emscripten Release builds pass.
- All 138 native tests pass with `TANKGAME_TEST_GL=1`, including two new status
  layout tests for player values/colours, independent visibility, viewport sizing,
  zero dimensions and nonfinite values.
- Linux modern fixtures at 801x601 with one and two player views show distinct
  values. Assertions verify visibility, unchanged depth, restored blend/depth/cull
  state, stable atlas identity, atlas recreation, incoming empty-scissor
  intersection, restored scissor state, and clipping across a split seam.
- Browser title, gameplay, firing and cached probes report zero GL errors and no
  context loss. Status glyph submissions increase throughout gameplay; buffer
  uploads stay at four after level entry and texture uploads at zero. Firing
  reduces displayed energy; shutdown is clean. Screenshots and `browser-smoke.json`
  retain the results. The instrumented run is not a performance benchmark.
- Browser runner logs include host DBus/GTK module warnings; the smoke harness
  reports no game errors. No new Windows/macOS or browser-matrix claim is made.

## Reproduction

From the repository root:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/hud-status-text-2026-10-27/build-fixture.py
```

From `runtime/`, with display access:

```sh
TANKGAME_RENDERER=modern ../build/status-fixture
```

From the root, convert fixture captures using the Python standard library:

```sh
python3 documentation/hud-status-text-2026-10-27/capture-png.py
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
```

With the server running, launch the browser smoke (adjust SDK/Electron paths to
local installation):

```sh
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox documentation/hud-status-text-2026-10-27/browser-smoke.cjs
```
