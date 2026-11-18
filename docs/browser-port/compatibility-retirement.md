# Compatibility renderer retirement — started 2026-11-18

The gameplay configuration workstream is complete at `cfa9583`. This is the next
renderer workstream: qualify the modern path, then remove the legacy fallback
once its coverage and visual-approval gates are satisfied. Linux/Windows still
support `TANKGAME_RENDERER=compatibility`; macOS/browser remain modern-only.

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

## Gates still open

- Windows/macOS modern-default runtime evidence with platform and renderer
  identified. Existing hosted jobs build and run CPU tests there; they do not
  enable graphics tests. Linux graphics CI uses Xvfb/software GL.
- Broader level/UI/effect visual approval and physical firing/controller input.
  The user's completed gameplay manual testing is recorded, but its unspecified
  platform/scenario scope does not by itself close these renderer-specific gates.
- A complete modern-path reachability audit and staged deletion verification.
  Keep the fallback while qualification is incomplete.

The previously documented co-op → single-player viewport defect was fixed on
November 12; it is not an outstanding blocker. Extended multiplayer soak remains
a separate coverage gap. The original November 7 retirement notes are historical
and should not be interpreted as undoing that fix or the newer browser evidence.
