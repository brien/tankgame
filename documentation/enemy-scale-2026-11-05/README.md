# Enemy upper-assembly scale correction

Native gameplay uses `TankRenderer`, whose body transform applies 0.06 then
10, leaving a residual 0.6 scale. The modern renderer used by the browser
incorrectly restored identity before drawing the upper assembly, making it
5/3 of the native size and enlarging the cannon offset by the same factor.
`BuildDraws` now preserves that residual scale. The separate compatibility
branch in `EnemyTankRendererImpl` also matches the native gameplay transform.

The earlier enemy fixture compared both modes of `EnemyTankRendererImpl`,
so it could not detect this difference from the actual native gameplay path.
This fixture instead compares native `TankRenderer` against modern
`EnemyTankRendererImpl`. Tests cover scale, cannon offset, relative yaw,
and combined pitch/roll/yaw.

Validation:

- Native and Emscripten builds succeeded.
- CTest: 145 passed, three opt-in graphics tests skipped initially.
- All three opt-in graphics tests subsequently passed with `TANKGAME_TEST_GL=1`
  in both compatibility and modern modes.
- Both fixture poses report GL error 0 in both modes.
- Pose 0: 25,301 foreground pixels in each mode, zero silhouette differences.
- Pose 1: 24,793 foreground pixels in each mode, zero silhouette differences.

These captures validate geometry and transforms on desktop using the browser's
shared modern renderer. They do not assert lighting parity or constitute an
in-browser gameplay check. Browser artifacts in `runtime/` were rebuilt.

Reproduce from the repository root:

```sh
python3 documentation/enemy-scale-2026-11-05/build-fixture.py
(cd runtime && TANKGAME_RENDERER=compatibility ../work/enemy-fixture)
(cd runtime && TANKGAME_RENDERER=modern ../work/enemy-fixture)
python3 documentation/enemy-scale-2026-11-05/compare-fixtures.py
```

The comparison script requires Pillow. Captures are stored alongside this file.
