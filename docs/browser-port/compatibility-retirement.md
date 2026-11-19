# Compatibility renderer retirement — completed 2026-11-19

Gameplay configuration was completed at `cfa9583`; the subsequent modern-renderer
retirement workstream is now implemented on the feature branch.

## Final implementation status

- Modern is the only renderer. `TANKGAME_RENDERER` is ignored, including old
  `compatibility` overrides. RendererMode and backend-selection parameters are gone.
- RenderingPipeline owns player/enemy passes directly; the unified renderer,
  factory, browser substitute and legacy world/UI/state paths are removed.
- GeometryAsset owns shared GPU geometry. DisplayList, compatibility resource
  aliases/raw texture handles and GLU headers/linkage are removed.
- Context/shader policy depends only on platform: Linux/Windows OpenGL 2.1 and
  GLSL120; macOS OpenGL 3.2 core and GLSL150; browser WebGL 1/GLES2 and GLSLES100.
  Linux/Windows compatibility contexts are an API baseline, not a legacy backend.

No renderer-retirement implementation step remains. The [audit and validation
record](modern-renderer-audit.md) documents the fixes, ownership contracts and
verification. Owner-reported Linux (SteamOS), macOS, Windows 11 and browser
acceptance predates the final deletion slices; it is not represented as a fresh
cross-platform run of the final branch. Extended soak, physical-controller
combinations and additional browser/device coverage remain separate limits.

Final local validation: **238/238** native graphics tests, native/browser builds
and **31/31** browser lifecycle checks pass. See the [retained evidence](../../documentation/renderer-retirement-2026-11-19/README.md).

## Historical implementation checkpoints

Everything below records intermediate states. References to a retained fallback,
pending audit or next deletion slice describe that checkpoint and are superseded
by the final implementation status above.

## First implementation slice: shared state in a core context

A new real OpenGL 3.2 core-context regression reproduced GL_INVALID_OPERATION
(1282) in all three shared state setup paths: BaseRenderer, PlayerTankRendererImpl
and RenderingPipeline. Pipeline cleanup also failed. Platform-only guards had
excluded these legacy calls from WebGL but still allowed them on modern desktops.

The fix restricts the following calls to compatibility rendering:

- `glShadeModel` in base and pipeline setup.
- Fixed-function lighting and texture enable/disable in player setup.
- Pipeline matrix/attribute stack push and pop.

Modern shaders already supply interpolation, lighting, texture selection and
transforms, and modern passes set their own required GL state. This slice changes
no shader, geometry, context-selection policy or compatibility-mode behavior.
The pipeline setup API is tested directly even where current orchestration does
not call it; base/player setup is also reached by normal player/bullet rendering.

`CoreRendererTest.ModernSharedRenderStateAvoidsFixedFunctionCalls` failed before
the fix and passes afterward. It checks both GL errors and DEBUG error logging
(which otherwise consumes errors), as well as retained depth/culling state.
This supplements the existing core-profile mesh-submission test with production
state setup/cleanup. It is not a whole-game core-profile qualification claim.

Validation of this slice:

- Native Release build passed; graphics-enabled CTest **234/234 passed**, no skips.
- Explicit compatibility `VideoStartupTest.*`: **3/3 passed** on Linux.
- Emscripten incremental Release build passed.
- `git diff --check` passed.
- The preceding configuration commit's browser evidence is **31 lifecycle + 24
  multiplayer checks**, eight sessions and fifteen multiplayer reloads. Those
  browser runtime checks were not repeated for this desktop-only state fix.

## Retirement inventory and order

| Area | Current dependency | Removal work after qualification |
| --- | --- | --- |
| Backend selection/context policy | RendererMode, VideoTask, GraphicsCapabilities, fresh-process selection tests | Make the supported backend policy explicit; retiring the renderer does not require simultaneously raising Linux/Windows OpenGL requirements |
| Shared orchestration | RenderingPipeline and BaseRenderer retain legacy camera, lighting and state helpers | Remove compatibility branches and obsolete helpers while preserving modern render order and state contracts |
| World/object passes | Terrain, water, player/enemy, bullet, item and effect renderers | Remove legacy draw paths after verifying category coverage and retained behavior tests |
| UI and diagnostics | HUD, menu, player overlays and debug-line compatibility branches | Keep modern geometry/font/viewport contracts and inspect single/split-screen presentation |
| Resources | DisplayList and compatibility portions of ResourceManager | Remove display-list generation/ownership and obsolete resource accessors |
| Build/platform boundary | TankRenderer and BrowserDeferredRenderers substitute; GLU dependencies | Remove obsolete implementations/substitutes and unused native GLU linkage after callers are gone |
| Tests and documentation | Mode-selection contracts and historical comparison fixtures | Replace fallback expectations with the decided policy; preserve reference captures and modern-only regression contracts |

No claim is made that every fixed-function call has been removed or guarded by
this first slice. The inventory identifies the next audit/removal surfaces; the
new regression covers the three shared state APIs, not every render pass.

## Manual qualification accepted

The owner reports that Linux (SteamOS), macOS, Windows 11 and the browser have
been manually tested, work correctly and look correct. This closes the requested
cross-platform runtime and owner visual-approval gates for the tested current
builds. Modern is the desktop default and the only macOS/browser backend.

This is owner-reported acceptance. Exact builds, browser versions, renderer
variable overrides, controller models, individual scenarios and session durations
were not supplied; an exhaustive device/scenario matrix is not inferred.
Existing Windows/macOS hosted CI still supplies build/CPU-test coverage rather
than graphics qualification. Linux graphics CI uses Xvfb/software GL.

## Historical remaining work after the first slice

- The [modern-path source audit](modern-renderer-audit.md) is complete. It found
  reachable fixed-function startup calls, stale browser effect statistics and a
  full-startup/frame core-test gap. The follow-up fixes startup and player-draw
  legacy calls, corrects the statistics and adds representative core-frame coverage.
  Continue with the staged deletion sequence in the audit.
- Verify each deletion against the modern regression contracts and native/browser
  builds; update renderer selection tests and documentation to the final policy.
- Retain the fallback until that implementation and verification are complete.
  The platform/runtime and owner visual-approval gates no longer block this work.

Physical-controller combinations and extended soak remain coverage limits, not
new approval requirements for the accepted retirement scope.

The previously documented co-op → single-player viewport defect was fixed on
November 12; it is not an outstanding blocker. Extended multiplayer soak remains
a separate coverage gap. The original November 7 retirement notes are historical
and should not be interpreted as undoing that fix or the newer browser evidence.


Audit follow-up validation: **235/235** native graphics tests and **3/3** explicit
compatibility startup tests pass; native/browser builds pass. The new core test
reproduced startup and player-draw errors before their fixes. See the
[audit findings and implementation record](modern-renderer-audit.md) for scope
and remaining coverage limits.


The feature-branch cleanup now removes duplicate GraphicsTask renderer instances
and unused state/Tank-based player APIs. RenderingPipeline owns all active pass
renderers. The next slice is the unified TankRenderer/factory/browser-substitute
removal with its selection/orchestration changes; compatibility still works in
this cleanup slice. See the audit's dormant-renderer cleanup record.


November 19 follow-up: shared modern drawing now replaces the former platform/
backend branches across the world, UI and pipeline. GraphicsTask::Update no
longer issues fixed-function state calls, and the core regression covers that
real update path. The next slice is resource API/ownership and GLU cleanup.
