# Browser runtime performance — 2026-09-27

## Scope and instrumentation

This change preserves the first browser renderer milestone. No renderer features,
geometry, gameplay, collision algorithms, or task ordering were changed.

`GlobalTimer` uses SDL millisecond ticks for simulation `dT`; its `GetFPS()`,
GraphicsTask's legacy HUD FPS, and HUDDataExtractor's `averageFPS` all use the
inverse of one frame's delta. There was no actual rolling FPS accumulator.
Those simulation/HUD paths are unchanged.

`BrowserFrame` now uses `emscripten_get_now()` to accumulate callback spacing and
CPU time spent in `App::Tick()`. It reports once per elapsed second:

- `fps`: frames / elapsed interval (weighted rate, not averaged inverse deltas).
- `avg_ms`, `recent_ms`, `max_ms`: callback spacing, including browser scheduling.
- `tick_ms`: mean CPU time for the whole task tick, including synchronous stdout.
- `frames`, `interval_ms`: denominators for combining reports correctly.

Tick timing excludes the performance report itself. Callback spacing includes
that cost. Tick time is not GPU execution time. These measurements apply to a
visible, active browser tab; background throttling or a suspended tab changes
callback spacing. No additional timers or allocations run on each entity.

## Logging audit and policy

| Category | Sources | Policy |
| --- | --- | --- |
| Startup/one-shot | SDL/context, renderer/system initialization, level loading and metadata, player creation/input setup | Keep |
| Warning/error | Invalid player/tank state, NaN input, resource/shader/render failures | Keep |
| Lifecycle | Level transitions, player ownership/respawn, shutdown | Keep |
| Useful low-frequency | GameWorld entity summary every 60 frames; new elapsed-time performance report | Keep |
| Frame debug | Six PlayerManager frame stages; periodic input, dead-state, and physics breadcrumbs in Player/PlayerManager/Tank | Debug gate |
| Entity/collision debug | Point/sphere query inputs/results, Bullet collision checks, Tank firing, GameWorld bullet creation, CombatSystem combo details | Debug gate |

The gate is centralized in `Logger.h`: `TANKGAME_LOG_DEBUG` compiles out calls and
argument evaluation only when both `__EMSCRIPTEN__` and `NDEBUG` are defined.
CMake Release uses `-O3 -DNDEBUG`; browser Debug retains verbose diagnostics.
RelWithDebInfo/MinSizeRel also use the quiet policy. Native logging is unchanged.
`Logger::Write` and the generated shell are unchanged. The latter logs stdout to
the console, appends each line to a growing textarea, then reads scrollHeight and
sets scrollTop for every line. Thus synchronous browser UI work is inside the
measured tick. Renderer/frame-loop logging was startup/error-only before this
change; no normal renderer spam needed removal.

## Reproduction

1. Configure/build the existing Emscripten Release target:
   `cmake -S . -B build-browser -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_UPDATES_DISCONNECTED=ON`
   then `EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4`.
   This build directory already has the Emscripten toolchain configured.
2. Serve `runtime/`: `python3 -m http.server 8765 --directory runtime`.
3. Install Playwright in a temporary directory and its Chromium browser. Run
   `tools/browser-performance.cjs LABEL` using Node with `NODE_PATH` pointing to
   that installation's `node_modules` and `PLAYWRIGHT_BROWSERS_PATH` pointing to
   its browser downloads. An optional second argument selects another URL.
4. The harness opens visible Chromium, focuses the canvas, presses Enter, verifies
   game setup, warms up for 3 seconds, measures 20 seconds idle, holds left mouse,
   warms up for 3 seconds, and measures 20 seconds firing. It then moves the mouse,
   captures the view, presses Escape, and checks that frame reports stop.
5. Results, console events, and screenshots go to `/tmp/tankgame-LABEL*`.

The harness retains the original shell and stdout behavior. It collects existing
console events; it does not replace or silence Module.print. It omits the first
report in each sample because that report straddles the warm-up boundary, then
combines complete reports as total frames / total interval time. Consequently
reported intervals cover approximately 17–19 of each 20-second observation.
Screenshots and mouse checks occur outside those measured intervals.

## Measurements and conclusion

Visible Chrome for Testing 153.0.8010.12, canvas 1280×720, WebGL renderer
`ANGLE (AMD, AMD Custom GPU 0405 (radeonsi vangogh ACO), OpenGL ES 3.2)`.
Both targets use Release `-O3 -DNDEBUG`, the non-blocking RAF loop, 1 MiB stack,
and the unmodified generated shell. No builds ran during the measured windows.
The instrumented baseline binaries were saved before changing logging.

| State | Baseline repeat FPS | Baseline ms/frame | After FPS | After ms/frame | FPS factor |
| --- | ---: | ---: | ---: | ---: | ---: |
| Idle level0 | 8.15 | 122.74 | 59.79 | 16.73 | 7.34× |
| Holding fire | 0.659 | 1517.16 | 59.79 | 16.73 | 90.71× |

Mean tick time fell from **116.41 to 0.49 ms idle** and **1543.88 to 0.56 ms
firing**. Frame time fell 86.4% and 98.9%, respectively. After-change interval
rates were mostly 59.94–59.99 FPS, with occasional stalls (maximum 61.2 ms idle,
74.8 ms firing in the first after run). The tiny tick cost and ~16.7 ms spacing
are consistent with refresh-paced execution; no remaining sustained bottleneck
justifies simulation/render phase profiling or speculative renderer changes.

