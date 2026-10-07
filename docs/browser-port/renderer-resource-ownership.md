# Renderer resource and submission ownership catalogue

**Status:** renderer-convergence milestone 4, 2026-09-29


**2026-10-07 follow-up:** the [shared enemy milestone](enemy-modern-renderer.md)
adds enemy bodies/housings/cannons through the existing catalogue and program.
The owner reports Windows runs correctly (renderer unspecified) and explicitly
defers macOS runtime validation while authorizing this narrow migration. Existing
macOS build support and desktop CI checks remain intact. Earlier evidence and
recommendations below are historical; they do not establish Windows modern
qualification or current macOS runtime success.

## Verified previous model

The milestone began with `ResourceManager` already authoritative for migrated CPU
geometry, textures, and materials. The remaining coupling was verified in code:
each of its 15 `DisplayList` catalogue slots could construct an identical linked
program when initialized in modern mode. The facade also owned its VBO, topology,
layout, vertex count, MVP array, default colour, and current texture. Every draw
called `glGetUniformLocation` for `uMvp`, `uDefaultColor`, `uHasColor`,
`uHasTexture`, and `uTexture`, then configured attributes and submitted the draw.
Thus program creation happened per geometry, uniform discovery happened per draw,
and `SetMvpMatrix` / `SetMaterial` / `Call` made geometry carry mutable draw state.

The VBO data was already derived cleanly by the GL-independent
`PrepareGeometryForGpu`: optional position/colour/UV/normal semantics and stride
were explicit, QUADS were triangulated, and TRIANGLES, LINES, and LINE_LOOP were
preserved. This made separation possible without changing the vertex format or
adding index buffers.

## New ownership boundary

The modern flow is:

```
asset / QGLMesh -> CPU Geometry (ResourceManager catalogue)
                 -> GpuGeometry (VBO + immutable metadata)
                 -> RenderContext (MVP + BasicMaterial)
                 -> ModernRenderer (one ShaderProgram + draw submission)
                 -> glDrawArrays
```

Responsibilities are deliberately narrow:

- `Geometry` and QGLMesh extraction remain backend-neutral CPU data.
- `GpuGeometry` owns the uploaded VBO, attribute layout, topology, and vertex
  count. It owns no shader, texture, material, or transform.
- `ModernRenderer::ShaderProgram` compiles/links the existing simple shader,
  fixes attribute locations, caches all uniform locations, reports compile/link
  logs, and deletes the program.
- `ModernRenderer::Draw` binds that one program, configures the selected
  geometry, binds the draw's material/texture and MVP, and issues `glDrawArrays`.
- `RenderContext::Draw` is the explicit call-site boundary. It computes the MVP
  and supplies a value `BasicMaterial`; it no longer mutates a geometry resource.

The one renderer/program instance is owned by `ResourceManager` and services all
migrated body, turret, bullet, item, horizontal quad/outline, and ring/star draws.
Program identity therefore does not vary by geometry. Program construction,
linking and uniform discovery occur once during catalogue initialization, not per
resource or frame. VBO upload and texture upload likewise remain initialization
work; draw submission performs no large allocation.

## `DisplayList` compatibility facade

A repository-wide rename would create churn unrelated to this milestone, so
`DisplayList` remains a temporary facade. In modern mode it retains CPU geometry
and a shared `GpuGeometry` and exposes the latter to `RenderContext`; it contains
no program or mutable material/transform state. `Call` deliberately throws in
modern mode so new code cannot recreate the old miniature-renderer API.

On native compatibility mode, the same facade lazily compiles and calls the
existing OpenGL display-list name. `BeginNewList`, `NextNewList`, `EndNewList`,
`NewList`, `EndList`, and the fixed-function geometry loop remain behind desktop
exclusions. The legacy renderer was not modernized or removed. Emscripten cannot
compile those symbols.

## Catalogue and lifecycle

The GL context is current before `GraphicsTask` initializes `ResourceManager`.
In modern mode initialization creates the sole `ModernRenderer` first and attaches
it non-owningly to `RenderContext`; textures, meshes, and each `GpuGeometry` are
then prepared. Catalogue lookup continues returning the same facade for a typed
resource identity, and `GraphicsTask` owns no competing mesh, program, or texture
catalogue.

Shutdown order is explicit:

1. `GraphicsTask` cleans and destroys the rendering pipeline, ending draw users.
2. `ResourceManager::CleanupDisplayLists` closes all facades, deleting modern
   VBOs (or native compatibility lists) while the context is current.
3. `RenderContext` drops its non-owning renderer pointer.
4. `ResourceManager` destroys the shared program.
5. Catalogue texture/material state is destroyed with the manager.
6. `VideoTask` later destroys the GL context.

No shared pointer to `GpuGeometry` is returned from the catalogue, and the
renderer pointer does not own or prolong the catalogue. This prevents resources
from surviving context teardown.

## Platform conditionals and portability

In `DisplayList.cpp`, `__EMSCRIPTEN__` directives decreased from 10 to 9. Those
nine remaining directives only exclude desktop compatibility-list operations.
`ModernRenderer.cpp` has one directive for the GLES precision versus desktop
GLSL 1.20 preamble, so the combined count for these refactored backend files is
unchanged at 10 while responsibility is better isolated. `PlatformGL.h` retains
one branch selecting GLES2, Windows, macOS, or Linux headers. There is no native
modern versus WebGL submission fork: both compile `GpuGeometry`,
`ModernRenderer`, and `RenderContext`.

macOS requires a supported core-profile context and a core GLSL shader spelling;
central program ownership provides one place to add it. Compatibility display
lists and the deferred fixed-function feature renderers remain deprecated APIs
outside the migrated modern slice. Windows must load buffer, shader/program,
attribute/uniform, active-texture, and mipmap entry points at runtime (for example
through `SDL_GL_GetProcAddress`); submission centralization now gives that loader
a small, explicit integration surface. Neither platform is claimed validated.

## Validation and boundaries

A native Release configure/build and all 86 tests passed. Xvfb runtime smokes for both default legacy and
`TANKGAME_RENDERER=modern` reached the native task loop without shader or GL
errors. They were stopped by a timeout at the title scene, so migrated gameplay
visuals and graceful shutdown were not freshly validated. The Emscripten
Release target configured, compiled, linked, and generated its HTML, JavaScript,
WebAssembly, and preload data artifacts, proving the same submission sources and
asset packaging build without compatibility GL symbols.

For local browser validation, serve `runtime/` over HTTP, open
`tankgame-linux.html`, start one-player gameplay, verify body/turret and mouse aim,
fire continuously, observe item and ring/star rendering, inspect the console for
WebGL errors, check approximately 60 FPS, and exercise shutdown. For native, run
both `./tankgame-linux` and `TANKGAME_RENDERER=modern ./tankgame-linux` from
`runtime/` and repeat the same migrated-slice checks.

No terrain, enemy, effect, HUD/menu, text, lighting, audio, browser UX, or session
feature was migrated. The recommended next convergence milestone is portable GL
function loading plus a core-profile shader/context variant and validation on one
additional desktop platform, not another visual category.
