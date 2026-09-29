# Browser-port architecture reconciliation and current roadmap

**Status date:** 2026-09-29

**Authority:** this document describes the implementation now present and the
forward plan. The older feasibility, architecture, audit, and migration documents
remain the historical record of the pre-implementation recommendation.

## Executive assessment

The browser proof of concept has crossed its original feasibility threshold. It
builds and links, preloads a deliberately limited asset package, yields every frame
to the browser, and has been exercised in headed Chromium. A flat-colour player
tank (body and turret), bullets, and turret response to mouse movement have been
observed at approximately 60 FPS after release-build diagnostic logging was gated.
This is a successful vertical slice, not a feature-complete browser port: terrain,
enemy visuals, effects, HUD/menu, texture sampling, lighting, browser audio, and a
polished browser interaction lifecycle remain absent or deferred.

The implementation followed the plan in its most important direction: gameplay and
`SceneData` extraction stayed shared; mesh calls became CPU `Geometry`; explicit
matrix/colour state and a real shader/VBO submission path replaced browser-side
fixed-function emulation. It deliberately used the plan's transitional option for
speed and native safety. Native still executes compatibility OpenGL while
Emscripten selects modern paths, substitutes no-op/deferred renderers, and excludes
legacy renderer sources.

The result is therefore **a mixture, closest to architecture B**: shared simulation,
DTOs, CPU geometry preparation, orchestration, and portions of renderer classes;
but a modern browser GPU implementation alongside the retained native compatibility
renderer. It is not yet architecture A (one modern renderer plus small platform
adapters). This divergence was useful PoC scaffolding. It becomes architectural
drift if more visual systems receive browser-only implementations before the
existing modern path is proved and adopted on native.

**Recommendation:** choose Option B now: converge on one programmable renderer for
Emscripten, Linux, macOS, and Windows. First make the existing geometry/shader/VBO
path selectable on native Linux and render the already-working player tank fixture.
Keep the compatibility renderer as a temporary runtime/build fallback and visual
oracle. Do not next implement browser-only terrain, enemies, effects, or UI.

## Current runtime findings and multiplayer priority (2026-09-29)

The [manual regression review](../../documentation/manual-review-2026-09-29/README.md)
of `6fd947f` rebuilt Linux and Emscripten Release targets and passed all 84 native
tests. The shared modern player/material slice runs in browser and Linux; native
compatibility remains the default. The milestone records below describe the
earlier implementation steps and their validation at the time.

| Finding | Current status |
| --- | --- |
| Split-screen viewport clearing | **Fixed and visually verified** in browser, Linux default, and Linux modern. Both views survive movement, camera changes, and firing in fresh co-op/versus runs. |
| Longer multiplayer/session sequences | **Open, deferred / low priority.** Browser can trap and Linux modern can segfault. Successful short runs do not establish a clean multiplayer baseline. |
| Returning to single-player | **Open, deferred / low priority.** Selecting single-player after multiplayer can retain two players and split-screen state on all three renderers. |
| Independent player-2 input | **Unverified, deferred / low priority.** No joystick was available; independent movement, firing, and camera controls were not tested. |

