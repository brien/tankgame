# R4 shared world debug lines — 2026-10-17

The pipeline now submits world-space diagnostic segments after transparent objects
and player overlays, before UI, once per valid player viewport. `SceneData` carries
endpoint/colour data, and scene extraction reads the existing debug flag. Press
**I on the title screen**, then start the game: live player/enemy simulation origins
receive red X, green Y and blue Z markers, each 0.75 world units long. The existing
debug controls and gameplay behavior are unchanged.

There was no active world-space debug-line producer in the pipeline. These origin
markers are new diagnostics, not a reproduction of a previous wireframe view.
Existing mesh edge resources are extruded geometry used for effects. HUD lines,
text, and targeting stipple remain in R5; this slice does not choose a stipple or
wide-line approximation for those features.

`ResourceManager` owns one immutable two-vertex segment, used by both backends.
Transforms map that segment to arbitrary endpoints; colours are unlit and
untextured. No per-frame buffer uploads or extra shader owners are introduced.
Lines are solid, one pixel wide, use LEQUAL depth testing without depth writes,
and disable blending. The pass restores the depth enable/function/write mask,
blend enable, and line width it changes; compatibility also restores its lighting,
texture, stipple and current colour state. Empty line lists do no work.
Native compatibility remains the default.

## Validation

- Native Release build succeeds. All **122/122** CTest cases pass with GL enabled.
  Three added tests cover arbitrary segment endpoints (including zero/negative
  components), live player/enemy marker extraction and reused scene cleanup.
- Emscripten Release builds and links successfully. Linking required permission to
  write the SDK cache lock outside the workspace.
- Linux native compatibility and modern fixtures at 601x501 exercise one and two
  viewports. Captures match byte-for-byte. The red and blue near lines are visible;
  the green far line is fully occluded. Depth pixels are unchanged, deliberately
  nondefault incoming depth/blend state is restored, and all GL probes are clean.
- Chromium/Electron gameplay records zero line draws before debug activation,
  1,974 after activation/start and 3,255 at the subsequent probe. Buffer uploads
  remain at four after level entry. Three WebGL probes report no error or context
  loss. `browser-debug.png` records the visible gameplay; `browser-smoke.json`
  contains counters and application output.

The first browser harness attempt pressed I during gameplay, where the existing
control is inactive. Correcting the harness to enable debug from the title menu
produced the passing report; no input behavior was changed.

This completes the initial R4 debug-line slice, not all of R4. The deterministic
fixture isolates line rendering and viewport behavior; it does not establish
exhaustive full-pipeline scene coverage, physical DPI behavior, Windows/macOS
runtime qualification, or multiplayer robustness. Browser shutdown was not tested
by this harness. Those gates and browser shell policy remain open.

## Reproduction

From the repository root:

```sh
cmake -S . -B build
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/debug-lines-2026-10-17/build-fixture.py
cmake -S . -B build-browser
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
```

The browser commands assume the already configured Emscripten build/toolchain.
From `runtime/`, with display access:

```sh
env -u TANKGAME_RENDERER ../build/debug-fixture
TANKGAME_RENDERER=modern ../build/debug-fixture
```

The fixture writes BMPs; checked-in PNGs are lossless conversions. Serve `runtime/`
on port 8766, then run the browser script using an installed Electron executable:

```sh
python3 -m http.server 8766 --directory runtime
env -u ELECTRON_RUN_AS_NODE /path/to/electron --no-sandbox documentation/debug-lines-2026-10-17/browser-smoke.cjs
```
