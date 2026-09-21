# Graphics and rendering audit

## Conclusion

The active renderer is an OpenGL 2.1 compatibility renderer even though it has recently been reorganized into data extractors and renderer classes. WebGL exposes an OpenGL ES-style programmable API: it has no compatibility profile, GLU, immediate mode, display lists, matrix stack, fixed-function lighting/materials, texture enable state, line stipple, or fixed-function texture environment. Emscripten has legacy emulation options, but relying on them would preserve implicit state, add overhead, leave GLU/display-list gaps, and provide a poor cross-platform end state. It can be useful only for a throwaway diagnostic, not the recommended architecture.

The data boundary around `SceneData` is the strongest leverage point. Keep it, make it ownership-safe, and replace everything downstream that assumes legacy GL with explicit shader/VBO passes shared across browser and desktop.

## Complete source-location inventory

The repository-wide search found active `gl*` calls in these production files (example-only code excluded from architectural scope but should not be compiled):

| Area | Files | Character of use |
|---|---|---|
| Root graphics | `GraphicsTask.cpp` | Context state, projection/model-view matrices, lighting, camera, HUD/menu/text quads, blending, and display-list geometry. |
| Resources | `TextureHandler.cpp`, `DisplayList.cpp`, `igtl_qmesh.cpp` | Texture creation/GLU mipmaps, list compile/call/delete, and immediate mesh triangles/edges. |
| Pipeline/state | `rendering/BaseRenderer.cpp`, `RenderingPipeline.cpp`, `ViewportManager.cpp`, `IRenderer.cpp` | Global state push/pop, matrices, lighting, error checks, viewport. |
| Scene geometry | `TerrainRenderer.cpp`, `TankRenderer.cpp`, `PlayerTankRenderer.cpp`, `PlayerTankRendererImpl.cpp`, `EnemyTankRendererImpl.cpp`, `BulletRenderer.cpp`, `EffectRenderer.cpp`, `ItemRenderer.cpp` | Immediate vertices/UVs/normals, transforms, colors, display-list calls and blend/depth state. |
| UI | `HUDRenderer.cpp`, `MenuRenderer.cpp` | Matrix/state changes, immediate quads/lines, textures and blending. |
| Resource duplication | `rendering/ResourceManager.cpp` | Textures/fonts/meshes plus compilation of immediate geometry into display lists. |

`HUDDataExtractor.cpp` contains a name matching the broad `gl*(` search but is not a substantive graphics API user; likewise comments/examples must not be conflated with active calls. Conversely, `Tank.cpp` and `TankHandler.cpp` include platform GL headers without active GL calls. Header dependencies should eventually be removed, but gameplay is not drawing geometry.

Platform OpenGL/GLU headers occur in `main.cpp`, `Tank.cpp`, `TankHandler.cpp`, `GraphicsTask.cpp`, `DisplayList.cpp`, `TextureHandler.cpp`, `igtl_qmesh.cpp`, and many `src/rendering` implementation files. GLU calls are concentrated in `GraphicsTask.cpp` (`gluPerspective`, `gluLookAt`) and `TextureHandler.cpp` (`gluBuild2DMipmaps`).

A mechanical audit counted the dominant patterns at the time of analysis: hundreds each of `glVertex3f`/`glTexCoord2f`, many `glVertex3i`, 80+ `glBegin`/`glEnd` pairs, 100+ enables, dozens of matrix pushes/pops/transforms, 17-ish list-ending/creation patterns, and fixed lighting/material calls. Counts are evidence of migration breadth, not an estimate or acceptance criterion; generated comments and duplicate renderer paths make exact line counts less meaningful than runtime pass coverage.

## Feature audit and WebGL disposition

### Headers and GLU

