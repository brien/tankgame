# Linkable Emscripten application milestone

## Baseline blocker map

The complete Emscripten link was rerun with the linker error limit disabled
before implementation. The unresolved compatibility API dependencies were:

| Source | Functions containing blockers | Unresolved API family | Startup required? | First-frame required? | Existing browser-safe path |
|---|---|---|---|---|---|
| `GraphicsTask.cpp` | `Start`, `Update` | `gluPerspective`, matrix-stack calls, fixed-function lighting/material and `glShadeModel` | Projection and basic depth/cull state are required; fixed-function state is not | Projection is required | `Matrix4::Perspective` and `RenderContext` already provide explicit CPU matrix state |
| `TextureHandler.cpp` | `TGA_Texture` | `gluBuild2DMipmaps` | Resource preparation currently runs at startup | Textures are not sampled by the current browser shader | WebGL texture upload and `glGenerateMipmap` |
| `BaseRenderer.cpp` | `Setup3DRenderState` and legacy state helpers | `glShadeModel` plus desktop matrix/lighting state reachable from renderers | No | Only the depth/cull/blend subset is useful to migrated paths | Explicit shader state; no lighting replacement exists yet |
| `EffectRenderer.cpp` | `RenderEffect`, `ApplyEffectScale` | matrix stack, fixed-function colors and transforms | No | No | None; effects are deferred |
| `HUDRenderer.cpp` | HUD/menu helpers and projection setup | immediate mode, matrix stack, fixed-function colors/textures, line stipple | No | No | None; UI is deferred |
| `MenuRenderer.cpp` | menu projection and quad helpers | immediate mode, matrix stack and fixed-function colors | No | No | None; UI is deferred |
| `RenderingPipeline.cpp` | global state, lighting, state push/pop | attribute/matrix stacks, `glShadeModel`, lighting/material calls | Basic WebGL state is required | CPU camera matrices are required | `RenderContext` already owns projection/view matrices |
| `TankRenderer.cpp` | tank transforms and enemy geometry helpers | immediate mode, matrix/attribute stacks and fixed-function colors/normals/UVs | No | No for the bootstrap frame | Geometry resources exist, but the unified renderer is not migrated |
| `TerrainRenderer.cpp` | terrain, wall, boundary and water helpers | immediate mode, matrix stack and fixed-function colors/normals/UVs | No | No for the bootstrap frame | None; terrain migration remains a later milestone |

`EnemyTankRendererImpl.cpp` was audited as a known legacy area but did not
appear in the unresolved-symbol set: its Emscripten branches already avoid its
desktop immediate-mode implementation. `BulletRenderer` and `ItemRenderer`
already have browser paths using `Matrix4`, `RenderContext`, and geometry-backed
draw resources.

## Resolution and platform boundaries

- `GraphicsTask` now initializes the browser projection with
  `Matrix4::Perspective`. Its desktop projection, matrix, lighting, material,
  texture-enable, and shading setup remains under the native path unchanged.
- The TGA loader now uses `glTexImage2D` and `glGenerateMipmap` on Emscripten,
  with `GL_CLAMP_TO_EDGE` for WebGL. Native builds retain
  `gluBuild2DMipmaps` and their original wrap behavior.
- Common and pipeline browser state retains only WebGL-supported clear,
  viewport, depth, cull, and blend operations. Fixed-function lighting,
  shading, attribute stacks, and matrix stacks have no browser emulation.
- CMake excludes the desktop implementations of terrain, unified tanks,
  effects, HUD, and menus only for Emscripten. It links
  `BrowserDeferredRenderers.cpp`, whose initialization logs each unavailable
  subsystem. The pipeline explicitly skips their visual submission, and its
  statistics report zero rendered tanks/effects rather than claiming success.
- Simulation and scene extraction are unchanged. Native builds still compile
  and run every original renderer implementation.

## Current browser rendering capability

The linked application contains the WebGL-compatible geometry backend and the
migrated bullet/item submissions. It can establish explicit projection/view
state, clear viewports, and issue geometry-backed `glDrawArrays` calls. Terrain,
tanks (player and enemy), effects, HUD, and menus are intentionally unavailable
in the browser build. Textures can be prepared, but the browser shader does not
sample them yet. This milestone makes no visual-fidelity claim.

## Before the first manual browser rendering test

The next milestone must provide a browser-owned non-blocking main loop, package
and preload runtime assets at their expected virtual paths, and adapt startup
for browser input/focus and audio user-gesture requirements. It should then
exercise context creation, shader compilation, resource loading, and the first
frame in a browser. Renderer migration, shader lighting, texture sampling,
terrain, tanks, effects, and UI remain separate follow-up work and are not
prerequisites for linking this bootstrap target.
