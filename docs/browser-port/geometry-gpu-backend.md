# Geometry GPU backend milestone

`DisplayList` remains the renderer-resource facade so existing desktop draw
sites and ownership do not change. On desktop, `SetGeometry` still compiles a
legacy OpenGL display list and `Call` invokes it. Under Emscripten, the same
resource now prepares an interleaved vertex stream, owns a WebGL buffer and a
minimal shader program, and submits it with `glDrawArrays`.

## CPU-to-GPU contract

`PrepareGeometryForGpu` is GL-independent. Positions are always first. Color,
UV, and normal fields follow, in that order, only when their corresponding
`Geometry` flag is set. The resulting offsets and stride are carried with the
prepared stream. No absent vertex attribute is added to the buffer.

Triangles, lines, and line loops map directly to WebGL draw modes. Every CPU
quad is expanded into `(0, 1, 2), (0, 2, 3)` triangles at this backend boundary,
preserving the legacy winding without sending unsupported `GL_QUADS` to WebGL.

## WebGL resource and shader lifetime

Each Emscripten `DisplayList::Implementation` owns one VBO and one linked
program. Shared `DisplayList` copies retain the existing `shared_ptr` ownership
model. The final owner deletes both objects. Shader compilation and program
linking check status and print the driver log before throwing an actionable
error.

The vertex shader has position, color, UV, and normal attributes, an MVP
uniform, a default-color uniform, and a flag selecting vertex versus default
color. UV and normal are carried through the shader. A `BasicMaterial` now supplies
an RGBA multiplier and optional owned `GpuTexture`; the fragment shader explicitly
selects between flat colour and `texture2D(uTexture, vUV) * colour`. Normals still
have no lighting effect. Decoded `ImageData` and portable sampling decisions remain
GL-free, while `GpuTexture` owns upload and deletion on both Linux modern and WebGL.

## Deliberate next boundary

`SetMvpMatrix` and `SetDefaultColor` expose the smallest explicit state inputs
needed by the programmable backend; they default to identity and opaque white.
Existing renderer classes still mutate the desktop fixed-function matrix,
color, texture, lighting, and blending state and do not populate these inputs.
Consequently `Call` now performs real WebGL-compatible GPU submission, but a
correct frame remains blocked on migrating caller-owned camera/model transforms
and draw color before texture, lighting, terrain, UI, or effects work begins.
