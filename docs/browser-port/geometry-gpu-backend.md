# Geometry GPU backend milestone

`DisplayList` remains a transitional renderer-resource facade so compatibility
draw sites do not require a broad rename. On desktop compatibility mode,
`SetGeometry` compiles a legacy OpenGL display list and `Call` invokes it. In
modern mode it owns a `GpuGeometry`; the shared `ModernRenderer` submits that
geometry with `glDrawArrays`.

## CPU-to-GPU contract

`PrepareGeometryForGpu` is GL-independent. Positions are always first. Color,
UV, and normal fields follow, in that order, only when their corresponding
`Geometry` flag is set. The resulting offsets and stride are carried with the
prepared stream. No absent vertex attribute is added to the buffer.

Triangles, lines, and line loops map directly to WebGL draw modes. Every CPU
quad is expanded into `(0, 1, 2), (0, 2, 3)` triangles at this backend boundary,
preserving the legacy winding without sending unsupported `GL_QUADS` to WebGL.

## Modern resource and shader lifetime

Each modern facade retains one `GpuGeometry`, which owns only its VBO and
immutable layout/topology/count metadata.  `ResourceManager` owns one
`ModernRenderer` and therefore one linked program for the entire migrated
catalogue. Shader compilation and program linking check status and print the
driver log before throwing an actionable error; attribute bindings and uniform
locations are cached once after link.

The vertex shader has position, color, UV, and normal attributes, an MVP
uniform, a default-color uniform, and a flag selecting vertex versus default
color. UV and normal are carried through the shader. A `BasicMaterial` now supplies
an RGBA multiplier and optional owned `GpuTexture`; the fragment shader explicitly
selects between flat colour and `texture2D(uTexture, vUV) * colour`. Normals still
have no lighting effect. Decoded `ImageData` and portable sampling decisions remain
GL-free, while `GpuTexture` owns upload and deletion on both Linux modern and WebGL.

## Explicit submission boundary

`RenderContext::Draw` computes the MVP and passes it with a `BasicMaterial` and
the selected `GpuGeometry` to `ModernRenderer::Draw`.  Geometry contains no
persistent transform, colour, texture, or program state. `DisplayList::Call` is
now compatibility-only and fails explicitly in modern mode. Existing deferred
feature categories continue to use compatibility fixed-function state and are
outside this boundary.
