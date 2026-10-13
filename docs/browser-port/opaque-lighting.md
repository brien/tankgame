# Shared opaque lighting, 2026-10-13

The native modern and Emscripten renderer use the same ambient-plus-directional
vertex lighting in the existing shared program. This follows the compatibility
pipeline: world direction `(0.5, 1, 0.5)`, global ambient 0.2 plus light ambient
0.2, diffuse 0.8, and zero material specular/emission. Colour is clamped per vertex
before texture modulation. Alpha is unaffected. All three existing shader dialects
share this implementation; no additional shader/resource owner was introduced.

`BasicMaterial::lit` selects lighting explicitly. Terrain and enemy materials are
lit; the coloured mesh submission used by players/items is lit. Overlay materials
and bullets are unlit. `RenderContext` enables scene lighting at viewport setup,
and defaults to unlit for isolated callers. Normals are transformed in world space
with the inverse transpose of the model's linear transform and normalized in the
vertex shader. The light is world-fixed, so camera movement cannot rotate it.
Singular transforms produce zero normals and ambient-only lighting.

Terrain supplies upward normals, matching compatibility surfaces, walls and
boundaries rather than silently introducing geometrically corrected wall shading.
Floating cubes previously inherited whichever normal the preceding draw left;
both paths now explicitly use the terrain upward normal. This is a deliberate
small compatibility behavior correction. Bullets likewise explicitly disable
compatibility lighting instead of inheriting another mesh's normal, and modern
bullet fills now use the existing additive blend/depth-write behavior. These
changes make the expanded visual comparison reproducible. Modern base-renderer
state setup avoids fixed-function matrix/light save-and-restore, which produced
GL_INVALID_ENUM in the isolated item/bullet comparison. Player mesh draws now
set their counterclockwise front faces explicitly and restore clockwise winding,
rather than depending on the preceding pass.

## Validation

- Native Release and Emscripten Release builds pass.
- **111/111 native CTest tests pass**, including display-enabled GL startup.
  Added tests cover inverse-transpose perpendicularity under rotation/nonuniform
  scale, reflection, singular transforms, and terrain normals across batches.
- Visible Chromium/AMD gameplay passes six GL_NO_ERROR/no-context-loss probes,
  movement, sustained firing, restart and clean shutdown. Buffer uploads remain
  at 26 after level entry. Steady firing samples are approximately 59.95 FPS
  with 0.62–0.69 ms tick CPU time. Screenshot intervals stall; these are local
  smoke measurements, not a controlled benchmark or portable performance promise.
- Linux compatibility and modern gameplay complete movement, jumping, firing, restart and exit 0.

The [evidence directory](../../documentation/opaque-lighting-2026-10-13/README.md)
contains captures, comparison measurements, the six-frame fixture and browser
harness. The fixture exercises levels 48/50, steps, floating geometry, same-level
edits, and player/enemy/item/bullet views at multiple rotations. Every pass checks
GL errors; repeated terrain lookups check stable GPU identity.

Six deterministic comparisons pass: four have at most 1/255 channel differences,
including one exact match. Two poses each differ at one silhouette-edge pixel
(maximum 65/255 and 85/255); remaining differences are at most 1/255. The checked
comparison enforces at most one such edge pixel per pose. These are measured
local tolerances, not a claim of bit-identical rasterization across drivers.

## Reproduction and limits

Build with `cmake --build build -j4`, then run
`TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure` on a working display.
Run `python3 documentation/opaque-lighting-2026-10-13/build-fixture.py` and execute
`../build/lighting-fixture` from `runtime`, once normally and once with
`TANKGAME_RENDERER=modern`. Run the evidence directory's `compare-fixtures.py`.
Serve the browser build's `runtime` directory on port 8766, then run the copied
`browser-smoke.cjs` with standalone Electron as in the terrain milestone.
Native gameplay uses the terrain milestone's `native-smoke.py` with
`TANKGAME_REVIEW_OUT` pointing to this evidence directory.

This completes the practical basic-lighting implementation and expands R3 visual
coverage. Windows/macOS runtime qualification remains deferred, with build/CI
support preserved. No new hosted CI results are claimed. These fixtures do not
cover every level, tank type, context-recovery scenario or driver. Transparent
effects/water, UI/text, audio, browser UX and multiplayer robustness remain outside
this change. Compatibility remains the native default and is not retired.