- Desktop headers are selected by preprocessor (`<GL/gl.h>`, `<OpenGL/gl.h>`, `windows.h`). Emscripten must instead compile against GLES/WebGL headers via a backend/platform header with no GL types escaping public engine interfaces.
- `gluPerspective` and `gluLookAt` build fixed-function matrices; GLU is not a WebGL API. Replace with C++ matrix functions and shader uniforms.
- `gluBuild2DMipmaps` both resizes/uploads/builds mip chains. Replace with validated dimensions/formats, `glTexImage2D`, then `glGenerateMipmap`, or offline mip levels. WebGL 1 restrictions on NPOT wrap/mipmap filters must be observed.

### Immediate mode: `glBegin`/`glEnd`, `glVertex*`, `glTexCoord*`, normals/colors

These appear in mesh drawing, terrain cells/walls/water, procedural tanks, bullets, effects, items, targeting UI, HUD/menu, text quads, and display-list builders. WebGL requires vertex data in buffers and draws via `glDrawArrays`/`glDrawElements`; per-vertex normal, UV, and color become attributes. Dynamic effects/UI can use streamed batches; static meshes/terrain should use persistent buffers. Integer terrain positions should be converted to explicit vertex formats rather than depending on `glVertex3i` conversion.

### Matrix stack and transforms

`glMatrixMode`, `glLoadIdentity`, `glPushMatrix`/`glPopMatrix`, `glTranslate*`, `glRotate*`, `glScale*`, and GLU camera/projection calls are pervasive. WebGL shaders receive model/view/projection matrices; there is no implicit stack. Introduce tested matrix math and explicit transform composition preserving current order, handedness, clockwise front faces, degree conventions, and coordinate system. A small CPU transform stack inside a compatibility-oriented backend may reduce first-pass visual risk, but it should emit matrices/uniforms, not emulate all fixed-function state globally.

### Fixed-function lighting/materials

`GraphicsTask::Start()` configures `GL_LIGHT0`, local viewer, attenuation, normalization and color material. Pipeline/renderers toggle `GL_LIGHTING`, `GL_NORMALIZE`, `GL_COLOR_MATERIAL`, and set normals/colors. WebGL needs shaders reproducing ambient/diffuse/color behavior, normal transformation, light direction/position and attenuation. Exact legacy lighting is visually sensitive; establish reference captures per level/tank/effect. Normal normalization can be in shader or assured by data/transforms.

### State assumptions (`glEnable`/`glDisable`)

Renderers mutate depth test, culling, texturing, blending, lighting, color material, line stipple and sometimes depth writes. `BaseRenderer` attempts push/pop-style cleanup, but state remains global and pass correctness depends on ordering. WebGL retains explicit depth/cull/blend/scissor state, but not texture/lighting “enable” switches. Replace with pass descriptors/material pipelines and a state cache that sets every relevant state at pass entry. Add debug validation so one effect cannot leak blend/depth/cull state into the next.

### Display lists

`DisplayList` wraps `glGenLists`, `glNewList`, `glEndList`, `glCallList`, and `glDeleteLists`. Both `GraphicsTask::BuildDisplayLists()` and `ResourceManager` compile cubes, bullets, tanks, item meshes and squares. Display lists do not exist in GLES/WebGL. Replace each list with a mesh/resource handle referencing vertex/index buffers; transforms/materials stay per draw. Avoid retaining `DisplayList` as a browser emulation interface because it encodes compile-time capture of arbitrary GL state rather than data.

### Textures and pixel formats

`TextureHandler::Load_TGA` parses TGA bytes, swaps BGR to RGB, chooses `GL_RGB`/`GL_RGBA`, calls GLU mipmap building, and uses `GL_CLAMP` or `GL_REPEAT`. Required work:

- retain decoder behind an image result (`width`, `height`, channels, pixels) or use one shared portable decoder;
- guarantee allocation/deallocation pairing and failure cleanup (the current code mixes `new[]` with `free`, an undefined-behavior risk to document/validate);
- set `GL_UNPACK_ALIGNMENT` appropriately for 3-channel row widths;
- use WebGL-supported sized/unsized format combinations for chosen WebGL version;
- use `GL_CLAMP_TO_EDGE`, not legacy `GL_CLAMP`;
- only use repeat/mipmaps on WebGL 1 NPOT textures if dimensions satisfy restrictions, or resize/choose WebGL 2;
- create textures once, return backend-neutral handles, and delete them explicitly.