Single-player sustained firing passed three 60-second browser runs and one
60-second run in each Linux renderer. This evidence supports the tested
single-player slice and the framebuffer-clear fix, not general multiplayer
robustness. See the [known-issues register](risks.md#current-known-issues-2026-09-29)
for evidence and follow-up boundaries.

**Planning decision: multiplayer robustness is deferred and low priority, outside
the current browser-port critical path.** These failures may reflect older
multiplayer/session-state architecture; the player-count reset omission predates
the port, while the crash root causes and their relationship remain unconfirmed.
Keep the failures visible in validation reports. Do not count the multiplayer
baseline as clean or interpret renderer smoke passes as multiplayer sign-off.

Continue shared renderer/resource convergence and the desktop validation gates.
Do not start a multiplayer rewrite or mix session/respawn/controller architecture
work into renderer migration. Revisit multiplayer later as a separately scoped
subsystem. R4/R5 retain their viewport and UI rendering checks; those checks do
not require resolving this deferred robustness backlog.

## Renderer convergence milestone 1 (2026-09-28)

The first convergence step is now implemented. `DisplayList` retains its neutral
facade but its geometry path is no longer selected by target platform: in modern
mode the same `Geometry` preparation, interleaved VBO, attribute layout, shader,
explicit MVP/default-colour uniforms, and draw call execute on Linux and WebGL.
Native compatibility display lists remain in the same facade solely as the
temporary regression backend. The shared modern scene slice is player body and
turret, bullets, and items; terrain, enemies, effects, and UI remain deliberately
outside this milestone.

### Selection and compatibility strategy

Native defaults to the compatibility renderer. Set `TANKGAME_RENDERER=modern` to
select the programmable slice from the same executable. Emscripten always selects
modern mode. Linux continues requesting an OpenGL 2.1 compatibility context, so
both choices coexist and the fallback's fixed-function calls remain legal. The
modern backend targets the OpenGL 2.1/GLES2 common buffer/shader subset.

The shader has one shared vertex body and one shared fragment body. WebGL receives
an ES precision preamble; desktop receives `#version 120`. This is the only shader
dialect split. GL header and prototype selection is isolated in `PlatformGL.h`.
The backend uses buffers, shaders, attribute arrays, uniforms, and draw arrays;
UVs and normals remain uploaded but intentionally have no texture/lighting effect.

### Validation and remaining boundaries

On Linux, Release configure/build and all 77 tests passed. Both modes initialized
under Xvfb from the same executable. Compatibility mode remained alive for its
smoke interval. Modern mode entered gameplay, created six tank entities, displayed
the flat-colour player body/turret with the expected hierarchy and winding, accepted
mouse/key injection, and shut down through Escape without a shader/GL error. The
headless software/Xvfb swap interval ran much faster than real display refresh, so
it is not evidence for a 60 FPS native performance claim. Bullets use the same
shared modern transform/draw path and the build/runtime path was exercised, but a
bullet was not captured in the headless screenshot; item code also shares the path,
while the selected level produced no item fixture. Precise legacy-versus-modern
pixel parity, bullet/item captures, and real-display frame pacing therefore remain
manual validation items rather than claimed results.

The Emscripten Release target was rebuilt successfully and still excludes the
compatibility-only renderer sources. Browser visual behavior remains supported by
the previously recorded headed-Chromium evidence; it was not re-observed in this
headless run. In the affected convergence files, Emscripten directives decreased
from 46 to 43. Remaining directives either omit desktop display-list/fixed-function
fallback code from WebGL, select the small GLSL preamble/header boundary, or guard
still-deferred compatibility subsystems. Renderer choice, modern transforms,
geometry upload, and modern draw submission are now runtime/shared decisions rather
than Emscripten renderer branches.

macOS still needs an early decision: its supported core contexts reject both the
2.1 compatibility fallback and GLSL 1.20, while its legacy compatibility context is
deprecated. Validate a core-compatible shader preamble/function set before claiming
modern macOS support. Windows OpenGL headers expose only 1.1 entry points directly,
so the modern functions need an explicit loader (or SDL proc loading); this Linux
milestone does not claim Windows readiness. Do not retire compatibility rendering
until later milestones cover terrain, enemies, effects, HUD/menu, textures and
lighting with native visual fixtures on all desktop targets.

## Renderer convergence milestone 2: shared basic materials and textures (2026-09-28)

The modern Linux and WebGL paths now share an unlit texture path. The selected
real fixture is the player's targeting/ready indicator and special-energy overlay:
it is small, already uses the migrated player transforms and UV-bearing square/tank
geometry, and uses the existing `ring.tga` and `p_itemstar.tga` assets without
pulling terrain, HUD, enemies, or lighting into the milestone. The legacy path
modulates these textures with `glColor` and uses additive blending; the modern
shader explicitly implements the same `texture sample * base/vertex colour`
relationship and retains the existing additive pass state.

The resource flow is now `TGA -> ImageData -> GpuTexture -> BasicMaterial ->
RenderContext/DisplayList shader draw`. `ImageData` contains dimensions, RGB/RGBA
format, and owned bytes, so decoding is independently testable and contains no GL
identifier. `GpuTexture` owns creation, portable upload, sampler setup, and deletion.
`BasicMaterial` contains an RGBA multiplier and an optional shared texture; an
absent texture selects the unchanged flat-colour shader behavior. `TextureHandler`
is the transitional catalogue/owner. It exposes owned texture objects to modern
code, while its raw-handle array remains only for compatibility renderers. The
separately instantiated `TextureHandler` members in `GraphicsTask` and the newer
`ResourceManager` remain duplicated ownership and should be unified later.

All currently loaded TGAs, including the 128x128 ring and 64x64 item-star used by
the fixture, are power-of-two RGB images. Upload sets unpack alignment to one,
uses linear filtering, mipmaps, and repeat where requested. The shared policy
automatically falls back to clamp-to-edge, linear minification, and no mipmaps for
NPOT inputs, satisfying WebGL 1/GLES2 restrictions without a browser-only uploader.
The old GLU/native versus `glTexImage2D`/browser upload split was removed.

Release Linux build and 80/80 tests passed. Under Xvfb/llvmpipe, both default
compatibility and `TANKGAME_RENDERER=modern` entered level 0 and shut down; a
captured modern frame visibly showed the modulated ring/item-star textures with
the expected orientation, transforms, and filtering, and logs contained no shader
or GL error. The compatibility frame remained the richer visual oracle; full
pixel parity is not claimed because modern lighting and the legacy animated
texture-matrix drift are intentionally still absent. The complete Emscripten
target configured, compiled, linked, and emitted HTML/JS/WASM/data using these same
`ImageData`, `GpuTexture`, `BasicMaterial`, shader, and player-renderer sources.
The preloaded `/texture` directory includes both selected assets. Browser visuals
were not re-observed in this environment: serve `runtime/`, open
`tankgame-linux.html`, press Enter for level 0, and verify the ring/star overlays,
tank and bullets, clean console, and frame pacing near the prior 60 FPS baseline.

In the texture/material implementation files touched here, `__EMSCRIPTEN__`
directives fell from 26 to 23. Texture upload and migrated player draw selection
lost their platform branches. The remaining directives isolate desktop display
lists/fixed-function fallback and the shader precision/version preamble; none
selects a distinct modern texture/material implementation.

macOS remains unvalidated: a core-profile context needs a core-compatible GLSL
preamble (GLSL 1.20 is not sufficient), although the texture calls themselves are
portable. Windows remains unvalidated: shader, VBO, active-texture, and mipmap
entry points still require an explicit loader/SDL proc-address strategy beyond the
system OpenGL 1.1 exports. The next convergence milestone should migrate one small
world-space category onto this material contract (not terrain as a whole) and
centralize duplicate resource ownership; lighting, terrain, enemy, effects beyond
this player overlay, HUD/menu, and text remain deferred.

## Renderer convergence milestone 3: authoritative resource catalogue (2026-09-29)

`ResourceManager` is now the authoritative catalogue for the migrated renderer
slice. `GraphicsTask` no longer loads a second copy of the GSM meshes, uploads a
second complete texture set, or owns shadow display-list/modern geometry objects.
Both Linux renderer modes and Emscripten resolve typed geometry and material IDs
through the same manager. The compatibility renderer builds its display-list view
from the catalogue CPU geometry and uses raw texture names only through the
explicit transitional texture bridge; modern interfaces expose no raw GL handle.

Texture decode results now remain as shared immutable `ImageData`, each
`GpuTexture` is uploaded once, and ring/star material templates have stable
catalogue ownership. Pipeline cleanup precedes catalogue destruction while the GL
context is current. Focused tests establish stable typed lookup identity and
explicit failure for invalid IDs. See the
[resource ownership audit](renderer-resource-ownership.md) for the before/after
map, lifetime rules, compatibility boundary, conditional count, and desktop
readiness notes.

This milestone does not migrate terrain, enemy tanks, effects, HUD/menu, lighting,
audio, or session behavior. The recommended next step is the narrow shader/program
and GPU-submission ownership boundary described in that audit, not a new feature
category.

## Renderer convergence milestone 5: desktop portability boundary (2026-09-29)

The shared modern slice no longer depends on Linux extension prototypes.
`GLFunctions` centrally loads every post-OpenGL-1.1 entry point through
`SDL_GL_GetProcAddress` on desktop, uses normal GLES symbols on Emscripten, and
fails initialization with a list of missing functions. `GraphicsCapabilities`
now explicitly selects Linux/Windows OpenGL 2.1 compatibility, macOS modern
OpenGL 3.2 core, and Emscripten ES 2.0 contexts. macOS compatibility mode is
explicitly unsupported rather than being placed in a core context.

The single logical shader now emits GLSL 1.20, GLSL 1.50 core, or GLSL ES 1.00
syntax. Release Linux compilation and all 89 tests passed; the complete
Emscripten application configured, linked and packaged. Linux modern initialized
and remained alive for a five-second headless smoke, but legacy segfaulted after
the environment reported its missing `XDG_RUNTIME_DIR`; there was no X server or
Xvfb, so neither visual path was revalidated. There was no browser either. This
environment also had neither Windows nor macOS toolchains/runtimes: those
targets have architectural and build-system validation only, not compile/link
or runtime validation. In particular, deferred renderer categories still use
compatibility APIs and therefore prevent a claim that the whole game is macOS
core-ready. See the focused
[desktop portability report](desktop-modern-renderer-portability.md).

The next milestone should qualify this boundary in Windows/macOS CI and on real
machines, then migrate one narrow deferred draw category. It must not be treated
as permission to begin terrain, effects, HUD/text, lighting, audio, browser UX,
or multiplayer work.

## Original intent, without hindsight

The original documents made three different levels of statement:

### Recommended end state

* One C++ programmable renderer using shaders and vertex/index buffers and a
  deliberately conservative WebGL/GLES-style feature subset on all four targets.
* `SceneDataBuilder` and render DTOs form the boundary between mutable gameplay and
  presentation. Camera/viewport/model/material/UI inputs become explicit data;
  renderer/resource interfaces contain no GL types or IDs.
* Simulation, event/collision/combat logic, loaders/decoders, extracted CPU geometry,
  matrix math, render-data extraction, visual algorithms, and resource descriptions
  remain shared.
* SDL-created context/canvas setup, browser scheduling, filesystem packaging,
  user-gesture/pointer-lock/audio policy, native library discovery, and possibly a
  thin GL/GLSL capability adapter remain legitimately platform-specific.
* Linux, macOS, and Windows remain supported, built, and runtime-smoke-tested. A
  browser build alone was never the acceptance condition.

### Acceptable transition, not the destination

The documents explicitly allowed a native compatibility backend plus a modern
browser backend behind the same DTO boundary. This reduced immediate native
regression risk, but was described as higher-maintenance and permitted only while
all desktop targets remained tested. A CPU transform stack or old renderer fallback
was likewise a migration aid, not permission to maintain two visual implementations
forever.

### Hard requirements versus recommendations

Hard requirements were a C++/WebAssembly game (not a gameplay rewrite), a
non-blocking browser lifecycle, no compatibility-GL dependency in required browser
rendering, working native targets, and honest feature/parity gates. A single shared
modern renderer was the preferred architecture rather than the only technically
acceptable first release. WebGL 1 versus WebGL 2, exact shader dialect management,
manifest shape, and whether to use a portable graphics library were intentionally
left for proof-of-concept evidence.

## Verified current runtime inventory

| Claim | Finding and evidence quality |
| --- | --- |
| Linux native remains functional | **Verified for compile and tests in this audit.** Release configure/build succeeds and 77/77 CTest tests pass. Historical browser milestone reports also say the Linux executable was manually run. No new native visual runtime was performed in this headless audit. |
| Existing native tests are green | **Verified:** 77/77 on Linux. These are mostly unit/headless tests, not visual renderer parity. |
| Emscripten builds and links | **Supported by source and prior run evidence.** CMake emits HTML/JS/WASM/data and recent milestone reports document successful Release/Debug builds, and a fresh Release Emscripten configure/build succeeded in this audit. |
| Browser loop is non-blocking | **Verified in source:** Emscripten registers `BrowserFrame` with `emscripten_set_main_loop_arg`; native retains `TaskHandler::Execute`, which calls the same one-frame `Tick`. |
| Assets are packaged/preloaded | **Verified, but only a subset:** fonts, eager textures, settings, title/level0, and four GSM files. Other levels and sound are omitted. |
| Browser was manually run | **Documented evidence:** headed Chromium milestone and performance runs, clean shutdown, screenshots, and measurements. This audit does not relabel that as automated cross-browser validation. |
| Approximately 60 FPS | **Verified as a local historical measurement, not a universal target:** 59.79 FPS idle and firing after logging cleanup on the recorded Chromium/AMD setup. |
| Player tank/body/turret, bullets, mouse rotation | **Documented headed-browser evidence.** Player body/turret and bullets were visible; mouse movement changed turret-relative view. Item submission exists but the report does not establish a visible gameplay item fixture. |
| Deferred visuals | **Confirmed:** terrain, enemies, effects, HUD/menu, texture sampling and lighting are deferred in browser. Browser audio is disabled. Pointer-lock UX, resize/high-DPI, full asset coverage, and context-loss recovery are also incomplete. |
| Neutral CPU geometry | **Mostly true:** `Geometry`, `PreparedGeometry`, topology expansion, and QGLMesh extraction contain no GL types. The `DisplayList` facade is neutral at its public surface but semantically and internally still combines native display lists and browser VBO/shader ownership. |
| Explicit matrices/colour | **True but transitional:** `Matrix4` and `RenderContext` are GL-free; migrated browser paths consume them. Native renderers still use the fixed-function matrix stack and colour state. `RenderContext::Current()` is global mutable state rather than a complete frame/command DTO. |
| Significant Emscripten branching | **True:** 69 Emscripten conditional directives in 16 production C/C++ files, plus Emscripten build selection in root/source/test CMake. Most renderer branches are transitional rather than permanent platform boundaries. |

## Original migration-plan status

“Complete” means the original behavioural success criteria have been met, not merely
that a target compiles. Work happened out of order: browser linking, geometry,
player/bullet rendering, packaging, and performance work advanced before the
cross-platform baseline, common native renderer proof, full input lifecycle, and
resource architecture were complete.

| Phase | Status | Implemented now | Remaining / deviation | Does the original success criterion still apply? |
| --- | --- | --- | --- | --- |
| 0 — baseline and executable-path discovery | **PARTIAL** | Source audits, active pipeline documentation, 77 Linux tests, and browser screenshots/performance evidence exist. | No recorded Linux/macOS/Windows behavioural matrix; no three-OS artifacts or reference captures; active/dead legacy paths are not fully instrumented. | Yes. Narrow it to representative fixtures and all-OS smoke evidence rather than blocking every later experiment. |
| 1 — portable graph/non-rendering WASM | **PARTIAL** | Emscripten application compiles/links; dependency and source-selection branches exist; core tests run natively. | No portable-core/renderer target split; source glob builds one application; no recorded WASM/Node tests or native CI matrix. Browser success skipped the intended non-rendering library gate. | Yes, revised: target decomposition should occur only where it enforces the convergence boundary, not as unrelated churn. |
| 2 — shared frame lifecycle | **SUBSTANTIALLY COMPLETE** | Shared one-frame `Tick`, native owning loop, RAF-owned Emscripten callback, cancellation and ordinary cleanup exist. | Hidden-tab delta policy, robust initialization error UI, and lifecycle automation remain. | Yes. “Menu simulation with rendering stubbed” has been superseded by a runnable rendered slice, but cleanup/timing requirements remain. |
| 3 — SDL canvas/input/focus/pointer lock | **PARTIAL** | ES2 SDL canvas works; keyboard starts game; SDL input updates; mouse turret motion was observed. | Relative mode is deliberately disabled; no click-to-lock UI, lock-loss/blur clearing, resize/high-DPI, event/hot-plug, or browser gamepad validation. | Yes; mouse motion without deliberate pointer-lock UX is not completion. |
| 4 — renderer proof foundation | **PARTIAL** | Real GSM extraction, GL-free geometry preparation, VBO, shaders, explicit MVP/colour, and player mesh rendering work in browser. TGA upload is browser-safe. | Shader does not sample textures or light; no shared modern path on any desktop; no four-target fixture/screenshots; VBO/shader lives inside an Emscripten branch of `DisplayList`; no recreation test. | Yes and now the highest-priority gate. The required proof should use the already-working player tank rather than invent a new fixture. |
| 5 — resource and asset pipeline | **PARTIAL** | Browser preloads a startup subset; QGLMesh emits CPU geometry; facade owns GPU resources; topology conversion is tested. | No manifest/root abstraction, one CPU/GPU ownership model, indexed buffers, font centralization, complete asset set, context recreation, or clean texture decode/upload split. Native/browser resource implementations remain coupled to old/new backends. | Yes. Complete it incrementally behind the shared renderer; do not wait for a perfect manifest before native proof. |
| 6 — terrain/camera/split-screen | **PARTIAL** | CPU perspective/view matrices and viewport iteration exist; single frame clear and both split-screen views are verified at 1280×720 in browser and both Linux renderers. | Browser terrain, resize/DPI, broader aspect/seam coverage, terrain batching, water, and four-target visual gates remain open. Multiplayer robustness is separately deferred / low priority. | Yes for rendering. Representative levels still need validation; passing viewport checks is not multiplayer sign-off. |
| 7 — tanks/items/bullets/effects | **PARTIAL** | Browser player body/turret and bullets render; geometry/item submission code exists. Extractors/DTOs remain shared. | Enemy tanks and effects are deferred; items lack documented visible validation; overlays, indicators, texture/lighting and full variant parity are missing. Browser chooses a separate player renderer factory path. | Yes. Revise order: converge the player/bullet path natively before adding categories. |
| 8 — HUD/menu/text/cutover | **NOT STARTED** | Shared HUD/menu data extraction and native legacy renderers pre-existed. | Browser implementations are deferred/no-op; no atlas/batched UI; native cutover is not begun. | Yes. Do not count pre-existing DTO organization as modern-renderer completion. |
| 9 — audio/interaction lifecycle | **NOT STARTED** | Browser deliberately disables sound and startup music, avoiding autoplay failure. | No activation/resume, codec/mixer, positional sound, hidden-tab, or browser audio smoke test. | Yes. Disabling audio was correct bootstrap scaffolding, not implementation. |
| 10 — persistence/polish/release | **PARTIAL** | Standard shell/data output, clean loop shutdown, release logging gate, performance harness, GL-error checks, and local screenshots exist. | Loading UX, persistence, fullscreen, responsive canvas, focus/accessibility, context loss, browser matrix, hosting/release automation and all native release artifacts are missing. | Yes, but release hardening remains after feature/convergence gates. Performance work was a justified out-of-order stabilization. |

## Renderer convergence milestone 4: shared program and submission (2026-09-29)

The programmable path no longer treats each `DisplayList` as a miniature
renderer.  The verified old lifecycle created one identical shader program for
every modern `DisplayList` (15 catalogue slots at initialization), stored the
MVP, colour, and texture as mutable geometry state, and queried five uniform
locations on every draw.  Its VBO was separable in principle but lived beside
the program and submission code in the same private implementation.

The modern flow is now `Geometry -> GpuGeometry -> ModernRenderer::Draw`.  A
`GpuGeometry` owns one interleaved VBO plus topology, layout and vertex count.
The catalogue owns one `ModernRenderer`, whose private `ShaderProgram` compiles
and links the existing logical shader once, caches all five uniform locations,
and serves player body/turret, bullets, items, and textured ring/star draws.
`RenderContext` computes the MVP and supplies it with a `BasicMaterial` in one
explicit call; neither transform nor material is persisted on geometry.

`DisplayList` remains a deliberately transitional facade to avoid unrelated
repository-wide churn.  It owns CPU geometry and either exposes its uploaded
`GpuGeometry` to the explicit modern submission path or executes the native
compatibility list through `Call`.  Calling `Call` in modern mode is now an
error.  Its compatibility branches retain fixed-function/list behavior and are
not compiled by Emscripten.

Lifecycle is deterministic: after a current GL context exists,
`ResourceManager` creates the shared program, uploads textures and VBOs, and
connects it to `RenderContext`.  Shutdown first removes pipeline users, then
releases every geometry facade/VBO, disconnects the context, destroys the
program, and finally releases catalogue textures, all before `VideoTask`
destroys the GL context.  No shader compilation, link, uniform lookup, VBO
creation, or texture upload occurs per frame.

Release native configure/build and all 86 tests passed.  Xvfb startup smokes reached the native task loop in both default compatibility
and `TANKGAME_RENDERER=modern` modes without shader or GL errors. The smokes
were time-bounded at the title scene, so they do not constitute fresh gameplay
visual or clean-shutdown validation.
The complete Emscripten Release target configured, compiled, linked, and emitted
HTML/JS/WASM/data packaging.  Browser visual validation remains manual: launch
the generated HTML through a local HTTP server, enter one-player gameplay,
verify body/turret and mouse aim, fire continuously, observe items and ring/star
overlays, check the console for WebGL errors, confirm approximately 60 FPS, and
exercise shutdown/navigation.

In the directly refactored facade, `__EMSCRIPTEN__` directives fell from 10 to
9.  One shader-preamble directive moved into `ModernRenderer`, so the combined
modern-backend total remains 10; the difference is now isolated to shader
dialect, while the nine facade directives exclusively exclude compatibility
display-list operations.  `PlatformGL` retains its one header-selection branch.

macOS still needs a core-profile context and a core-compatible GLSL variant;
centralizing program creation makes that variant a single narrow change, but
the modern slice still uses GLSL 1.20 syntax and compatibility-only renderers
remain elsewhere.  Windows still needs runtime loading for buffer, shader,
program, attribute, uniform, active-texture, and mipmap entry points.  Those
calls now have clear integration points in `GpuGeometry`, `ModernRenderer`, and
`GpuTexture`, rather than being repeated per geometry.

The next convergence milestone should establish the portable GL function
loading/core shader boundary and validate this same migrated slice on another
desktop platform.  It should not add terrain, enemies, effects, HUD/menu, text,
lighting, or audio.

## Current architecture and delta

```text
                    SHARED C++
 simulation/entities/levels/input state
                 |
       SceneDataBuilder + DTO extractors
                 |
 RenderingPipeline / viewport-camera orchestration
                 |
     migrated shared modern slice
 Geometry -> GpuGeometry -> RenderContext
                 |
  one ModernRenderer / ShaderProgram
                 |
      Linux modern + Emscripten

 Native compatibility fallback (separate bridge)
 fixed-function matrices/colour + display lists
 legacy lighting/textures and deferred feature categories
```

### Boundary audit

* **`Geometry` and GPU preparation:** genuinely GL-free. It carries positions,
  optional RGB/UV/normal fields and legacy-aware topologies; quads are expanded at
  preparation. It is non-indexed, lacks alpha/tangents/material identity, and is a
  data record rather than a resource owner.
* **QGLMesh extraction:** backend-neutral for all former triangle/edge variants.
  The parser/class still bears a historical “QGL” name and file-format oddities,
  but no longer exposes drawing methods.
* **Resource ownership:** not neutral yet. `ResourceManager`, `GraphicsTask`,
  `TextureHandler`, and `DisplayList` duplicate or mix CPU load and API upload.
  Public `DisplayList` hides GL types, but its name, compile-style methods, default
  state setters, and radically different implementations leak the transition's
  semantics. `TextureHandler` publicly returns GL texture IDs.
* **Matrix math:** `Matrix4` is neutral and tested. Browser transforms are explicit;
  native equivalents still live in `glMatrix*`/GLU calls, creating two transform
  implementations.
* **Render context:** GL-free in type terms, but a process-global mutable current
  context used only by migrated paths. It is not yet the proposed explicit
  `FrameContext`/draw-command contract.
* **Scene/render DTOs:** useful and mostly neutral; they successfully isolate
  simulation. They are not a complete renderer boundary: pipeline/renderers still
  reach managers/resources/global state, `uiData` has ownership ambiguity, and GPU
  material/resource handles are not modeled cleanly.

### GPU implementation audit

* Native `DisplayList::SetGeometry` still compiles vertices into an OpenGL display
  list; Emscripten prepares interleaved data, creates a VBO, compiles a shader per
  `DisplayList` implementation, and calls `glDrawArrays`.
* Quad conversion, triangles, independent lines, line loops, RGB, UVs and normals
  are represented. UV/normal attributes are uploaded but currently have no visual
  texture/lighting effect. There is no IBO despite the original VBO/IBO goal.
* Shader and VBO lifetime is RAII-like behind shared ownership, but the shader is
  unnecessarily per-resource and context-loss recreation is absent.
* The algorithms and GLES2 calls are not intrinsically browser-only. They are a
  reasonable seed for native use after shader dialect/function-loading/context
  compatibility is resolved. The current compilation guard—not the rendering
  model—is what makes them browser-specific.

### Renderer-class divergence

Already shared: scene extraction, viewport iteration, geometry definitions,
topology preparation, and much orchestration. Split concepts include:

* projection/view/model and colour: CPU `Matrix4`/`RenderContext` in browser versus
  native matrix stack and fixed-function colour;
* player tank: `PlayerTankRendererImpl` selected only for browser versus the native
  unified/legacy tank renderer, with further branches inside player renderer code;
* bullets/items: common classes with branch-specific transforms/state;
* terrain, enemies, effects, HUD/menu: full native compatibility implementations
  versus link-time browser deferred substitutes and pipeline skips;
* textures/lighting: native fixed-function behaviour versus prepared but unsampled
  textures and flat colour in browser;
* backend selection: preprocessor and CMake source exclusion, not a narrow runtime
  renderer factory capable of selecting modern/compatibility backends on desktop.

## Emscripten conditional audit

### Inventory method and counts

`rg` finds **69 Emscripten conditional directives across 16 production `.cpp/.h`
files**. CMake has **eight Emscripten-related `if` conditions** across root, source,
and test build files. A directive count measures branching sites, not independent
features; large `#ifdef` regions matter more than their raw count.

### Legitimate long-term platform/build boundaries

| Locations | Meaning | Disposition |
| --- | --- | --- |
| `main.cpp` | Emscripten header, browser callback, RAF registration/cancellation, native owning loop | Permanent adapter, preferably isolated in an application-platform unit. |
| `VideoTask.cpp` | GLES context profile versus desktop context; no startup relative mouse; current sound-setting suppression | Context and user-gesture policy are legitimate. Audio/settings policy should move to its subsystem rather than accumulate here. |
| `GameTask.cpp`, `InputTask.cpp` | Browser avoids immediate music/relative mouse behaviour | Legitimate policy until an explicit activation state replaces simple exclusion. |
| `Logger.h` | Release-browser debug logging gate for JS/DOM console cost | Justified performance policy, though a build logging level is more general than platform identity. |
| root/source CMake | SDL_mixer/TTF Emscripten ports, HTML suffix, preload package, WASM stack/memory, include-path workaround, no host GLU | Genuine toolchain/output/filesystem boundaries. Asset list should later come from a shared manifest. |
| test CMake | Emscripten library/source/link differences and avoiding host GLU | Legitimate while tests are made runnable under WASM; compile-only is not validation. |

### Transitional renderer divergence

| Locations | Current split | Why it should converge |
| --- | --- | --- |
| `DisplayList.cpp` | Browser VBO/shader versus native GL display list; 13 Emscripten directives | Central PoC scaffold. Promote programmable implementation to a shared GPU resource/backend and retire compile-style display-list semantics after fallback removal. |
| `GraphicsTask.cpp` | CPU projection/browser state versus GLU matrix stack and native fixed-function lighting/material | Ordinary frame/render setup should be shared explicit state; only context capability selection belongs at platform edge. |
| `BaseRenderer.cpp` | Browser-supported state subset and `RenderContext` versus native push/pop/fixed state | Replace with common pass/state descriptors; these branches are not permanent platform facts. |
| `RenderingPipeline.cpp` | Browser CPU camera/state and repeated skips versus native lighting/matrix/state | Highest concentration of architectural drift. A shared renderer should consume identical frame/pass commands. |
| `PlayerTankRenderer.cpp`, `PlayerTankRendererImpl.cpp`, `TankRendererFactory.cpp` | Browser-specific renderer selection and CPU transforms versus native fixed-function implementation | The same visual concept has two paths. Make modern player rendering selectable native first, then common. |
| `BulletRenderer.cpp`, `ItemRenderer.cpp` | Browser explicit transforms/default colour and omission of compatibility capability calls | Useful migrated slices; converge by using their modern branch everywhere. |
| `TextureHandler.cpp` | WebGL upload/mipmap/wrap versus GLU/native upload | API upload may need a thin capability adapter, but decoding, ownership, format rules and sampling semantics should be shared. |
| source CMake + `BrowserDeferredRenderers.cpp` | Excludes six legacy implementation files and links browser no-op substitutes | Acceptable proof scaffolding only. Remove as each complete shared pass replaces it; do not expand the substitute list. |

### Architectural-smell threshold

No individual bring-up branch is automatically a mistake. The smell is that backend
selection cuts through `GraphicsTask`, pipeline, base renderer, resource facade, and
ordinary object renderers. Branches that select different visual algorithms,
transforms, renderer classes, or “skip rendering” should eventually disappear.
Future permanent conditionals should be limited to application scheduling, context
creation/capabilities, browser interaction policy, filesystem packaging, audio
activation, and minimal API/shader portability adapters. Continuing to add
`__EMSCRIPTEN__` blocks to terrain/enemy/effect/HUD implementations would create the
parallel renderer the original plan warned against.

## Options from the current state

### Option A — retain dual renderers

This has the lowest short-term effort and protects today's native appearance. It
also requires every missing visual feature to be implemented and verified twice,
leaves duplicated transform/colour/material logic, increases visual drift, and
turns every bug fix into a two-backend decision. Apple's deprecated compatibility
OpenGL makes this a poor long-term macOS strategy. It is acceptable only as a
time-boxed fallback during convergence.

### Option B — converge on the modern renderer everywhere (**recommended**)

The CPU geometry/preparation, QGLMesh extraction, `Matrix4`, `RenderContext`, real
GSM resources, shaders, VBO submission, and shared DTO pipeline provide a meaningful
head start. Most of the modern algorithm is reusable natively. Required work is
native programmable context negotiation/function loading, a supported common GL
subset, GLSL ES versus desktop/core syntax strategy, centralized shader/buffer
ownership, and explicit pass/material state.

Risk should be controlled rather than denied: preserve compatibility rendering as a
selectable native fallback and screenshot oracle; migrate a complete player-tank
slice first; run both from the same scene fixture; switch native default only after
terrain/entities/UI parity and all desktop platform gates. macOS must be tested early
because its core-profile constraints can invalidate a Linux-only design.

### Option C — adopt a third-party portable rendering library

Nothing learned so far requires this. The existing slice proves direct GL/WebGL is
viable, and a new dependency would not remove the work of translating materials,
lighting, terrain, effects, and UI. Reconsider only if the native-context/shader
spike reveals unsustainable portability work and a candidate proves WebGL plus all
three desktops, licensing, binary size, and required passes.

## Recommended target architecture

```text
 simulation / game state
          |
 SceneDataBuilder + owned, GL-free SceneData/FrameContext
          |
 shared RenderingPipeline (pass ordering only)
          |
 shared programmable Renderer
   |-- shared mesh/image/font CPU descriptions
   |-- shared geometry preparation + GPU resource catalog
   |-- shared opaque / transparent / debug / UI passes
   |-- shared model-view-projection, materials, lighting and colour
          |
 narrow graphics portability layer
   |-- buffer/texture/program/state operations + capability table
   |-- GLSL source/version adaptation where demonstrably necessary
          |
   +------ SDL desktop GL context: Linux / macOS / Windows
   +------ SDL Emscripten WebGL context

 separate true platform adapters:
 browser RAF + focus/pointer lock/audio activation + preload/canvas
 native owning loop + native filesystem/window/audio startup
```

## Revised milestones from the present state

### R1 — protect and prove the shared modern player slice on native Linux

**Objective:** compile the existing programmable geometry path on native Linux and
render the same player body/turret fixture without changing the default renderer.

* **Scope/files:** `DisplayList`/successor GPU ownership, shader source portability,
  native GL loading/context attributes, `Matrix4`, `RenderContext`, player renderer,
  and a build/runtime backend selector.
* **Prerequisites:** capture current native and browser player views plus deterministic
  scene/mesh assertions; decide minimum desktop GL/GLSL baseline.
* **Exclude:** terrain, enemies, lighting parity, texture sampling, UI, audio, and
  removal of compatibility code.
* **Gates:** Linux old/default path still builds/runs; opt-in modern path shows the
  real body/turret with correct transforms/colour; browser fixture remains clean of
  GL errors; unit tests pass.
* **Retire afterward:** player-specific `__EMSCRIPTEN__` transform/factory selection
  where the modern path can be shared. Do not retire native fallback.

### R2 — establish one renderer/resource contract and validate macOS/Windows early

**Objective:** turn the PoC facade into an explicit shared programmable backend and
prove it compiles/runs on all desktops before feature migration.

* **Scope/files:** rendering interface, frame/pass context, program cache, mesh/VBO
  handles, resource teardown/recreation descriptions, CMake target/source grouping,
  desktop context/function loading and shader variants.
* **Prerequisites:** R1 results and human decisions on WebGL version, desktop GL
  floor, shader strategy, and backend selector lifetime.
* **Exclude:** broad visual feature work and speculative gameplay refactors.
* **Gates:** opt-in tank fixture builds and runtime-smokes on Linux/macOS/Windows and
  Emscripten; no GL types/IDs in DTO/CPU geometry interfaces; fallback remains green.
* **Retire afterward:** per-resource shader ownership, broad `DisplayList` browser
  implementation branches, and CMake player-renderer source substitution where
  superseded.

### R3 — complete one shared opaque world pass

**Objective:** migrate texture sampling, basic lighting/material state, player,
bullets/items, enemy tanks, and representative opaque terrain through one shared
path, rather than adding browser-only renderers.

* **Scope/files:** image decode/upload boundary, common shaders/materials, terrain
  batching, tank variants, bullet/item renderers, winding/depth/cull state and
  resource catalog.
* **Prerequisites:** R2 backend and approved native/browser reference fixtures.
* **Exclude:** transparent effects, water polish, HUD/menu/text and audio.
* **Gates:** deterministic one-level screenshots on four targets; player/enemy/
  bullet/item fixture coverage; browser GL errors zero; fallback comparison and
  performance budget; gameplay tests unchanged.
* **Retire afterward:** corresponding native immediate/display-list paths,
  `BrowserDeferredRenderers` terrain/enemy substitutes, and renderer-level platform
  branches for migrated opaque concepts.

### R4 — shared transparent/effect and complete level/view pass

**Objective:** add effects, overlays, water/transparent ordering, debug lines,
split-screen, resize and per-viewport projection using explicit shared pass state.

* **Prerequisites:** complete opaque pass and agreed blend/depth parity tolerances.
* **Scope/files:** effect renderer/data, transparent sorting/state, viewport/camera,
  canvas/drawable resize and terrain variants.
* **Exclude:** HUD text, audio, and deferred multiplayer/session-state robustness work.
* **Gates:** representative effects and one-/two-player captures on all targets;
  browser resize/DPI/GL-error smoke; no state leakage between passes.
* **Retire afterward:** native effect/overlay compatibility calls, browser effect
  substitute, and pipeline skip/state conditionals.

### R5 — shared UI/text cutover and compatibility-renderer retirement decision

**Objective:** implement HUD, menu, text, and global/player-local UI with shared
batched resources, then decide whether parity is sufficient to make modern native
the default.

* **Prerequisites:** resource ownership stable and baseline UI captures available.
* **Scope/files:** HUD/menu renderers, font atlas/cache, UI DTO ownership, old
  `GraphicsTask` text/HUD/menu reachability, backend default/telemetry.
* **Exclude:** browser audio/persistence except activation UI hooks, and deferred
  multiplayer/session-state robustness work.
* **Gates:** menus/HUD/text and split-screen UI on four targets; stable resource
  counts; visual approval; fallback remains selectable for one release if desired.
* **Retire afterward:** deferred HUD/menu source, legacy UI functions/resources, and,
  after explicit approval, the compatibility renderer and most renderer conditionals.

### R6 — browser interaction, audio, asset completeness and release matrix

**Objective:** finish product boundaries once rendering convergence is no longer at
risk.

* **Scope:** click-to-play/pointer lock/focus/blur, gamepad, audio activation and
  codecs, all required level/assets, resize/fullscreen, hidden-tab timing, context
  loss, loading/error UX, persistence policy, hosting and CI/release artifacts.
* **Prerequisites:** R5 or an explicit decision to ship while native fallback remains.
* **Gates:** supported-browser matrix and long run; Linux/macOS/Windows build plus
  runtime smoke; audio/input scenarios; cold/warm load and recovery; reproducible
  packages.
* **Retire afterward:** startup audio/input exclusions, hand-maintained minimal
  preload list, bootstrap diagnostics and temporary fallback if parity was approved.

## Platform validation and test protection

| Platform | Evidence today | Gap/action |
| --- | --- | --- |
| Linux | 2026-09-29 Release build and 84/84 CTest pass; default/modern gameplay and split-screen captures exist. Sustained single-player firing passes; modern crashes in a longer multiplayer sequence. | Multiplayer is an open, deferred / low-priority limitation. Retain deterministic default/modern renderer fixtures and the crash evidence; no clean multiplayer baseline is claimed. |
| macOS | Source/header/CMake accommodation only; no build or runtime artifact found in current browser-port records. | Unvalidated. Add compile plus real core-context/shader runtime in R2, before broad migration. |
| Windows | Visual Studio files and source branches exist; no recent build/runtime artifact found. | Unvalidated. Add CMake/VS compile and GPU smoke in R2. |
| Emscripten | 2026-09-29 Release build, fresh co-op/versus captures, and three 60-second single-player firing runs at 59.81–59.82 FPS with clean GL probes/shutdown. A longer multiplayer sequence traps. | Multiplayer is deferred / low priority; independent player-2 input is unverified. Evidence remains one Chromium/AMD environment; CI/WASM tests, browser matrix, interaction/audio, context recovery, and feature completeness remain open. |

Before renderer convergence changes, retain the existing browser screenshots/results
and add a deterministic scene fixture with mesh counts/bounds, camera/model matrices,
and frame-state assertions. Add image comparisons with tolerant thresholds rather
than brittle exact pixels. A smoke test should assert shader link, zero GL errors,
non-empty draws and clean teardown; screenshots require human-approved references on
each graphics family/platform.

### Reconciliation validation history

The 2026-09-28 audit ran the following in the available Linux environment:

* `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
* `cmake --build build -j4`
* `ctest --test-dir build --output-on-failure` — 77/77 passed
* `emcmake cmake -S . -B build-browser-audit -DBUILD_TESTS=OFF
  -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_UPDATES_DISCONNECTED=ON`
* `cmake --build build-browser-audit -j4` — HTML/JS/WASM/data link succeeded
* `rg` inventories for `__EMSCRIPTEN__`, `EMSCRIPTEN`, source selection, and all
  browser-port documents; `git diff --check` after documentation edits

The browser artifact was not launched during this documentation audit. Runtime and
performance conclusions are therefore attributed to the checked-in headed-browser
milestone reports rather than represented as a new run.

## Documentation reconciliation

* `feasibility.md`, `graphics-audit.md`, and the original phase bodies in
  `migration-plan.md` are useful historical design records. Their “future” wording
  is stale if read as current status, so they now point here.
* `architecture.md` accurately captures the pre-port topology but not the current
  browser branches; it remains historical and points here.
* `risks.md` remains materially relevant, but several risks have changed state; a
  status note directs readers here rather than silently rewriting old predictions.
* `display-list-boundary.md`, `qglmesh-geometry.md`, `geometry-gpu-backend.md`,
  `transform-color-state.md`, `linkable-application.md`, and `runnable-browser.md`
  are milestone records. Earlier statements such as “tank rendering unavailable”
  were true at that milestone and are superseded by later sections/documents; they
  should not be edited into a false contemporaneous narrative.
* `documentation/browser-runtime-performance.md` is the authoritative local
  performance/run record, not a promise of portable 60 FPS.

## Human decisions still required

1. Approve convergence (Option B) and time-box the compatibility fallback, or
   explicitly accept the long-term cost of Option A.
2. Choose the supported WebGL baseline and minimum desktop GL/core-profile version,
   especially the macOS floor.
3. Choose shader portability: common restricted source, generated preambles/variants,
   or separately tested dialect files.
4. Decide whether backend selection must be runtime-selectable for comparison or a
   build option is sufficient during rollout.
5. Define visual-parity tolerances: reproduce legacy lighting exactly versus approve
   a consistent modern look; define required debug/line-stipple approximations.
6. Define supported browser/device matrix, canvas DPI cap, resize/fullscreen policy,
   and pointer-lock fallback UX.
7. Decide audio/browser codec requirements and whether settings remain read-only or
   gain persistence.
8. Decide how long all runtime assets should be preloaded versus staged, and which
   levels/game modes define the first browser release.
9. Provide or fund real macOS and Windows build/runtime agents; source conditionals
   cannot substitute for validation.

## Exact documentation changes in this reconciliation

No production or test source is changed. This document is added as the current
status/roadmap, and short historical-status pointers are added to
`architecture.md`, `migration-plan.md`, and `risks.md`.
