# Browser and Linux regression review — 2026-09-29

Revision: `6fd947fd2ec8355c941db8b58f4d2f0816143e2b` (`Fix split-screen framebuffer clearing`).

## Assessment

**The framebuffer-clearing fix passes visual checks, but the game does not receive a full functionality sign-off.** Both player views render in browser WebGL, Linux default, and Linux modern mode. However, the longer gameplay/menu sequences exposed a browser trap and a Linux modern segmentation fault. Selecting single-player after a two-player game also retains the split-screen layout.

No production code was changed. This review records failures for follow-up; successful repeat runs do not invalidate the failures.

**Planning update:** multiplayer robustness is **deferred / low priority** and
outside the current browser-port critical path. These findings may reflect older
multiplayer/session-state architecture; crash attribution remains unconfirmed.
Continue renderer/resource convergence and revisit multiplayer later as its own
subsystem. See the [current roadmap](../../docs/browser-port/current-state-roadmap.md#current-runtime-findings-and-multiplayer-priority-2026-09-29)
and [known issues](../../docs/browser-port/risks.md#current-known-issues-2026-09-29).
This prioritization does not change the failed observations or grant multiplayer
sign-off.

## Results

| Check | Browser | Linux default | Linux modern |
| --- | --- | --- | --- |
| Release build | Pass | Pass | Same native binary |
| Startup, single-player meshes/textures, movement, rotation, jump, projectiles | Pass | Pass | Pass |
| Co-op: upper and lower views survive repeated frames, movement, camera changes and firing | Pass in fresh run | Pass | Pass in fresh run |
| Versus menu selection: both views render and update | Pass in fresh run | Pass | Pass in fresh run |
| Sustained single-player firing and respawn | 3 × 60 seconds pass; 9 reloads | 60 seconds pass | 60 seconds pass |
| Longer sequence through restart and two-player modes | **Fail: WASM trap** | Completed without crash | **Fail: SIGSEGV** |
| Select single-player after two-player | **Fail: retains split-screen** | **Fail: retains split-screen** | **Fail: retains split-screen** |
| Clean shutdown | All three firing runs and both fresh mode runs pass; failed sequence trapped | Exit 0 | Both fresh mode runs exit 0; original sequence crashed |
| Independent player-2 controls | Not tested: joystick unavailable | Not tested: joystick unavailable | Not tested: joystick unavailable |

Native CTest: **84/84 passed**, including all three viewport tests and `EventLifecycleTest.LevelReloadDiscardsEventsForDestroyedEntities`. See [tests.log](tests.log). These tests do not cover the runtime failures below.

### Split-screen evidence

Player 0 occupies the lower half and player 1 the upper half. Screenshots taken before and after movement, camera elevation changes, and firing show both views surviving. Linux default HUDs remain inside their respective halves. No clear-related flicker, stale trails, or obvious depth-order regression was seen in the sampled captures; this was not a continuous video or exhaustive occlusion test.

| Renderer | Co-op after movement/firing | Versus after movement/firing |
| --- | --- | --- |
| Browser | [Co-op](browser-fresh-coop-later.png) | [Versus](browser-fresh-versus-later.png) |
| Linux default | [Co-op](compatibility-coop-later.png) | [Versus](compatibility-versus-later.png) |
| Linux modern | [Co-op](modern-fresh-coop-later.png) | [Versus](modern-fresh-versus-later.png) |

The modern run's original `modern-coop-idle.png` has only one visible tank immediately before the crash. Fresh mode captures show both views correctly; disappearance around a gameplay/lifetime failure must not be confused with the previous unconditional full-frame clear.

## Failures requiring follow-up

### 1. Runtime crashes in two-player/transition sequences — deferred / low priority

**Browser:** `browser-driver.cjs` starts single-player, exercises controls and firing, returns to the menu, restarts, then enters co-op and versus. During versus firing, it recorded `RuntimeError: table index is out of bounds`, beginning at WASM function 2215, offset `0x104a49`. The logs immediately preceding the failure include player-0 and player-1 respawn reloads. The loop could not be shut down normally.

- [Driver and failure stack](browser-driver.log)
- [Full console, asset responses and graphics probes](browser-console.json)
- [Condensed reload/crash events](browser-crash-events.log)

**Linux modern:** `native-driver.py modern` completed 60 seconds of firing with six respawns, then movement/jump, menu return, restart, and entry into co-op. The game disappeared after the initial co-op captures. The driver's `BadWindow` error was a consequence of the game crash, not sufficient evidence on its own; the system crash record confirms **SIGSEGV** in this call chain:

```text
CollisionSystem::CheckSphereCollision
CollisionSystem::Initialize callback
EventBus::Dispatch
Bullet::NextFrame
GameWorld::UpdateEntitiesWithCleanup<Bullet>
GameWorld::Update
GameTask::HandlePlayingState
```

- [System crash report with demangled stack](modern-crash-demangled.txt)
- [Driver output](modern-driver.log), [key lifecycle events](modern-events.log), [full compressed game log](modern.log.gz)

Fresh browser and Linux modern co-op/versus sessions completed without crashes, so these are sequence/timing-sensitive failures. No exact recurrence rate or shared root cause is established. Source inspection identifies a relevant lifetime risk: the respawn path releases only the respawning player's tank before `NextLevel` clears every world tank, potentially leaving the other player's controlled-tank pointer stale (`PlayerManager.cpp:203`, `LevelHandler.cpp:228`). This is a follow-up hypothesis, not a sanitizer-confirmed diagnosis of these two crashes. Clearing queued events alone does not establish safety of all player/collision pointers.

The failing sequences are retained as runnable drivers. They may require repetition; preserve both the failed and successful evidence. The browser `versus-firing` screenshot was captured after an exception and must not be treated as a successful live frame.

### 2. Single-player selection retains two players — deferred / low priority

Reproduction from a fresh process/page:

1. Enter single-player, then press Escape to return to the menu.
2. Press Right, Enter to start co-op.
3. Press Escape, Left, Enter to select single-player again.
4. The game retains two half-height viewports rather than returning to a full-frame single-player view.

Confirmed visually on all three renderers. A separate browser check without sustained combat measured the final active GL viewport:

| State | GL viewport `(x, y, width, height)` |
| --- | --- |
| Initial single-player | `(0, 0, 1280, 720)` |
| Co-op | `(0, 360, 1280, 360)` |
| Single-player selected afterward | `(0, 360, 1280, 360)` |

See [measurements](browser-transition.json), [reproduction driver](browser-transition.cjs), and [resulting frame](browser-transition-selected-single.png).

`GameTask::HandleMenuState` sets two players for multiplayer selections but otherwise clamps the existing count instead of resetting it to one (`GameTask.cpp:152–158`). Git blame dates that behavior to 2025 or earlier, before the recent browser fixes. This is separate from the framebuffer clear correction.

## Build, runtime and performance evidence

- Reconfigured both existing build directories before building: Linux with `CMAKE_BUILD_TYPE=Release`, browser with `BUILD_TESTS=OFF`, `CMAKE_BUILD_TYPE=Release`, and its existing Emscripten toolchain. Both used `FETCHCONTENT_UPDATES_DISCONNECTED=ON` and `cmake --build … -j4`. Configure/build logs are in this directory. Emscripten linking required access to its SDK cache outside the workspace.
- Native runs used `runtime/tankgame-linux` with `runtime/` as the working directory, SDL X11, 1280×720, default compatibility or `TANKGAME_RENDERER=modern`.
- Browser files were served by `python3 -m http.server 8766 --directory runtime`. Headed Chromium **153.0.8010.12** used WebGL 1.0 through ANGLE on **AMD Custom GPU 0405 / radeonsi vangogh ACO**. The canvas framebuffer was 1280×720.
- Browser firing runs averaged **59.81–59.82 FPS** across recorded performance intervals. Each ran at least 60 seconds, crossed three respawns, reported no exceptions, returned `NO_ERROR`, retained its context, and stopped its frame loop normally. WASM heap samples remained **18,350,080 bytes**. See [summary](browser-fire-summary.json) and the full `browser-fire/` records.
- All captured browser GL probes returned zero; the failed browser smoke run had no failed game-asset HTTP response. This does not override its WASM failure. Native logs contained no reported shader/GL initialization failure; native `glGetError` was not separately instrumented and native frame pacing was not benchmarked.
- [Artifact hashes](artifact-manifest.json) identify the tested executable, tests, HTML, JS, WASM and data. Old symbol/source-map files in `runtime/` were not used to resolve the current browser stack.
- Native logs were compressed to retain the complete diagnostic output. Existing local files and previous review evidence were preserved; `runtime/applog.txt` was restored byte-for-byte to its pre-review contents. Build outputs were refreshed.

## Limits

This is a keyboard/mouse regression review, not a complete playtest. Player 2 is assigned generic joystick input, and no input devices were exposed in this environment. Independent player-2 movement, firing and camera controls therefore remain unverified. Selecting versus was tested for rendering and stability, not for scoring/rule correctness.

Modern/browser terrain, enemies, HUD/menu, lighting, and browser audio remain deferred by the current port scope. They were not classified as new failures. Audible sound, physical controllers, all-level traversal, long sessions, resize/high-DPI, context recovery, macOS and Windows were not tested.

**Next work:** continue the browser-port renderer/resource convergence roadmap.
Keep the verified framebuffer-clear fix and retain these failures as explicit
limitations. Multiplayer robustness is deferred / low priority; do not launch a
multiplayer rewrite or mix it into renderer migration. When multiplayer is
revisited as a separate subsystem, investigate entity lifetimes and session/mode
reset behavior, repeat the failing sequences, and validate independent player-2
input with a joystick before claiming a clean baseline.
