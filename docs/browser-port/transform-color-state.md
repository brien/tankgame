# Explicit geometry transform and color state

The browser geometry backend now receives draw state through a small CPU-side
`RenderContext`. `Matrix4` stores column-major values and uses column vectors,
matching OpenGL and GLSL. Multiplication is post-multiplication: a legacy call
sequence `Translate; RotateX; RotateY; Scale` is represented as
`T * Rx * Ry * S`, and therefore applies scale first when transforming a
vertex. Each submitted matrix is `projection * view * model`.

For Emscripten, `RenderingPipeline` constructs the 45-degree perspective matrix
with the existing per-viewport aspect ratio and 0.1/1000 near/far planes. It
also constructs the view matrix from the existing camera position, focus, and
world-up vector. Native builds retain `gluPerspective`, `gluLookAt`, and the
fixed-function matrix stack.

The geometry-backed browser paths migrated in this slice are player tank body
and turret resources (including their existing geometry-based overlays), bullet
pieces, and items. Their legacy transform call order is reproduced explicitly,
and every call supplies its fallback RGBA color. Geometry with vertex colors
continues to take priority in the WebGL shader through `uHasColor`.

The browser pipeline now dispatches alive player records to that migrated path
and filters out enemy records. It draws the opaque body and turret; the player
model has no separate cannon draw in the existing renderer. Its additive mesh
overlay and textured targeting indicators remain deferred with effects/UI.
Enemy tanks remain immediate-mode geometry and are not migrated. Terrain,
standalone effects, sky, HUD, and menus also retain their fixed-function paths.
Those boundaries intentionally remain visible for later renderer milestones.
