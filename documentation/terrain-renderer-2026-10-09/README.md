# Opaque terrain evidence, 2026-10-09–12

See the [milestone report](../../docs/browser-port/terrain-modern-renderer.md) for
implementation scope and reproduction. Work began October 9; Linux gameplay and
browser runtime checks finished October 12 after session interruptions.

## Build and tests

- `native-configure.log`, `native-build.log`: native Release configuration/build.
- `ctest.log`: **108/108 passed, zero skipped**, with `TANKGAME_TEST_GL=1` on the
  local display. `ctest-modern-gl.log`: separate modern startup check passed.
  A final rebuild after removing unused test includes also passed;
  `ctest-final-headless.log` records 107 passing with the GL test skipped.
- `browser-configure.log`, `browser-build.log`: Emscripten **3.1.64 Release**
  package, using the installed SDK and existing cache. No toolchain adapter.

## Linux rendering

`native-smoke.py` drives X11/XTest input and captures the actual game window.
`modern-driver.log` and `compatibility-driver.log` record movement, jumping,
sustained firing, menu/restart and **exit 0**. Full gameplay logs are retained as
`modern.log.gz` and `compatibility.log.gz`; neither records a GL/shader failure.

Representative captures:

- [Modern gameplay](modern-idle.png), [movement](modern-moved.png),
  [sustained firing](modern-firing-sustained.png), [restart](modern-restart.png).
- [Compatibility reference](compatibility-idle.png),
  [compatibility movement](compatibility-moved.png).

The native gameplay run does not poll GL errors each frame. Explicit GL checks
belong to the startup test and deterministic fixture below. Items were not
isolated in these captures; no new item-parity claim is made.

## Deterministic fixture

`terrain-fixture.cpp` uses production renderers/resources. Run `build-fixture.py`
after a native build, execute `build/terrain-fixture` from `runtime` with and
without `TANKGAME_RENDERER=modern`, then run `compare-fixtures.py` to convert the
BMPs losslessly to PNGs and compare RGB values.

Frames 0/1/2 cover underground level 48, level 50 with a ceiling, and a same-number
level edit. Each includes steps and a floating cube. Lighting is disabled in the
compatibility fixture; no negative cells means the water pass draws no geometry.
All six frames report `GL_NO_ERROR`; repeated modern lookups retain GPU identity.

| Frame | Different RGB pixels | Silhouette differences | Maximum channel difference |
| --- | ---: | ---: | ---: |
| 0 | 609 | 0 | 1/255 |
| 1 | 1,701 | 0 | 1/255 |
| 2 | 1,670 | 0 | 1/255 |

`fixture-comparison.json` contains the numerical results. Compare
[modern frame 0](fixture-modern-0.png) and
[compatibility frame 0](fixture-compatibility-0.png).
The small channel differences are consistent with rounding in fixed-function
versus shader execution. This is geometry/texture evidence, not lighting parity.

## Browser rendering and cache behavior

`browser-smoke.cjs` runs with standalone Electron **44.7.0**, Chromium
**152.0.7977.130**, on the local visible display. WebGL reports
`ANGLE (AMD, AMD Custom GPU 0405 (radeonsi vangogh ACO), OpenGL ES 3.2)`.
Serve `runtime/` on port 8766, then run:

```sh
env -u ELECTRON_RUN_AS_NODE /path/to/electron browser-smoke.cjs --no-sandbox
```

The script records console output, screenshots, draw state, buffer-upload counts,
and GL/context probes in `browser-smoke.json`. It starts a page before attaching
the debugger; early harness attempts stalled before loading the game and are not
counted as application failures or successful runtime evidence.

The final run completed title, single-player idle, repeated idle, movement,
ten-second firing, restart, and two-Escape shutdown. **Six probes returned
GL_NO_ERROR, no context loss, no JavaScript/GL/shader errors, and clean shutdown.**
Screenshots show [terrain/player/enemies](browser-idle.png),
[movement](browser-moved.png), [firing](browser-firing.png), and
[restart](browser-restart.png).

Upload instrumentation counted 22 `bufferData` calls after title initialization
and 26 after entering level 0. It remained **26** through repeated idle, movement,
firing and the identical-level restart. Terrain samples have depth testing/writes
on, clockwise fronts and blending off. Instrumentation observes the production
backend without replacing its draw/upload implementation.

Steady firing intervals report approximately **59.95 FPS**, with tick CPU time
around **0.56–0.63 ms**. Startup/screenshot intervals include larger stalls. This
is a bounded, instrumented local smoke, not a controlled before/after performance
benchmark or cross-browser qualification. Modern lighting, water, effects, UI,
audio, other-browser/device coverage, and multiplayer robustness remain deferred.
Windows and macOS runtime checks are explicitly deferred by the owner.
