# Modern-default Linux qualification — 2026-11-07

Revision under test: `e6784e6` on `renderer/modern-default`. The game/test binaries
were built in the preceding default-switch step (158/158 GL-enabled tests and
3/3 explicit compatibility startup checks passed there). This follow-up runs live
Linux/X11 gameplay and deterministic production effects; no production changes.

## Results

| Check | Result |
| --- | --- |
| Unset `TANKGAME_RENDERER` | Application log confirms `modern`. |
| Explicit `compatibility` | Application log confirms `compatibility`. |
| Title, single-player, movement, jump, menu return and restart | Both processes survive the sequence and exit normally with status 0. Inspected captures show textured terrain, tanks, overlays and restored menu; modern also shows status/FPS/menu text. |
| Co-op split-screen | Two viewports and local HUDs visible in both modes. Independent player-2 input was not tested. |
| Co-op → menu → select single-player → start | **FAIL in both modes:** two players/viewports remain. Reproduces the historical session-state backlog; not unique to the modern default. |
| Sustained firing input | **Unverified:** synthetic mouse presses were sent, but captures do not establish projectile emission or energy consumption. Pointer repositioning in the modern rerun did not resolve this evidence gap. Do not interpret screenshot names as passed assertions. |
| Deterministic effects | Both modes exit 0, six fixture frames have GL error 0. Modern additive passes preserve depth/state; occlusion checks preserve the other viewport. |
| Effect image comparison | All six cases pass the existing tolerance: identical silhouettes, channel rounding <= 1 except at most two coplanar outline samples per frame. No tolerance was widened. |

Representative title, movement, menu, split-screen and failed single-player-return
screenshots are retained alongside both application/driver transcripts. Effect
frames 0/5 and the full comparison JSON are retained; rerunning regenerates all
captures. Live gameplay is not seeded or pixel-compared. Menu text overlays the
existing HUD/world when returning from play, as documented by the Escape-menu fix.

## Reproduction

Run from the repository root, with a Linux X11 display and libX11/libXtst available.
Run drivers sequentially: they focus the game window and inject keyboard/mouse
input. The driver checks process survival/exit; visual outcomes require inspection.

```sh
python3 documentation/modern-default-2026-11-07/native-smoke.py default
python3 documentation/modern-default-2026-11-07/native-smoke.py compatibility
python3 documentation/modern-default-2026-11-07/build-fixture.py
(cd runtime && env -u TANKGAME_RENDERER ../build/modern-default-effect-fixture)
(cd runtime && TANKGAME_RENDERER=compatibility ../build/modern-default-effect-fixture)
python3 documentation/modern-default-2026-11-07/compare-fixtures.py
```

The native driver is adapted from the terrain milestone, explicitly distinguishing
unset default from compatibility and adding co-op/single-player transitions. The
effects fixture/comparator come from the October effects milestone, with output
paths updated and obsolete `GraphicsTask::defaultFont` cleanup removed to match
current resource ownership. The initial compile exposed that stale cleanup; the
updated fixture builds and passes. Historical artifacts are unchanged.

## Qualification disposition

Step 2 is **partially validated**, not complete. Keep the default switch and the
explicit fallback; do not remove compatibility based on this run. Windows/macOS
modern-default runtime checks, browser regression, physical firing/input checks,
versus transitions, broader level/effect coverage and visual owner approval remain
unverified here. The multiplayer/session-state defect remains deferred under the
existing scope decision. These Linux results do not close that backlog or imply a
long-session performance guarantee.
