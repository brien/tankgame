# R5 live render telemetry — 2026-10-27

The modern native/browser debug overlay now displays actual draw calls and
submitted triangles instead of zero placeholders. Lifetime 64-bit counters live
at the shared `ModernRenderer::Draw` submission boundary. The pipeline takes a
difference around `RenderAllPlayerViews`, including all valid player views, HUD,
menu and global diagnostics. Out-of-frame submissions do not affect frame totals.
Lines and loops count as draw calls but contribute no triangles; quad resources
count their prepared triangles. Counts describe submitted work, not visibility,
GPU timing or driver success.

The overlay presents the preceding completed frame (zero before the first frame).
This includes its own glyph submissions without requiring a partial-frame count.
The pipeline copies debug presentation data without changing the scene's immutable
snapshot. No additional GPU resources, queries or uploads are introduced. Pipeline
initialization/cleanup clears the displayed totals; recreated renderer resources
start fresh lifetime counters. Existing object-count/render-time fields are outside
this slice and retain their previous semantics.

Compatibility stays the native default with its existing diagnostic text policy.
This advances R5 telemetry; broader visual approval and explicit compatibility
retirement remain open. Windows/macOS runtime and multiplayer robustness remain
deferred.

## Validation

- Native and Emscripten builds pass. All 142 GL-enabled native tests pass, including
  real submission counting for quads/lines/loops, rejected draws, resource recreation
  and 64-bit text formatting. All three startup tests also pass in modern mode.
  Sandboxed SDL startup initially failed; display-enabled runs passed.
- One-/two-view Linux fixtures verify doubled scene submissions, frame-local totals,
  isolation from out-of-frame draws, global text, unchanged scene snapshots, empty
  viewports and cleanup. Captures exactly match independently rendered text using
  the preceding frame's totals. Depth remains unchanged and GL probes are clean.
- Browser title navigation, gameplay, firing and shutdown pass seven probes with
  zero GL errors and no context loss. No buffer/texture uploads occur after level
  entry. The gameplay screenshot displays Calls: 158 / Tris: 10110, matching the
  independently instrumented WebGL frame totals. Firing changes submitted work.
  Browser host warnings are retained in the run log. This instrumented run is not
  a performance benchmark or a broader browser/platform qualification.

## Reproduction

From the repository root:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/render-telemetry-2026-10-27/build-fixture.py
```

From `runtime/`, with display access:

```sh
TANKGAME_RENDERER=modern TANKGAME_TEST_GL=1 ./tankgame_tests --gtest_filter=VideoStartupTest.*
TANKGAME_RENDERER=modern ../build/telemetry-fixture
```

From the root (adjust SDK/Electron paths for the local installation):

```sh
python3 documentation/render-telemetry-2026-10-27/capture-png.py
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
```

With the server running:

```sh
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox documentation/render-telemetry-2026-10-27/browser-smoke.cjs
```
