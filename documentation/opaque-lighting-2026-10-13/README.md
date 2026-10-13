# Opaque lighting evidence, 2026-10-13

See the [implementation report](../../docs/browser-port/opaque-lighting.md).

- `build.log`, `browser-build.log`: successful Release builds.
- `ctest.log`: 111/111 pass with the display-enabled GL startup test.
- `fixture-modern.log`, `fixture-compatibility.log`: six frames, every terrain,
  enemy, player, item and bullet pass returns GL_NO_ERROR; modern terrain cache
  lookups preserve GPU identity.
- `fixture-comparison.json`: six comparisons pass the executable tolerance gate
  in `compare-fixtures.py`. At most one silhouette-edge pixel differs in each
  pose; all other RGB channels differ by at most 1/255. Frame 5 is identical.
- `fixture-*-0.png` through `fixture-*-2.png`: levels 48/50, steps, floating
  terrain and entities. Frames 3–5 isolate entities at different rotations.
- `browser-smoke.json` and `browser-*.png`: visible Chromium/AMD gameplay,
  six clean GL/context probes, no uploads after level entry and clean shutdown.
- `modern-smoke.log`, `compatibility-smoke.log`, compressed game logs and
  corresponding screenshots: native movement, jump, sustained firing, restart,
  and exit 0. These gameplay captures precede the final explicit player winding
  correction; the six final fixture pairs and final browser run include it.

Build the fixture from the repository root with
`python3 documentation/opaque-lighting-2026-10-13/build-fixture.py`.
From `runtime`, run `../build/lighting-fixture`, then
`TANKGAME_RENDERER=modern ../build/lighting-fixture`. Run `compare-fixtures.py`
from the evidence directory or repository root. The fixture saves BMPs, which
that script converts losslessly to the checked-in PNGs before comparing them.
The build helper uses the existing native Unix Makefiles build's object files.

Native gameplay reuses `../terrain-renderer-2026-10-09/native-smoke.py`, setting
`TANKGAME_REVIEW_OUT` to this directory. Browser reproduction uses the included
Electron harness with `runtime/` served on port 8766.

This is bounded local GPU evidence. Windows/macOS runtime qualification,
exhaustive visual coverage, transparent effects/water, UI and multiplayer
robustness remain deferred. No hosted CI or cross-browser qualification is claimed.