### Meshes/models

`igtl_QGLMesh` usefully stores vertices (position/normal/UV), triangles, edges and GSM loading. Its drawing methods loop data into immediate GL. Keep parsing and CPU processing; add read-only contiguous data extraction or conversion into renderer mesh data in future implementation. Upload indexed triangles once. Edge/debug drawing becomes a separate line buffer. Verify winding: current renderer declares `GL_CW`, while many modern examples assume CCW.

`ResourceManager::PrepareMesh()` invokes `LoadOBJ()` for `.gsm` file names, unlike `GraphicsTask::PrepareMesh()` which uses `LoadGSM()`. Determine which path is actually producing visible models and lock expected output before migrating.

### Camera and projection

`CameraManager` computes camera state, `SceneDataBuilder` currently copies camera data partly through `GraphicsTask::cams`, and `RenderingPipeline`/legacy code applies `gluLookAt` and perspective. Move view/projection calculation to portable math using camera DTOs and current framebuffer/viewport aspect. One camera per split-screen player needs its own viewport and projection; never use full-canvas aspect for a half-width/half-height view unless deliberately matching current behavior.

### HUD/menu/text

HUD/menu use immediate lines/quads, texture IDs, alpha/additive blending, fixed normalized positions, and projection/model-view manipulation. `GraphicsTask::RenderText` uses SDL_ttf-created surfaces/textures with immediate textured quads; newer HUD code has text hooks/resource access but remains GL-bound. The modern path should:

- render UI in an explicit orthographic pass after 3D for each view or once globally as design requires;
- batch solid/textured quads and lines;
- use a font atlas or cached SDL_ttf glyph textures rather than per-frame texture creation;
- preserve alpha/additive blend modes and depth/cull disable state explicitly;
- distinguish player-local HUD from global menu/debug overlays in `SceneData`.

### Blending, depth, and effects

The code uses standard alpha (`SRC_ALPHA`, `ONE_MINUS_SRC_ALPHA`) and additive (`ONE`, `ONE`) blending, plus depth-mask/depth-test changes. WebGL supports these operations. The incompatibility is not blending itself but implicit ordering/state leakage and possibly alpha conventions. Define opaque terrain/tanks, alpha/additive effects, and UI passes, sort transparent elements as needed, and set depth write/test per pass.

### Viewport and split screen

`ViewportManager` describes viewports and calls `glViewport`; `RenderingPipeline::RenderAllPlayerViews` renders each player. `glViewport` is supported by WebGL. Add `glScissor` clearing per viewport if required, compute projection aspect from each viewport, and respond to canvas drawing-buffer resize/high DPI. Preserve the same logic for all native targets.

## Active vs legacy duplication

`GraphicsTask` retains large `DrawHUD`, `DrawMenu`, `RenderText`, `BuildDisplayLists`, and other legacy functions while `RenderWithNewPipeline()` is the active update path. Separately, tank geometry is duplicated among `TankRenderer`, `EnemyTankRendererImpl`, and player renderer variants. Before migration:

1. instrument/call-graph test which functions/resources execute in title, one-player, two-player, versus, all tank types, every effect/item, and debug modes;
2. define visual fixtures for those paths;
3. migrate behavior, not blindly every dead duplicate;
4. do not delete old code until native visual parity and browser coverage prove it unused.

This discovery phase is documentation/testing work and does not justify choosing legacy emulation.

## Architecture options

### Option A — separate renderer implementations

**Shape:** keep `SceneDataBuilder` and common render DTOs; introduce a backend interface. The desktop backend wraps/refactors current compatibility GL. A browser backend uses shaders/VBOs/WebGL. Both are C++.

