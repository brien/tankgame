# Enemy renderer evidence, 2026-10-07

The [milestone report](../../docs/browser-port/enemy-modern-renderer.md) describes
implementation, commands, limits, and manual Windows/browser checks. This directory
records fresh results only; earlier manual-review directories remain historical.

## Native results

Release game/tests built. `ctest.log` records 98/98 passing under Xvfb with the
existing GL startup opt-in enabled. `ctest-modern-gl.log` records that startup
check in a separate modern process. Eight added enemy tests run without a display.

`linux-compatibility-summary.log` and `linux-modern-summary.log` retain startup
and shutdown excerpts; both gameplay processes exited with status 0. Full local
logs were inspected for GL/shader failures and none were recorded. Input was
injected with the existing Xvfb display and xdotool. Native gameplay captures:

- `linux-compatibility-idle.png`, `linux-compatibility-moved.png`: retained terrain,
  player and HUD reference; these views do not isolate nearby enemies.
- `linux-modern-idle.png`, `linux-modern-moved.png`, `linux-modern-firing.png`:
  visible enemies, movement/relative position changes, and the retained player
  ring/star and firing path. Distant enemies limit precise aim/depth assessment.

## Controlled close-up (separate from gameplay)

`enemy-fixture.cpp` uses production `VideoTask`, `ResourceManager`, and
`EnemyTankRendererImpl` without changing simulation. It draws three enemies at
fixed positions/body rotations and relative aim, then changes position/aim for
pose 1. It uses deliberate DTO colours (.2,.05,.1) for primary and (.05,.15,.2)
for secondary, with health=maxHealth=100, to expose body-versus-turret colour and
avoid saturation hiding contours. These are inspection colours, not assertions
about every gameplay type's colour.

`fixture-compatibility.log` and `fixture-modern.log` report `GL error=0` in both
poses. The four `enemy-fixture-{compatibility,modern}-{0,1}.png` images have equal
foreground masks: 43,440 pixels in pose 0 and 44,203 in pose 1, zero differing
mask pixels in each comparison. Thus geometry and transforms produce matching
silhouettes with the tested depth/cull state; lighting still differs visibly.
This is not whole-game pixel parity or a performance benchmark.

To reproduce on Linux after the native Release build:

```sh
python3 documentation/enemy-renderer-2026-10-07/build-fixture.py
cd runtime
../work/enemy-fixture
TANKGAME_RENDERER=modern ../work/enemy-fixture
```

Use the same SDL/OpenGL dependency environment as the native build and a usable
`DISPLAY`. The fixture writes BMPs in `work/`; the checked-in PNGs are lossless
conversions. It links production objects from the existing Unix Makefiles build;
this evidence helper is not an extra CI target or cross-platform build gate.

## Browser evidence

`browser-smoke.cjs` is the browser replay driver. Install Playwright locally, run
an HTTP server on port 8766 serving `runtime/`, then execute the driver with a
headed display. It uses `/usr/bin/chromium` and SwiftShader explicitly; adjust
the executable for a manual machine. Screenshots, GL/context probes, opaque draw
state samples, console output, and shutdown status are written beside the driver.
The shader/VBO implementation is unchanged by its draw-call instrumentation.

`browser-smoke.json` records Chromium 151.0.7922.173, four zero-error/no-context-loss
WebGL probes, opaque clockwise/depth-write draw samples, no JavaScript errors, and
successful loop shutdown. `browser-{idle,enemy-motion,moved,firing}.png` capture
actual gameplay after waiting for enemy draws. There was one console HTTP 404 for
the absent favicon; all application companion requests succeeded. The Release
build logs record successful packaging with Emscripten 3.1.69 and the local
SDL2/HTML-minification adapters described in the milestone report.

Build and actual browser results are recorded in the milestone report. Xvfb and
SwiftShader observations do not qualify hardware GPU behaviour, browser diversity,
real-display frame pacing, or deferred multiplayer robustness.
