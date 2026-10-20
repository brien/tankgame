# R5 animated player targeting line — 2026-10-19

The modern desktop and browser pipelines now render the targeting line using the
existing HUD DTO, player-ID routing, catalogue line geometry and shared program.
The ray starts 0.1 units above the player, extends 32 world units along the DTO's
combined body/turret angle, and fades from targeting colour to black. It renders
in each player's world camera before the screen-space HUD meters.

The material enables a shader stipple mode, reset on every ordinary draw. Its
four mask nibbles preserve all 16 bits even on GLES2 implementations with mediump
float precision. Each bit spans 16 drawing-buffer pixels along the major raster
axis. Animation retains the DTO's truncated, wrapped mask value. Hidden rays,
zero masks, invalid animation/transform values, zero-area viewports, fully clipped
and screen-degenerate segments skip submission. Homogeneous six-plane clipping
establishes the stipple origin before perspective division, including rays that
cross from behind the eye. The GPU still clips the original segment and
perspective-interpolates its colour fade.

The pass explicitly uses one-pixel lines, LEQUAL scene-depth testing and no depth
writes or blending. It restores changed state and leaves the camera context
intact. No new shader programs, buffers, textures, or per-frame uploads are added.

## Visual policy and remaining scope

The modern path draws one segment, rather than the compatibility two-vertex
LINE_LOOP that can traverse the ray twice with a continuous stipple counter.
Its mask starts at the clipped visible endpoint, and raster-boundary samples can
differ by a pixel. It also deliberately preserves scene depth, whereas the legacy
HUD targeting path inherits depth writes. These are documented modern rendering
choices, not a claim of exact legacy stipple parity; compatibility is unchanged
and remains the native default. Very large drawing buffers on fragment-mediump
hardware may quantize dash boundaries more coarsely and are not qualified here.

This completes the initial targeting feature slice. Font/text caching, global UI
scope, visual approval/compatibility retirement and broader R4 validation remain.
Windows/macOS runtime qualification and multiplayer robustness remain deferred.

## Validation and reproduction

- Native build and **130/130 GL-enabled CTest tests pass**. New tests cover ray
  transforms, viewport offsets, mask wrapping/nibbles, near-plane clipping,
  hidden/invalid/degenerate and zero-mask cases. Initial sandboxed GL startup
  failed at SDL initialization; the display-enabled rerun passes.
- Ten Linux modern fixture cases pass at 801x601, including odd split heights,
  solid/stippled rays, hidden rays, complete foreground occlusion and clipping.
  All preserve depth, restore nondefault depth/blend/width state and report
  GL_NO_ERROR. PNG captures and `native-fixture.txt` retain the evidence.
- Emscripten Release build/link passes after allowing the SDK cache lock.
- Electron/Chromium gameplay records targeting draws, working HUD meters/icons,
  zero errors/context loss in four WebGL probes, stable buffer uploads (four
  after level entry) and zero texture uploads through firing/idle, plus clean
  shutdown. See `browser-smoke.json` and screenshots. This is a single-browser
  smoke, not a performance or broad device qualification.

From the root:

```sh
cmake -S . -B build
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/targeting-line-2026-10-19/build-fixture.py
cmake -S . -B build-browser
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
```

From `runtime/` with display access:

```sh
TANKGAME_RENDERER=modern ../build/targeting-fixture
```

The fixture writes BMP captures and exits nonzero on assertion failures. Retained
PNGs were converted losslessly using the BMP/PNG helpers in the preceding HUD
slice's `compare.py`. Serve `runtime/` on port 8766, then from the root:

```sh
env -u ELECTRON_RUN_AS_NODE /path/to/electron --no-sandbox documentation/targeting-line-2026-10-19/browser-smoke.cjs
```