- **Reuse:** simulation and render extraction are shared. The desktop visual code can initially remain largely intact; browser can reuse geometry definitions after they are represented as data rather than GL statements.
- **Change:** establish strict boundary, disentangle global resource IDs/state, implement a complete second renderer, and add backend-neutral mesh/image/font resources.
- **Linux/macOS/Windows:** select the compatibility backend under native SDL GL contexts. Existing behavior remains the regression reference. Modern native backend can be introduced later.
- **Browser:** select WebGL backend under Emscripten SDL/canvas.
- **Maintenance:** high long-term burden—two shader/lighting/effect/UI implementations or a legacy versus modern semantic pair, duplicated bug fixes and golden tests.
- **Cleanliness:** materially better than current code if the boundary is real, but legacy backend constraints can leak into DTOs.
- **Risk:** fastest preservation strategy for desktop, but renderer behavior may drift and macOS compatibility OpenGL remains deprecated. Geometry currently encoded in calls must be extracted anyway.

This is viable as a risk-controlled transition and remains compliant only if all three desktop backends continue to be built/tested.

### Option B — one shared modern renderer (recommended end state)

**Shape:** replace compatibility rendering with one programmable C++ renderer targeting a rigorously limited GL ES/WebGL-style feature set. Use Emscripten WebGL in-browser and SDL-created shader-capable OpenGL contexts on Linux/macOS/Windows.

- **Reuse:** all simulation/extractors, CPU mesh/TGA/font loading where validated, viewport/camera state, effect parameters and resource catalog. Visual algorithms are migrated once.
- **Change:** all GL presentation/resource calls downstream of scene extraction, matrix math, shader programs, buffer resource ownership, text path, and context setup.
- **Linux/macOS/Windows:** compile the same renderer against native OpenGL entry points. CI and real-hardware smoke tests cover all three. On macOS choose a supported-enough core profile/version and avoid APIs outside it; do not introduce DirectX/Metal-only assumptions.
- **Browser:** compile the same source against GLES headers/WebGL. Differences are isolated to context/capability/resource packaging layers.
- **Maintenance:** lowest long-term renderer duplication and strongest visual consistency.
- **Cleanliness:** aligns with existing snapshot/pipeline direction; explicit resources/passes remove global compatibility state.
- **Risk:** larger native change up front and therefore greater immediate regression surface. Shader/version precision, line rendering, legacy lighting parity, winding, and context function loading must be validated.

A WebGL 1/ES 2 common denominator gives broad compatibility; WebGL 2/ES 3 offers better formats/features. Choose after PoC, not by assumption. Desktop OpenGL may use GLSL variants generated/maintained from one shader definition if syntax versions cannot be identical.

### Option C — portable rendering library behind the same boundary

A cross-platform C++ library that supports WebGL plus Linux/macOS/Windows could implement buffers/shaders/backend differences. It may reduce low-level portability work but adds a dependency, build integration, shader conventions, binary/size/licensing review, and still requires translating all immediate geometry/effects. This option is viable only after a spike demonstrates all four targets and the required blend/viewport/text behaviors. Do not choose a browser-only library or one whose macOS/Windows/Linux backend support is incomplete.

## Minimum abstraction contract

A future contract should cover, without GL types:

- `Initialize(RenderSurface&, Capabilities)`, `Resize(drawableWidth, drawableHeight, dpiScale)`, `Render(SceneData const&)`, `Shutdown()`;
- immutable mesh/image/font input and opaque typed handles;
- camera/viewport rectangles and explicit model matrices;
- materials (texture, base color, lighting mode, alpha mode), not arbitrary enable/disable calls;
- opaque, transparent/additive, debug-line, player-HUD and global-UI passes;
- error reporting through engine logging rather than `exit()`;
- context loss/resource recreation for browser robustness, useful on native as well.

Keep `SceneData` free of ownership ambiguity and graphics API identifiers. The current raw `uiData` pointer and access back into `App::graphicsTask` should be removed during implementation—not as prerequisite source churn, but as part of making the boundary enforceable.
