# Browser and Linux manual visual review — 2026-09-28

Revision: `58a38d8044dc20c2c547962b53a3e0027c80abac`.

## Assessment

The shared player/material milestone is visibly present in browser and Linux modern mode. The game does **not** receive an unconditional runtime sign-off: split-screen loses the lower player view, and one browser run trapped with a memory access error. A repeat browser run completed successfully. These findings should be resolved or explicitly tracked before treating the baseline as healthy.

This validates the narrow “renderer convergence milestone 2” in `docs/browser-port/current-state-roadmap.md`. It does not complete revised R2 (all-desktop backend contract) or R3 (complete opaque world pass). macOS/Windows remain unvalidated. Modern terrain, enemies, HUD/menu, lighting and browser audio remain deferred as planned.

## Method and builds

Real Linux windows and headed Chromium were exercised with injected keyboard/mouse input; screenshots were manually inspected by the assistant. This is a visual smoke review, not a human audio/controller playtest or exhaustive gameplay certification. Temporary Python X11 and Playwright drivers are retained here for the tested input sequences.

- Linux: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_UPDATES_DISCONNECTED=ON`, then `cmake --build build -j4`.
- Tests: `ctest --test-dir build --output-on-failure`: **80/80 passed**; see `tests.log`.
- Browser: reconfigured existing Emscripten build with `cmake -S . -B build-browser -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_UPDATES_DISCONNECTED=ON`, then built with the installed SDK cache. HTML/JS/WASM/data linked successfully.
- Existing browser build initially omitted new sources because CMake uses a configure-time glob. Reconfiguration fixed the undefined symbols; this was a stale build directory, not a source compile failure.
- Linux ran from `runtime`, default compatibility and `TANKGAME_RENDERER=modern`, SDL X11, 1280×720.
- Browser: locally served runtime, Chromium 153.0.8010.12, ANGLE / AMD Custom GPU 0405 / radeonsi vangogh ACO, WebGL.

## Observations

| Check | Browser | Linux default | Linux modern |
| --- | --- | --- | --- |
| Startup and level 0 | Pass; intentionally blank title | Pass; visible title/menu and terrain | Pass; intentionally blank title |
| Player body/turret, ring/star textures | Visible | Visible with richer legacy rendering | Visible; same modern visual design as browser |
| Firing | Visible bullets; longer-run exception noted below | Visible gameplay/projectiles | Visible bullets |
| Input and transitions | Successful repeat exercised W/D, mouse, jump, menu return, restart, co-op and versus selection | Movement/rotation visible; menu return/restart exercised | Key/mouse sequence exercised; camera tracking limits movement conclusions from still images |
| Shutdown | Repeat logged browser loop stopped | Focused repeat exited 0 | Focused repeat exited 0 |
| Split-screen | Lower half blank in co-op and versus | Lower half blank in co-op | Lower half blank in initial co-op capture |

Browser successful repeat: 33 performance intervals averaged **59.70 FPS** across the run (including transitions), no page exceptions, WebGL probe `NO_ERROR`, context not lost. The only HTTP 404 was `/favicon.ico`, confirmed by the local server log; all game assets returned 200. No shader/GL failure was found in native logs; native GL errors were not separately instrumented and native frame pacing was not measured.

## Findings

### 1. Split-screen clears the first player's view

Start a two-player game from the title (Right, Enter). Only the upper player viewport remains visible; the lower half is background. Browser versus shows the same defect. See `browser-two-player.png`, `browser-versus.png`, and `compatibility-two-player.png`.

`RenderingPipeline::RenderAllPlayerViews` calls `RenderScene` for each player. Each scene calls `ClearBuffers`, which clears the entire colour/depth framebuffer. `ViewportManager::SetActiveViewport` sets the viewport but no scissor. A viewport alone does not restrict a clear. The second pass therefore erases the first. These lines date to commit `62c42669` (2025-07-22), preceding the recent browser changes. Fix by clearing once per frame or restricting each clear to its viewport, then validate both players' HUD/camera/input.

### 2. Intermittent browser memory trap during firing

First run: open page, Enter, hold left mouse for approximately 20 seconds. A `RuntimeError: memory access out of bounds` occurred; performance logging stopped after about nine gameplay intervals, and later controls/shutdown could not complete. Preserved in `browser-first-console.json`.

A repeat of sustained firing, with explicit canvas focus before keyboard controls, completed the full sequence without an exception. This does not clear the first failure. The first logger retained the exception message but not its WASM stack, so root cause and attribution to recent changes remain unknown. Next step: reproduce with matching debug symbols and stack/heap checks. Do not interpret screenshots taken after the first trap as successful input tests; the browser screenshots retained here are from the successful repeat.

### 3. Current-status documentation contradicts its milestone updates

The roadmap's opening assessment and older inventory/phase table still describe texture sampling as absent and the modern renderer as browser-only, while its newer milestone sections correctly describe the shared Linux renderer and texture path. Read the milestone sections as the current status and reconcile those summaries in a documentation follow-up.

## Limits and next steps

No audible sound assessment, gamepad test, item fixture, long session, all-level traversal, context-loss recovery, resize/DPI validation, or macOS/Windows run was performed. Native versus was not independently confirmed. Initial native input/capture attempts lost reliable transition tracking; focused repeats completed with exit 0. No production code was changed for this review. Builds and game runs refreshed runtime binaries and `runtime/applog.txt` (the log already had local changes before review).

Prioritize the browser memory trap and existing split-screen defect. Keep the native compatibility backend. Complete the cross-desktop R2 gates before claiming broader convergence; continue the shared material/resource work within the documented limited scope.
