# R5 player HUD meters and icons — 2026-10-19

The modern native and Emscripten pipelines now render player-local health/energy
bars and borders, heart/energy icons, fire/jump cost markers, combo/special meters
and special cost ticks, and the debug timing meter. Native compatibility remains
the default and visual reference. Font text, FPS/debug strings and the animated
world-space targeting line remain follow-up slices. Existing title lettering is
terrain, not font rendering.

`HUDLayout` converts the existing HUD DTO to GL-free draw descriptions.
`HUDMeterRenderer` submits them through the catalogue's existing shared shader.
Menu primitives are generalized to positive unit UI quads/outlines with UVs;
menus apply a negative Y scale to preserve their original geometry. HUD segments
reuse the debug-line primitive. The heart and bang icons reuse catalogue textures.
No extra geometry buffers, texture owners, or shader programs are introduced.
Draw descriptions vary with gameplay values; GPU geometry remains immutable.

HUD coordinates retain the compatibility identity/orthographic projection inside
each active viewport, independent of the world camera. Opaque bars, alpha-blended
unavailable specials, and additive icons select explicit blend modes. The pass
disables depth testing/writes and culling and uses one-pixel lines. It restores
depth/blend/cull enables, depth mask, line width, separate RGB/alpha blend factors
and equations, texture-unit-zero binding and the active texture unit. A copied
render context preserves the caller's camera/projection.

Three narrow corrections also apply to compatibility:

- HUD lookup uses `playerId`, so a missing player-0 tank does not route player 1's
  HUD to the lower viewport. This is a rendering-boundary fix, not multiplayer
  session/respawn work.
- Hiding the energy bar also hides its action-cost lines.
- Zero, negative and nonfinite special costs produce no subdivision ticks;
  pathological counts are bounded at 1,024 before integer conversion. Normal
  gameplay tick spacing and the base line remain unchanged.

## Validation

- Native Release configure/build pass; **127/127** tests pass with GL enabled.
  Five new tests cover sparse/reordered player identity, visibility, clamped
  fill widths, affordability/distinct-cost markers and safe tick counts.
- Emscripten Release configure/build/link pass. Linking required permission for
  the SDK cache lock outside the workspace.
- Ten deterministic Linux compatibility/modern fixture pairs have **zero RGB
  channel differences** at 801x601, in single and odd-height split viewports.
  Cases cover distinct player values, overfilled/depleted bars, zero maxima/cost,
  hidden bars and all meters hidden. Icons, outlines, alpha specials and debug
  meters are included. Depth pixels stay unchanged; all GL probes are clean.
  Modern checks verify nondefault blend/cull/depth state, texture bindings and
  camera context restoration. These isolate HUD rendering, not full multiplayer
  gameplay or exhaustive scene parity. See `comparison.json` and fixture output.
- The existing menu fixture also passes all ten compatibility/modern pairs
  byte-for-byte after primitive reuse (`menu-regression.txt`).
- Chromium/Electron gameplay shows health/energy bars and textured icons; firing
  visibly reduces the energy fill. Four probes report zero WebGL errors and no
  context loss. Menu has zero player-HUD draws. After level entry, buffer uploads
  stay at four and texture uploads at zero through sustained firing and idle.
  Output confirms renderer teardown and `Browser frame loop stopped.`
- The initial browser counter used a tolerance smaller than the existing ES
  shader's mediump uniform quantization. Screenshots showed the HUD correctly;
  changing the diagnostic comparison to 0.001 yielded the passing report without
  a renderer change. Instrumented frame times are not a performance qualification.

This completes the HUD meter/icon slice, not R5. Text/font caching, targeting
stipple policy, global UI scope and compatibility retirement remain open. R4's
broader scene/DPI/browser-shell gates, deferred Windows/macOS runtime qualification
and multiplayer robustness are unchanged.

## Reproduction

From the repository root, with the existing native/browser toolchains configured:

```sh
cmake -S . -B build
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/hud-meters-2026-10-19/build-fixture.py
cmake -S . -B build-browser
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
```

From `runtime/`, with display access:

```sh
../build/hud-fixture
TANKGAME_RENDERER=modern ../build/hud-fixture
```

Then from the root:

```sh
python3 documentation/hud-meters-2026-10-19/compare.py
```

The fixture exits nonzero on state/depth/visibility/GL failures. The comparison
script expects ten pairs, writes representative PNGs and a JSON report, and accepts
at most one 8-bit RGB channel value of difference (observed maximum: zero).
Repeat `documentation/menu-renderer-2026-10-17/build-fixture.py` and both menu
fixtures to check menu parity; compare corresponding BMP files byte-for-byte.

Serve `runtime/` on port 8766, then run the browser test with an installed Electron:

```sh
python3 -m http.server 8766 --directory runtime
env -u ELECTRON_RUN_AS_NODE /path/to/electron --no-sandbox documentation/hud-meters-2026-10-19/browser-smoke.cjs
```
