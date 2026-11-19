# Modern renderer audit — 2026-11-18

Audited source: `6e600b7`, with the subsequent owner acceptance recorded in the
working documentation. The source audit was followed by the targeted fixes and
dynamic regression described below. This is not a new visual qualification or
a compatibility-removal patch. Findings below describe the audited baseline.

## Findings requiring work

1. **High priority: native startup still reaches fixed-function GL.**
   `GraphicsTask::Start()` in `src/GraphicsTask.cpp`, immediately after the shared
   depth/culling setup, guards lighting/material initialization only with
   `#ifndef __EMSCRIPTEN__`. Modern desktop startup therefore executes
   `glLightModeli`, `glPushMatrix`, `glLightfv`, `glLightf`, `glPopMatrix`,
   `glColorMaterial` and legacy lighting/normalization/texture enables.
   These operations are invalid in the macOS 3.2 core context. Linux/Windows
   request a 2.1 compatibility context even for modern rendering, masking this
   dependency. The follow-up regression reproduced startup error 1282 twice
   before the fix. Visual acceptance does not establish an
   error-free GL stream. Guard this block by backend, then remove it with the
   fallback. Add a real core-context GraphicsTask startup/stop regression that
   checks both GL errors and DEBUG logger output, since logging can consume errors.

2. **Medium priority: browser effect statistics are overwritten with zero.**
   `RenderingPipeline::RenderEffects()` records submitted effects, but
   `UpdateRenderStats()` subsequently overwrites `effectsRendered` with zero
   under `__EMSCRIPTEN__`. Effects now render on the browser. Remove this stale
   deferred-renderer branch and specify consistent object-count semantics across
   platforms and viewports. This affects reported statistics, not effect pixels;
   it does not invalidate owner visual acceptance. Draw/triangle submission
   counters are separate and should remain authoritative for GPU submissions.

3. **Coverage gap: core tests do not exercise application startup or a full frame.**
   `CoreRendererTest` covers mesh/VAO submission and three shared state APIs.
   `VideoStartupTest` starts VideoTask, not GraphicsTask. Effect pixel/depth tests
   use the platform's selected context (2.1 on Linux). Add core coverage for
   startup, representative world/UI passes, single/split views and resource
   teardown before claiming the whole modern path is core-qualified. Existing
   CPU contracts and manual qualification remain useful, but do not fill this gap.

## Reachability and retained dependencies

| Surface | Audit result | Retirement action |
| --- | --- | --- |
| GraphicsTask startup | Reachable legacy block identified above; also initializes duplicate legacy renderer members | Fix startup first; remove duplicate members after verifying their references |
| Pipeline camera/lighting/state | Modern matrices and RenderContext lighting selected before compatibility GL/GLU branches; shared stack operations now return in modern mode | Keep modern render order and state, delete compatibility branches |
| Terrain and water | Terrain returns through RenderModernTerrain; water has a modern-only pass after opaque objects | Preserve terrain cache, winding, depth bias and water depth-write contract |
| Player/enemy/bullet/item/effect draws | Modern geometry is selected, but the full-frame regression exposed legacy player setup and matrix push/pop surrounding that geometry | Retain geometry/material/transform contracts and transparent ordering |
| HUD/menu/targeting/debug | Modern geometry/font routes bypass fixed-function alternatives | Preserve viewport/scissor restoration, text and split-screen isolation |
| Sky | GraphicsTask::DrawSky is empty | No hidden legacy sky draw to migrate |
| DisplayList | Modern SetGeometry owns GpuGeometry; RenderContext::Draw obtains ModernGeometry from this facade | Replace/rename the facade while preserving GPU ownership; do not delete the class wholesale |
| ResourceManager | Shared textures, meshes, glyphs and terrain batches feed the modern renderer | Keep catalogue and caches; remove only legacy generation/accessors |
| TextureHandler/GpuTexture | Shared uploads already use portable GL texture operations; raw compatibility handles remain | Remove raw-handle consumers first; preserve shared texture lifetime |
| Unified TankRenderer | Selected for compatibility; browser supplies a no-op substitute to satisfy symbols | Remove factory enum/constructor paths and GraphicsTask's duplicate member before deleting implementation/substitute |
| Build boundary | Native GLU linkage lives in src/tests CMakeLists; platform headers still include GLU | Delete calls/includes before removing linkage; validate both native and browser source lists |

