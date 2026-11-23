# Browser-port current status

**Status date:** 2026-11-19; documentation consolidated 2026-11-23

**Authority:** this closeout is the current plan. Retained dated implementation
reports are historical evidence; their open items do not override this closeout.
Superseded plans and earlier roadmap snapshots are available in Git history.

## Browser delivery closeout

The browser port is implemented and deployed. On 2026-11-06 the owner reported
successful manual testing on Linux, Windows and macOS, and confirmed that the
current game works when deployed to a webserver. This closes general desktop
runtime and hosted-browser delivery checks for the configurations tested.

This is owner-reported acceptance, not a new automated test run. Browser/OS
versions, desktop renderer selections, deployment URL/package hash, test-case
coverage and session duration were not supplied. The report does not establish
an exhaustive browser/device matrix, exact visual parity or resolution of the
historically deferred multiplayer failures.

The reviewed repository revision is `295d976` (macOS modern vertex-array fix),
following the terrain precision, enemy upper-assembly scale and Escape-menu fixes.
The tested/deployed revision was not independently identified; no package hash
is inferred from the report.

| Milestone | Implementation status | Closeout disposition |
| --- | --- | --- |
| R1 — shared modern player path | Implemented | Closed for browser delivery. |
| R2 — renderer/resource contract and desktop portability | Implemented | General desktop runtime success reported by owner; per-renderer matrix is not claimed. |
| R3 — shared opaque world | Implemented | Terrain, entities, materials and lighting delivered; exhaustive visual parity deferred. |
| R4 — effects and view handling | Implemented | Water, effects, overlays, debug lines, split-screen rendering and resize delivered; multiplayer robustness remains separate. |
| R5 — UI/text and native cutover decision | UI/text implemented | Modern-only migration complete; legacy drawing/resource APIs and GLU removed. |
| R6 — browser lifecycle and release | Implemented and deployment confirmed | Close current delivery; defer broader qualification rather than claiming it passed. |

## Remaining-item dispositions

The owner authorized proceeding with remaining work or skipping it as necessary.
For this closeout, retain the working implementation and make these scope choices:

- **Compatibility retirement: backend retired on the feature branch.** Modern
  is the only selectable backend. `TANKGAME_RENDERER` values, including the old
  `compatibility` override, now use modern rendering on every platform. The
  unified tank renderer, factory and browser substitute are removed. Linux/Windows
  retain OpenGL 2.1/GLSL120; macOS retains 3.2 core/GLSL150; browser retains WebGL 1.
  World/UI/orchestration fixed-function branches are removed. GeometryAsset
  replaces DisplayList, compatibility resource APIs are gone, and GLU is no longer
  linked or required. Platform-only context/shader APIs and documentation closeout are complete. See [the audit](modern-renderer-audit.md).
- **Broader qualification: deferred.** Existing Chromium automation and the owner's
  manual report support delivery in tested environments. Firefox, Safari, mobile,
  physical gamepad/audio-device combinations, exhaustive recovery scenarios and
  extended soak/performance budgets are not newly qualified. Use the
  [manual QA plan](manual-qa-test-plan.md) and
  [results sheet](manual-qa-results-template.md) when expanding coverage or
  investigating a regression. No universal browser/device support claim is made.
- **Multiplayer/session-state follow-up (2026-11-12): implemented.** The retained
  co-op-to-single-player failure is fixed, along with stale player pointers and
  unsafe level reloads during entity updates, inconsistent versus mode/scoring,
  overwritten versus spawns and a level-filename buffer overflow. Native tests,
  sanitizer checks and Linux/Chromium smoke evidence are recorded in the
  [multiplayer report](../../documentation/multiplayer-session-2026-11-12/README.md).
  Extended multiplayer soak and physical-controller/platform coverage remain
  open; these fixes do not establish that every historical crash is resolved.
- **Settings persistence: skipped for this release.** Packaged read-only defaults
  reset on reload; persistence is a future feature, not unfinished port work.
- **Additional asset/build refactors: deferred.** Full preloading and the existing
  shared C++ build are sufficient for this delivery. Staged downloads, indexed
  geometry optimization and further target decomposition need a concrete benefit.
- **Documentation reconciliation: completed here.** Current status supersedes old
  phase tables; README includes browser build/package/run guidance and the required
  macOS launch guidance. Dated evidence reports are retained; superseded planning snapshots are in Git history.

No further browser feature implementation is required for the current delivery
scope. Reopen the relevant item for a reproduced defect, new support target or
a newly scoped renderer improvement. Deferred checks are coverage gaps, not passing tests.

## Implemented platform decisions

Current code uses WebGL/GLES2 with GLSL
ES 1.00, desktop OpenGL 2.1/GLSL 1.20 on Linux/Windows, and OpenGL 3.2 core/GLSL
1.50 on macOS. Shader variants share generated source; `TANKGAME_RENDERER`
is ignored and context/shader selection depends only on platform. Browser audio uses SDL_mixer OGG/WAV with explicit user
activation. All shipped levels and required assets are preloaded, and settings
remain read-only. See the [release implementation and hosting guide](../../documentation/browser-release-2026-10-28/README.md)
for packaging and the existing automated evidence.

## Compatibility retirement closeout — 2026-11-19

All five retirement steps are implemented on the feature branch. The modern
renderer is the only backend; its world/UI passes, GeometryAsset resources and
platform-only context/shader APIs replace the old selection, drawing and resource
paths. GLU is no longer required. No renderer-retirement implementation work
remains. See [the audit and final validation](modern-renderer-audit.md).

The owner's Linux (SteamOS), macOS, Windows 11 and browser acceptance was reported
before the final deletion slices. Exact builds, versions, controller combinations
and durations were not supplied. Final local checks do not imply a new runtime
qualification on every platform or a deployed release.
