# Renderer resource ownership catalogue

**Status:** renderer-convergence milestone 3, 2026-09-29

## Audit and decision

Before this milestone, `GraphicsTask` loaded all 26 TGAs, loaded four GSM meshes,
extracted the migrated geometry, and constructed its `DisplayList` resources.
It then constructed a `ResourceManager`, which repeated the texture uploads, tried
to load those same GSM files through the OBJ loader, and built a second resource
set. Draw code used the `GraphicsTask` set, so the object named resource manager
was not authoritative. `BasicMaterial` values for ring/star textures were also
assembled at each draw. Both owners used RAII internally, but their overlapping
lifetimes doubled uploads and made teardown responsibility unclear.

`TextureHandler` decoded TGA data directly into a temporary `ImageData`, uploaded
one `GpuTexture`, and retained both the owning object and a copied raw GL name.
The raw name was used by compatibility rendering. `DisplayList` owned CPU
`Geometry`, a modern VBO and shader program, and (on desktop) a compatibility
display-list name. Shader programs remain per geometry resource; they were not
duplicated between the two old catalogues, and consolidating them is deliberately
deferred until the GPU backend is separated from the historical `DisplayList`
facade.

We chose **Option A: make `ResourceManager` authoritative**. It was already
injected into `RenderingPipeline` and already described the exact migrated
resource set. Repairing its GSM loader and eliminating the shadow members was
smaller and clearer than introducing a third asset manager. It is now the sole
catalogue for player body/turret, bullet/quad geometry, items, ring/star materials,
and the texture set. Typed `GeometryResource` and `MaterialResource` lookups replace
direct access to the migrated `GraphicsTask` members and fail explicitly for an
unknown identifier.

## Ownership and lookup flow

The flow is now:

1. After SDL has made the GL context current, `GraphicsTask` creates and initializes
   one `ResourceManager`.
2. `ResourceManager` loads each selected GSM once, creates CPU `Geometry` once,
   and assigns it to one catalogue-owned `DisplayList` facade.
3. Its sole `TextureHandler` decodes each TGA once into shared, immutable
   `ImageData`, then uploads one shared `GpuTexture`.
4. Catalogue-owned ring and star `BasicMaterial` templates retain the appropriate
   texture. Draws copy the small value object only to apply per-player colour;
   the texture ownership remains shared and stable.
5. Linux-modern and Emscripten-modern use identical typed lookups. There is no
   platform-specific registry.

CPU meshes and extracted geometry are owned by `ResourceManager` and its
`DisplayList` values. Decoded images and GPU textures are owned by
`TextureHandler`; callers receive `shared_ptr<const ...>` views. Materials are
owned as catalogue values. Modern VBOs and the current per-resource shader program
are RAII state inside `DisplayList`; GL handles do not cross its interface.

## Compatibility bridge

The legacy renderer consumes the same catalogue. `DisplayList::SetGeometry`
creates its desktop compatibility display list lazily from the shared CPU
geometry. `TextureHandler::GetTextureArray()` remains the explicitly transitional
raw-name bridge and contains the name of the same `GpuTexture`; it does not upload
a second texture. Modern code uses `GpuTexture` and typed material lookups and does
not receive raw GL names. Terrain, enemy, effect, and HUD compatibility consumers
still use bridge accessors, but no modern version of those features was added.

## Initialization and shutdown

Catalogue construction is CPU-only. GPU allocation begins only in `Initialize`,
after context setup and (for Emscripten) after preloaded files are available.
Shutdown cleans the rendering pipeline first, then calls catalogue cleanup and
destroys the `ResourceManager` while the context is still current. This releases
materials, textures, VBOs, programs, and compatibility lists before `VideoTask`
tears down the context. No catalogue `shared_ptr` is retained by simulation code
or by a renderer after pipeline cleanup.

Initialization failure is explicit for a missing or undecodable GSM. The existing
TGA loader continues logging individual missing textures because native packaging
historically permits optional texture failures.

## Conditional and platform audit

Across the affected ownership/resource and migrated-renderer files,
`__EMSCRIPTEN__` directives decreased from 30 to 29. The removed branch was an
obsolete projection fallback made unnecessary by the shared modern mode decision.
Remaining conditionals do not select a different catalogue: they isolate desktop
fixed-function/display-list calls, deferred compatibility-only renderers, and the
small desktop GLSL 1.20 versus GLES precision preamble.

On macOS, catalogue teardown must continue while the correct context is current.
The ownership model adds no macOS-specific path, but the modern shader still needs
a core-profile dialect before modern macOS can be claimed. On Windows, catalogue
initialization/destruction likewise requires a current context; VBO, shader,
active-texture, and mipmap functions still require an explicit loader because the
system headers expose only OpenGL 1.1 entry points. Neither platform was built or
run for this milestone.

## Boundaries and next milestone

The retained `GraphicsTask` font belongs to the deferred compatibility HUD/text
path; the unused duplicate font in `ResourceManager` was removed. Terrain, enemy
tanks, effects, HUD/menu, lighting, audio, and multiplayer behavior were not
migrated. Compatibility bridge calls in those systems remain visible technical
debt rather than new modern resources.

The recommended next convergence milestone is to extract shared shader/program
ownership and modern draw submission from `DisplayList` into a narrow GPU backend,
while keeping this catalogue and its typed resource identities unchanged. Do not
start terrain or enemy migration until that boundary has native fixtures and
cross-platform function-loading support.