Unused helpers are distinct from reachable failures. Searches of `src/` found
only declarations/definitions for BaseRenderer's SetLighting/PushMatrix/PopMatrix,
PlayerTankRendererImpl's effects/targeting setup helpers, and the old
PlayerTankRenderer::DrawPlayerTanks entry point. TerrainRenderer's separate
Setup3DRenderState has no call sites and hides a nonvirtual base method. These
contain unsafe legacy calls, but were not identified as active modern paths.
Remove them rather than preserving misleading APIs. Do not confuse the old
Tank-based player overloads with the active TankRenderData-based rendering API.

GraphicsTask::Stop destroys the pipeline, cleans and resets the resource manager
before VideoTask's higher-priority stop destroys the context. ResourceManager's
Cleanup alone retains TextureHandler-owned textures until destruction; the
normal Stop path resets the owner. Preserve that ordering during retirement and
exercise it in the startup/stop test. This observation is not a demonstrated leak.

## Implementation sequence

1. Reproduce and fix the startup core-context error; retain the regression.
   Correct stale browser statistics and add representative full-frame core
   coverage, including teardown/recreation while the context is alive.
2. Remove dormant helpers and duplicate GraphicsTask renderers. Remove the
   unified TankRenderer/factory alternative and browser substitute together.
3. Delete compatibility world/UI/orchestration branches, preserving modern
   passes and their state contracts. Validate native GL regressions and browser
   build after each coherent slice.
4. Replace the DisplayList facade with an explicitly named GPU geometry resource;
   delete display-list APIs, raw compatibility accessors and GLU includes/linkage.
5. Finalize modern-only selection and fresh-process tests. Retiring the renderer
   need not raise Linux/Windows context requirements: retain 2.1/GLSL120 policy
   unless separately changed. Update stale context comments and retirement docs.

## Validation and limits

The existing graphics-enabled native CTest suite was rerun during this audit:
**234/234 passed**, using `TANKGAME_TEST_GL=1`, `LIBGL_ALWAYS_SOFTWARE=1` and
`SDL_AUDIODRIVER=dummy`. This result demonstrates the coverage gap above rather
than disproving the startup finding. That initial run preceded the fixes below. No new visual captures were made.

The owner-reported Linux (SteamOS), macOS, Windows 11 and browser acceptance
remains recorded in [compatibility-retirement.md](compatibility-retirement.md).
Compatibility removal is still pending; the source audit is complete for the
surfaces listed here. The representative core regression below expands dynamic coverage; it is not
a proof of every possible runtime path or every driver.


## Implemented follow-up

- Guarded native GraphicsTask lighting/material initialization by compatibility
  mode. Modern startup no longer calls removed fixed-function APIs.
- The representative frame regression then exposed GL_INVALID_ENUM (1280) in
  player rendering. Its TankRenderData entry point still called native legacy
  state setup and unconditionally pushed/popped the matrix stack. Modern setup
  now establishes depth state and returns; matrix operations are compatibility-only.
- Removed the browser-only zero-effect override. Object counts consistently
  describe the last rendered scene (including objects skipped by an individual
  pass); submission counters remain the all-view frame totals.
- Added `CoreRendererTest.ModernGraphicsStartupFramesAndRecreationAvoidFixedFunctionCalls`.
  It starts the real GraphicsTask in a 3.2 core context, submits terrain/water,
  player/enemy tanks, a bullet, item, smoke, debug lines, HUD and menu/text through
  the pipeline, transitions 1 → 2 → 1 views, clears scene objects and verifies
  counts reset, then stops and recreates the resources. It checks GL errors,
  logger errors and nonzero submissions. It does not assert pixel equivalence
  or all effect/tank/menu variants; existing visual acceptance and specific
  effect pixel/depth contracts remain separate evidence.

The next retirement slice is removal of dormant helpers and duplicate
GraphicsTask renderers, followed by the unified compatibility renderer/factory
alternative. The fallback remains available in this patch.


Final follow-up validation: native build passed; graphics-enabled CTest
**235/235 passed**; explicit compatibility `VideoStartupTest.*` **3/3 passed**;
Emscripten incremental build passed; `git diff --check` passed. Browser runtime
was not rerun for this patch, so the browser statistics correction is verified
by source review/build rather than a new browser runtime assertion.


## Dormant renderer cleanup — feature branch follow-up

Removed the five duplicate GraphicsTask renderer members and their startup/stop
calls. RenderingPipeline is now the sole owner of the active pass renderers;
resource cleanup still happens before context destruction. Repository reference
searches found no external consumers of the removed members.

