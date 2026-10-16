# R4 drawable sizing and viewport coverage — 2026-10-16

History through `6309427` completes the shared effects slice after terrain,
lighting and water. This follow-up implements drawable-size tracking and closes
the odd-height split-screen gap. Compatibility stays the default.

`VideoTask` requests resizable/high-DPI windows and queries SDL drawable pixels
at startup and immediately before rendering. `GraphicsTask` compares actual
dimensions and player count against its viewport manager instead of a static
player-count cache. Each view's existing projection uses its new aspect ratio.
Zero-area drawables skip rendering; empty split views retain player identity
(including a one-pixel-high drawable). The upper view receives any odd row.

## Validation

- Native Release build and browser Release build pass.
- 119/119 CTest cases pass with `TANKGAME_TEST_GL=1`. Modern mode separately
  passes both GL startup tests, including actual SDL window resizing.
- CPU tests cover odd sizes, a simulated doubled drawable size, empty views,
  restoration, and transition back to single-player.
- Both Linux backends render through `RenderingPipeline` at 801x601, 997x503,
  and back to 801x601, with one and two views. Terrain, enemy, water and effect
  passes are exercised. Each view has visible geometry and matches its isolated
  reference pixel-for-pixel; all six frames per backend report zero GL errors.
  PNGs record the first two sizes. These are view-isolation checks within each
  backend, not a new cross-backend visual tolerance claim.
- Chromium gameplay probes cover 801x601, 997x503, a simulated DPR of 2 producing
  1602x1202 drawable pixels, and restoration to DPR 1. Every viewport matches
  its drawing buffer; all four probes report zero GL errors and no context loss.

The browser harness establishes CSS canvas sizing **before SDL window creation**.
SDL 2.28.4 detects external sizing at creation; setting CSS later on the stock
generated shell does not opt it into responsive layout. The harness's first
attempt confirmed that limitation. The product shell's responsive/fullscreen
policy remains deferred. This does not claim physical monitor DPI testing,
browser split-screen qualification, player-overlay ordering, comprehensive level
coverage, or multiplayer robustness. Windows/macOS runtime gates remain deferred.

## Reproduction

From the repository root:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/viewport-resize-2026-10-16/build-fixture.py
```

From `runtime/`, run `../build/viewport-fixture` and then
`TANKGAME_RENDERER=modern ../build/viewport-fixture`. The fixture writes BMP
captures; the checked-in PNGs are lossless conversions. Also run
`TANKGAME_RENDERER=modern TANKGAME_TEST_GL=1 ./tankgame_tests --gtest_filter=VideoStartupTest.*`.

Build the configured `build-browser` target with its Emscripten environment,
serve `runtime/` on port 8766, and run:

```sh
env -u ELECTRON_RUN_AS_NODE /path/to/electron documentation/viewport-resize-2026-10-16/browser-smoke.cjs --no-sandbox
```

The harness writes screenshots and `browser-smoke.json`. It closes the browser
after the probes; game shutdown and long-run behavior are not tested here.
