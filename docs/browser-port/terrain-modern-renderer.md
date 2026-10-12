# Shared opaque terrain (2026-10-09–12)

The owner deferred Windows modern-renderer runtime qualification alongside macOS
and authorized the next terrain slice. Both platforms' source/build support and
existing desktop CI remain intact; no new hosted-CI or Windows/macOS runtime
result is claimed.

## Implementation

`RenderingPipeline -> TerrainRenderer -> ResourceManager -> RenderContext ->
ModernRenderer` now draws opaque terrain on modern desktop and Emscripten.
Compatibility remains the native default and visual reference. The browser
terrain stub and modern terrain skip have been removed; CMake compiles the same
terrain source for both targets, excluding only the retained compatibility calls.

`TerrainGeometry` builds GL-free quad batches for floating blocks, black/white/
checker surfaces, stepped walls, and boundary walls/ceilings. Existing geometry
preparation triangulates the quads for the programmable backend. The batches use
vertex colours, existing textures, a white `BasicMaterial`, and the existing
single shader program. No assets or preloads were added.

The conversion preserves the **active** compatibility contract, including:

- Metadata default/block colours, height-dependent texture selection and UV repeat.
- Surface strips' historical start/end offsets, omission of non-final negative
  strips, and height >=25 cutoff. It does not repair level topology.
- All four wall scan directions and their transition-dependent colours/UVs.
- Floating cube placement and counterclockwise winding; clockwise surface/wall
  winding; level 48's lowered, counterclockwise boundaries and omitted ceiling.
- Existing rectangular-boundary coordinate quirks and ignored draw flags.

The modern pass explicitly enables depth testing/writes, `GL_LESS`, back-face
culling, and disables blending. Its batch winding is explicit and ends at `GL_CW`
before entity rendering. Water is not submitted by the modern pass.

`ResourceManager` owns terrain CPU/GPU resources. The cache compares active map
cells, dimensions, level number and the two used colours, so it catches in-place
edits and reloads with the same level id. Unchanged frames and additional viewports
perform a bounded content comparison without mesh construction or GPU upload.
Changed content builds/uploads replacement batches, releasing old VBOs; upload
failure invalidates the CPU cache so a subsequent attempt can retry. Cleanup
releases terrain buffers before the shared program/context. No new program owner
or per-cell draw calls were introduced; at most six batches are submitted per view.

## Validation

Native Release configure/build passed. All **108/108 CTest tests passed**, including
the existing display opt-in; the modern startup test also passed in a separate
process. Ten new headless tests cover strip endpoints/UVs/winding, texture choices,
height/negative-strip behavior, four stepped wall faces, negative-base colours,
floating cube bounds, boundary variants, cache invalidation/reuse, and map bounds.

Both Linux renderer modes completed single-player movement, camera/turret input,
jumping, more than ten seconds of firing, menu/restart, and clean shutdown.
Captures show the terrain and retained player/entities. Logs contain no recorded
GL/shader errors. This is bounded single-player evidence, not multiplayer sign-off.

The standalone fixture uses production terrain renderers/catalogue, with lighting
disabled in compatibility to isolate this unlit migration. It exercises level 48,
level 50, floating blocks, stepped surfaces, and same-level content edits in three
frames. Both modes return `GL_NO_ERROR` in every frame. All three pairs have
**zero silhouette differences**; the largest RGB-channel difference is **1/255**.
The fixture also verifies stable GPU geometry identity on repeated modern lookups.
It does not establish lighting or water parity, exhaustive level coverage, or
whole-game pixel parity.

Emscripten **3.1.64 Release** compiled and packaged HTML/JS/WASM/data using the
installed SDK's ordinary configuration, with no SDL2 or HTML-minifier adapters.
Visible Chromium 152 / Electron 44 on the AMD GPU completed movement, sustained
firing, restart and clean shutdown with six zero-error/no-context-loss probes.
Buffer uploads remained at 26 from initial level entry through gameplay and the
identical-level restart. Steady firing intervals were near 59.95 FPS; screenshot
intervals stalled, so this is smoke evidence rather than a controlled benchmark.
Full runtime results are recorded in the evidence README.

## Reproduction

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
TANKGAME_TEST_GL=1 TANKGAME_RENDERER=modern ./runtime/tankgame_tests --gtest_filter=VideoStartupTest.*
python3 documentation/terrain-renderer-2026-10-09/build-fixture.py
cd runtime
../build/terrain-fixture
TANKGAME_RENDERER=modern ../build/terrain-fixture
cd ..
python3 documentation/terrain-renderer-2026-10-09/compare-fixtures.py
```

The GL tests and fixture require a working display. The fixture helper links
objects from the existing Unix Makefiles native build. `native-smoke.py modern`
and `native-smoke.py compatibility` in the evidence directory use X11/XTest to
replay the gameplay sequence and capture the game window.

With the installed SDK activated (`EM_CACHE` set to its writable existing cache
where necessary):

```sh
emcmake cmake -S . -B build-browser -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
```

Run the evidence directory's `browser-smoke.cjs` with a standalone Electron
executable to replay its visible Chromium/WebGL check. Tooling is local and not a
new game dependency. See the [evidence README](../../documentation/terrain-renderer-2026-10-09/README.md).

## Remaining scope

This completes the first opaque terrain slice, **not all of R3**. Modern lighting,
water/transparent effects, HUD/menu/text, browser audio/interaction/lifecycle work,
full asset coverage and compatibility retirement remain separate. The historical
multiplayer failures remain deferred and visible in the risk register. Windows
and macOS runtime qualification remain explicitly deferred; later platform checks
should include this terrain slice.