The first baseline run, before any logging edits, recorded 4.57 FPS / 219.01 ms
idle and 0.719 FPS / 1391.23 ms firing. The repeat confirms the severe slowdown,
but baseline variability means these factors are local observations, not a
portable benchmark. The shell's accumulated text and the game's variable-delta
simulation make elapsed time and input sequence more reproducible than exact
entity state. Long firing runs can include existing deaths/respawns.

Baseline repeat console traffic was about 64.7 lines/s idle and 35.8 lines/s
firing, despite extremely low FPS. The first baseline was 66.1 and 33.7 lines/s.
Afterward normal stdout consists of the ~1 Hz performance line, ~1 Hz entity
summary, and occasional lifecycle messages. The first after firing sample had
2.9 console lines/s. Idle console counts also include WebGL warnings and must
not be interpreted as application stdout alone; the final harness records
`stdoutLines` separately from total console lines.

**High-frequency logging was the primary cause.** Only diagnostic call sites
were gated between the measured builds. The collapse in synchronous tick cost
and recovery to ~60 FPS directly demonstrate the cost of logging through the
default shell. We did not separately apportion string formatting, WASM/JS
crossings, console transport, and textarea layout, so the measurements establish
the combined logging path's cost rather than a precise share for each component.

## Validation and limits

- Native: `cmake -S . -B build`, `cmake --build build -j4`, and
  `ctest --test-dir build --output-on-failure`: **77/77 passed**, 1.10 seconds.
- Preprocessor probe verified browser optimized builds omit debug calls and
  argument evaluation; browser Debug and native Release retain the original
  `Logger::Write` call. Native gameplay/rendering code is unchanged.
- Initial harness runs captured complete numerical samples, then failed during
  screenshot or shutdown checks. Screenshot capture now uses the page directly.
  Code inspection confirmed existing Escape behavior: playing → menu, then a
  second Escape → task shutdown. The harness now follows that sequence; no game
  input/lifecycle behavior was changed to satisfy automation.
- Existing `WebGL: INVALID_ENUM: disable: invalid capability` warnings occur in
  both builds. The faster build reaches Chromium's 256-warning suppression limit.
  They are not JavaScript exceptions and are not a demonstrated performance
  bottleneck after the logging fix. A favicon 404 also occurs in both builds.
  These warnings were present during the performance measurements; the follow-up
  below records their subsequent fix.
- No terrain, enemy rendering, HUD/menu, texture, lighting, effect, or audio work.
- No commit was made. Existing unrelated worktree files were left untouched.

The final full run completed successfully: **59.79 FPS idle and firing**, average
16.73/16.72 ms, mean tick 0.48/0.56 ms; no JavaScript page errors. Application
stdout was **3.8 lines/s idle and 2.9 lines/s firing**, including lifecycle output.
Escape through the existing menu sequence stopped the loop; no performance
reports appeared during the 1.5-second post-shutdown observation.

Tank/turret and bullets remained visible; mouse movement changed the relative
body/turret view. Entity summaries showed live bullet counts changing during
firing. Screenshots: [idle](browser-performance-idle.png),
[firing](browser-performance-firing.png), [after mouse movement](browser-performance-rotated.png).
The full interval measurements are in [the results JSON](browser-runtime-performance.json).
The focused performance change is ready to commit.

## Exact files changed

- `src/main.cpp`: browser interval/tick monitoring.
- `src/Logger.h`: centralized browser optimized-build debug gate.
- `src/PlayerManager.cpp`, `src/Player.cpp`, `src/Tank.cpp`, `src/GameWorld.cpp`,
  `src/Bullet.cpp`, `src/collision/CollisionSystem.cpp`,
  `src/combat/CombatSystem.cpp`: classify debug call sites through that gate.
- `tools/browser-performance.cjs`: repeatable measurements and browser checks.
- `documentation/browser-runtime-performance.md`: this audit/report.
- `documentation/browser-runtime-performance.json`: captured interval data.
- `documentation/browser-performance-idle.png`,
  `documentation/browser-performance-firing.png`,
  `documentation/browser-performance-rotated.png`: final browser evidence.

Generated browser/native build outputs are not source changes. The pre-existing
modified `runtime/applog.txt` and unrelated untracked files are excluded.

## Follow-up: WebGL INVALID_ENUM cleanup

The warning came from `BulletRenderer::SetupBulletRendering()` calling
`glDisable(GL_TEXTURE_2D)`. A temporary browser capability-call trace captured
118 calls to `disable(3553)` during a short level0 idle/firing reproduction.
`GL_TEXTURE_2D` is a desktop fixed-function capability, not a WebGL enable/disable
capability. The browser's bullet shader is already untextured.

The call is now guarded for desktop builds, matching the existing platform
boundary in the player renderer. Native state changes and browser geometry are
unchanged. Repeating the capability trace found zero invalid calls.

`tools/browser-performance.cjs` now records `gl.getError()` and context-loss state
after each measured phase, outside the timed interval. It fails on WebGL console
diagnostics, GL errors, context loss, JavaScript exceptions, missing frames, or
failure to stop. It does not suppress warnings or drain errors every frame.

Validation on the same visible Chromium/AMD WebGL setup:

- 20 seconds idle and 20 seconds firing: **59.79 FPS / 16.72 ms** in both phases.
- `gl.getError()`: **GL_NO_ERROR (0)** after both phases; no context loss.
- **Zero WebGL console diagnostics and zero JavaScript exceptions** from startup
  through shutdown. The unrelated favicon 404 remains.
- Tank, turret input, and bullets remain visible and functional; the existing
  two-Escape shutdown sequence completes and frame reports stop.
- Native configure/build and full CTest suite: **77/77 passed**, 0.79 seconds.

Follow-up files: `src/rendering/BulletRenderer.cpp`,
`tools/browser-performance.cjs`, and this report. No renderer features added;
no commit made.
