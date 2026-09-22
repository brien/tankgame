# Display-list boundary investigation

## Lifecycle and usage map

`DisplayList` is constructed as a member of both `GraphicsTask` and
`ResourceManager`. `GraphicsTask::BuildDisplayLists()` is the builder used by
the active rendering path. It creates the terrain cube, bullet, three tank
mesh variants, two square primitives, and the item mesh after the GL context
exists. `ResourceManager::Initialize()` also builds a duplicate set for the
same categories. Those resources are passed to `RenderingPipeline`, although
the current concrete draw sites still invoke the `GraphicsTask` copies.

Geometry is not defined by `DisplayList`. The two builder functions issue
`glBegin`/`glEnd`, vertex, normal, texture-coordinate, matrix, and scale calls
between `BeginNewList()` and `EndNewList()`. Tank and item builders additionally
call `igtl_QGLMesh::DrawTriangles*()` or `DrawEdgesExtruded()`, whose
implementations emit immediate-mode OpenGL themselves. Thus the old class
merely owned integer OpenGL list names and controlled compilation; it did not
own vertices, normals, texture coordinates, indices, topology, or captured
commands in a backend-neutral form.

The active consumers are `TerrainRenderer`, `PlayerTankRenderer`,
`BulletRenderer`, `EffectRenderer`, and `ItemRenderer`. They establish texture,
lighting, color, blending, and model-view state and apply transforms before
calling `Call(0)`. Consequently the stored geometry intentionally depends on
caller-owned render state and transforms. `LevelHandler`, `TankHandler`, and
`FXHandler` provide simulation/render data but do not create or invoke display
lists directly.

## Boundary decision

The draw sites form a useful **resource invocation** boundary, but the legacy
begin/end compilation API is not a sufficient **geometry creation** boundary.
It captures arbitrary compatibility-profile OpenGL emitted outside the class.
A WebGL backend cannot turn those calls into buffers without either emulating
legacy OpenGL or converting the builders and mesh draw methods to explicit CPU
geometry. Emulation and silent no-op rendering are intentionally rejected.

As the smallest safe step, `DisplayList` is now a facade with an opaque,
shared implementation. OpenGL types and list names live only in the native
implementation, which preserves native compilation and drawing. The
Emscripten implementation owns no fake list identifier and fails loudly if
code tries to compile or draw one. This isolates `glGenLists`, `glNewList`,
`glEndList`, `glCallList`, and `glDeleteLists` from WebAssembly while retaining
the existing call-site boundary for the eventual geometry resource.

This is deliberately a staging boundary, not a WebGL renderer. Immediate-mode
geometry generation remains visible to the Emscripten linker and is expected
to be the next failure. The next slice should define a small CPU geometry
record (position, normal, UV, and primitive topology), convert the square and
cube builders first, and let native upload/compile that record. Mesh extraction
should follow separately; shaders, matrices, lighting, and other renderer
modernization are outside this slice.
