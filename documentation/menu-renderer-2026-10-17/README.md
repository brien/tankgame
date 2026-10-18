# R5 title-menu selection — 2026-10-17–18

The existing title-menu selection outline and additive fill now render through
`MenuRenderer` on both modern native and Emscripten. Right/Down/Left navigation
moves the highlight across Solo, Co-op and VS; starting gameplay hides it.
The browser menu substitute and modern pipeline menu skip are removed. HUD and
font/text rendering remain deferred. The title lettering is existing terrain,
not newly implemented font rendering.

Two catalogue-owned immutable geometries use the existing shared shader with
explicit unlit colours. The menu copies the render context, replaces its view
with identity and draws at z=-1 under the current viewport projection, preserving
the reference layout. This deliberately preserves the existing perspective-sized,
per-viewport menu behavior; global UI scope and orthographic HUD/text layout are
follow-up R5 work. No buffers or programs are created per menu frame.

The modern pass disables depth testing/writes and culling, selects one-pixel lines
and additive ONE/ONE blending, then restores those states, including separate
RGB/alpha blend factors and equations. It does not mutate the shared camera
context. The GL loader now exposes the two core blend-state entry points on
native and GLES. Compatibility retains its immediate renderer and remains the
native default. A small reference correction disables/restores culling around
compatibility menus too: the clockwise fill could otherwise be culled by inherited
world/HUD state. Both paths now show the intended additive fill. Its alpha is
intentionally ignored by ONE/ONE, matching the original blend convention.

## Validation

- Native Release build succeeds; **122/122** tests pass with GL enabled. The initial
  sandbox run could not access the display; the rerun with display access passed.
- Emscripten Release build and link pass using the SDK cache with write permission.
- Linux compatibility/modern fixtures at 801x601 produce byte-identical BMPs for
  all three selections, a hidden menu and an out-of-range selection, in one and two
  viewports (ten pairs). Visible selections change the framebuffer; hidden/invalid
  selections do not. Depth pixels remain unchanged and GL probes return no error.
  Modern checks also verify nondefault incoming blend/cull/depth state and the
  world context survive. Representative PNGs are lossless conversions of captures.
- Chromium/Electron navigation captures visibly show the highlight moving between
  Co-op, VS and Solo. Six probes report zero WebGL errors and no context loss.
  No buffer uploads occur during menu navigation; four uploads on gameplay entry
  remain stable afterward. Menu draw counts stop increasing in gameplay. Output
  records clean renderer teardown and `Browser frame loop stopped.`
- The first browser attempt found the prior HTTP server unavailable; restarting
  the local server produced the passing checked-in report.

These checks qualify the menu-selection slice only. R5 HUD, text/cache, global UI
scope, resource lifecycle coverage and retirement decision remain open. R4 broader
scene coverage, physical DPI/browser-shell policy and deferred Windows/macOS
runtime and multiplayer robustness gates are unchanged. This is not native
compatibility retirement or full UI parity approval.

## Reproduction

From the repository root (native build already configured):

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/menu-renderer-2026-10-17/build-fixture.py
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
```

From `runtime/`, with display access:

```sh
../build/menu-fixture
TANKGAME_RENDERER=modern ../build/menu-fixture
```

Compare each `compatibility-*.bmp` against the corresponding `modern-*.bmp`;
they must be byte-identical. The fixture returns nonzero on its other assertions.
Serve `runtime/` on port 8766 and run the browser harness:

```sh
python3 -m http.server 8766 --directory runtime
env -u ELECTRON_RUN_AS_NODE /path/to/electron --no-sandbox documentation/menu-renderer-2026-10-17/browser-smoke.cjs
```
