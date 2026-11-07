# Renderer regression baselines

Status: 2026-11-07. These contracts protect modern rendering without executing
compatibility rendering. They support retirement step 3; they do not grant visual
owner approval or close the remaining platform qualification gaps.

## Preserved reference evidence

The [Linux qualification captures](../../documentation/modern-default-2026-11-07/README.md)
record revision `e6784e6`: title, movement, menu return, co-op and the known failed
single-player transition for each backend, plus deterministic effect frames 0/5.
The recorded six-frame effect comparison passed the existing tolerance. These are
reviewed evidence, **not owner-approved universal golden images**. Do not regenerate
or silently replace them when rendering changes; capture new evidence separately
and explain the intended difference. The retained split-screen failure is a bug
reference, not desired behavior to encode as a passing regression assertion.

## Executable contracts

| Area | Tests | Protected behavior |
| --- | --- | --- |
| Enemy geometry and pose | `tests/test_enemy_renderer.cpp` | Vertex counts, bounds, winding, normals, body/upper-assembly scales, relative aiming and cannon offsets. Expected coordinates are constants, not output from the compatibility renderer. |
| Terrain and water | `tests/test_terrain_geometry.cpp` | Surface/wall/boundary geometry, texture coordinates, winding, water height/ribbons and cache invalidation. |
| Meshes and transforms | `tests/test_geometry.cpp`, `tests/test_geometry_buffer.cpp`, `tests/test_matrix4.cpp` | Mesh attributes, extrusion/edges, topology conversion and camera/model composition. |
| HUD, text and viewport data | `tests/test_hud_layout.cpp`, `tests/test_font_atlas.cpp`, `tests/test_viewport_manager.cpp`, `tests/test_scene_data.cpp` | Player identity, meter bounds/visibility, font layout, viewport seams and snapshot ownership. These are not full UI pixel comparisons. |
| Effects | `tests/test_effect_renderer.cpp` | All effect types' geometry scales, texture selection and rotation order. |
| Modern effect pixels/depth | `tests/test_modern_effect_rendering.cpp` | Every effect visibly draws; non-outline additive passes preserve depth; output blend/depth/cull state is valid; opaque objects occlude effects; split-screen rendering leaves the other view untouched. |
| Core-profile submission | `tests/test_video_startup.cpp` | Visible pixels after VAO binding changes, renderer recreation, texture upload and resize. |

The modern effect GL tests promote the recorded fixture's behavioral checks to
GoogleTest. They use a 320×240 region and compare before/after pixels and depth in
the same context. They require visible output where appropriate, so skipping all
draws cannot pass. They do not require cross-driver pixel equality or access to a
legacy implementation. Types 0/6 intentionally write outline depth; all other
effect types must leave it untouched.

## Running and CI

```sh
cmake --build build --parallel 4
env -u TANKGAME_RENDERER TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
```

CTest runs game tests from `runtime` so production resources resolve. Without
`TANKGAME_TEST_GL=1`, graphics tests explicitly skip; CPU contracts still run.
Modern-effect tests also skip when compatibility is explicitly selected.

Linux CI now installs Xvfb and runs the suite with software OpenGL, dummy audio,
GL tests enabled and the renderer variable unset. This makes modern rendering the
exercised default. Windows/macOS CI still provides build and CPU-test coverage;
it does not claim graphics qualification. The new hosted workflow has not been
run from this local session.

## Local validation

The Release build and all 160 tests pass with `TANKGAME_TEST_GL=1`,
`LIBGL_ALWAYS_SOFTWARE=1`, dummy audio and the renderer variable unset. This run
used the available Linux display; Xvfb is not installed locally, so the hosted
Xvfb wrapper itself remains unverified. Both new modern-effect tests passed.
`git diff --check` passes. Windows/macOS and browser runtime were not rerun.

## Remaining retirement gates

Windows/macOS modern-default runtime, physical firing input, broader level/UI
visual coverage and owner visual approval remain open. The known co-op-to-single-
player session defect is deferred. The new regression contracts supplement those
gates; compatibility removal remains a separate implementation decision.