Removed unused BaseRenderer lighting/matrix helpers and its unused 2D/blending
helper chain; TerrainRenderer's nonvirtual, uncalled Setup3DRenderState;
PlayerTankRendererImpl's uncalled state helpers, empty geometry methods and
unused constants; and the old Tank-based PlayerTankRenderer entry points and
private overloads. The active TankRenderData API and its shared setup helpers
remain, including those used by the compatibility renderer.

This completes the dormant-helper/duplicate-owner part of step 2. The unified
TankRenderer, factory alternative and browser substitute still serve the
compatibility selection contract and are the next retirement slice, together
with the necessary selection/orchestration changes. Modern geometry resources,
shader/context policy and render ordering are unchanged by this cleanup.


Cleanup validation: native and Emscripten builds passed; graphics-enabled native
CTest **235/235 passed**, including the full core startup/frame/recreation test;
explicit compatibility VideoStartupTest **3/3 passed**; `git diff --check` passed.
No new tests were needed for deleting uncalled code. Browser runtime and
macOS/Windows builds were not rerun for this cleanup.


## Unified renderer retirement — feature branch follow-up

RenderingPipeline now owns PlayerTankRendererImpl and EnemyTankRendererImpl
as concrete members and always uses the established opaque-player, opaque-enemy,
then transparent-overlay ordering. Removed the unified TankRenderer, its factory,
the browser no-op substitute, and the obsolete factory example. Both native and
browser CMake source lists now use the same remaining renderer implementation.
The enemy input-rejection test now exercises the concrete renderer directly.

RendererMode temporarily remains as a constant-modern facade while remaining
legacy branches are removed. Every environment value, including `compatibility`,
uses modern rendering; this preserves startup for old launch configurations
without exposing an incomplete fallback. Fresh-process selection tests enforce
this policy. Context and shader baselines are unchanged; OpenGL compatibility
*context* support on Linux/Windows is distinct from the retired renderer.

This completes step 2 and advances the selection policy from step 5 because
removing the unified renderer makes the old fallback incomplete. Next: remove
unreachable world/UI/orchestration branches, then replace the DisplayList GPU
resource facade and remove obsolete GLU/resource APIs. Do not delete that facade
wholesale: modern geometry still depends on it.


Retirement-slice validation: native configure/build and Emscripten build passed;
graphics-enabled native CTest **235/235 passed**, including all five fresh-process
selection cases. With `TANKGAME_RENDERER=compatibility`, VideoStartupTest and the
full core startup/frame/recreation regression **4/4 passed without skips**, proving
the old override reaches the modern backend. `git diff --check` passed. No new
browser runtime or macOS/Windows execution is claimed for this slice.


## Shared modern drawing paths — 2026-11-19

Removed compatibility branches from terrain/water, player/enemy tanks, bullets,
items, effects, HUD, menu, debug lines and RenderingPipeline camera/lighting/UI
orchestration. Deleted their obsolete helper declarations, cached legacy state,
fixed-function matrix/attribute management and duplicate platform implementations.
The existing modern transforms, materials, pass order, winding, blending and
viewport/scissor restoration remain the shared implementation.

GraphicsTask startup and resize projection now use RenderContext directly.
The deletion also found native fixed-function calls in GraphicsTask::Update
that were still reachable even after backend selection retirement. Those matrix,
normalization, smooth-shading and lighting calls are removed. The existing core
regression now starts GameTask with the title scene and invokes GraphicsTask::Update
before its representative single/split-screen frames and resource recreation.
The fixture follows normal game-world initialization; merely constructing GameTask
is insufficient for scene extraction. This test exercises the actual update entry
point as well as the pipeline API.

Source searches find remaining immediate-mode/display-list drawing calls only in
DisplayList.cpp. ResourceManager/DisplayList still retain backend guards and the
legacy geometry facade, and unused GLU headers/linkage remain for the next slice.
This completes the world/UI/orchestration deletion (step 3); resource ownership,
API naming and build dependency cleanup (step 4) remain. The constant-modern
selection policy and context requirements are unchanged.


Drawing-path deletion validation: native build and Emscripten build passed;
graphics-enabled native CTest **235/235 passed**, including the expanded core
startup/title-update/frame/recreation regression; `git diff --check` passed.
No new browser runtime, macOS/Windows runtime or pixel-comparison captures were
made for this slice. Existing effect pixel/depth checks passed in the native suite.
