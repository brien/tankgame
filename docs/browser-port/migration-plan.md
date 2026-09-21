# Tankgame browser migration plan

## Principles and continuous gates

This is a future implementation sequence, not work performed by this analysis.

1. Keep gameplay in C++ and compile it to WebAssembly.
2. Preserve a working Linux, macOS, and Windows application at every merge point.
3. Separate behavioral baselines from modernization: capture native behavior before changing rendering.
4. Prefer a shared modern renderer; a retained native compatibility backend is an acceptable transition, never a reason to stop testing a native platform.
5. Make each phase selectable at build/run time until it passes parity gates. Avoid a flag maze by retiring the old path only after coverage proves the replacement.
6. Add CI build matrices for native compilers and Emscripten early. Rendering still requires smoke tests on real/virtual GPUs beyond compile-only CI.

## Phase 0 — Baseline and executable-path discovery

**Objective:** establish what currently builds, runs, and renders before planning deletions or parity claims.

**Affected:** CMake/Visual Studio projects (observation only initially), `App`, task graph, `GraphicsTask`, `RenderingPipeline`, every renderer/resource loader, representative runtime assets.

**Prerequisites:** Linux/macOS/Windows build environments and reproducible launch from `runtime`.

**Work:**

- Record native build commands/dependency versions for all three OSes.
- Exercise title/menu; one- and two-player/split-screen; versus; representative levels, tank types, bullets, effects, items, HUD, fonts, sound and debug rendering.
- Instrument whether legacy `GraphicsTask` functions and duplicated renderer implementations are reached.
- Capture reference screenshots/frame traces and audio/input behavior.
- Add/extend unit tests for GSM parsing, TGA decoding, level parsing, scene extraction, task ordering and deterministic simulation.

**Unknowns:** current Windows CMake executable naming, active Assimp need, whether all current paths are healthy, and the `LoadOBJ(.gsm)` inconsistency.

**Success criteria:** a written behavioral matrix, passing baseline tests, known active rendering call graph, and native artifacts on Linux/macOS/Windows.

**Desktop gate:** this phase changes no behavior; all three builds/run-smokes become mandatory for later phases.

## Phase 1 — Portable build graph and non-rendering WebAssembly compile

**Objective:** compile core engine/game code with Emscripten without claiming a playable port.

**Affected:** future CMake target/source grouping; core gameplay, events, collision/combat, data extractors, loaders; dependencies.

**Prerequisites:** Phase 0 source-path map.

**Work:**

- Split logical targets: portable core, resource decoding, renderer interface, native application/backend, browser application/backend.
- Add an Emscripten toolchain configuration and feature flags; do not make native `find_package` branches less strict.
- Determine whether Assimp is truly needed; either build its Emscripten-supported configuration or exclude only proven-unused code from the browser target.
- Compile parser/simulation tests under Emscripten/Node where practical; use warnings and sanitizers on native to expose UB.

**Unknowns:** transitive SDL_ttf/mixer packaging, compiler assumptions, parser alignment/ownership issues.

**Success criteria:** portable C++ library and tests compile to WASM; no gameplay code is replaced with JS; native build matrix remains green.

**Desktop gate:** native CMake and Visual Studio configurations link the same portable core and pass prior tests.

## Phase 2 — Shared frame lifecycle with browser scheduler

**Objective:** run initialization and exactly one game tick per browser callback without blocking the page.

**Affected:** conceptually `main.cpp`, `App`, `TaskHandler`, `ITask`, `GlobalTimer`; browser shell configuration.

**Prerequisites:** Phase 1 WASM link.

**Work:**

- Factor `TaskHandler::Execute()` semantics into `Tick()` plus native loop. Preserve sorted numerical priority/order and kill cleanup.
- Browser startup waits for filesystem/dependency readiness, creates the app, then registers a callback using Emscripten's main-loop API.
- Define graceful stop/cancel behavior and initialization failure reporting instead of raw `exit`.
- Reset/clamp timer on long callback gaps; define hidden-tab pause policy.

**Unknowns:** intended input/render/swap latency from actual task order and whether any code assumes `Execute()` never returns.

**Success criteria:** menu simulation can advance (even with rendering stubbed), shutdown is clean, the page remains responsive, timing tests pass.

**Desktop gate:** native executable calls the same `Tick()` from its normal owning loop and matches task-order tests on Linux/macOS/Windows.

## Phase 3 — SDL canvas, input, focus and pointer lock

**Objective:** create a browser graphics surface and make menu/basic tank input observable in C++.

**Affected:** `VideoTask`, `InputTask`, input handlers, `GameTask` menu transitions.

**Prerequisites:** browser scheduler and placeholder backend.

**Work:**

- Add platform context configuration: WebGL attributes in browser; existing or modernized native contexts per selected renderer strategy.
- Pump SDL events, track resize/focus/controller events, and retain keyboard snapshots.
- Implement click-to-play pointer-lock acquisition/loss and focus UI; defer relative mode until gesture.
- Validate keyboard, mouse buttons/deltas and a representative standard gamepad. Preserve GameCube/native handlers.

**Unknowns:** SDL joystick hat/mapping behavior in browsers and browser shortcut conflicts.

**Success criteria:** canvas initializes; keyboard navigates menu; pointer lock yields relative motion; controller input is logged/acted upon where supported; blur clears input.

**Desktop gate:** window/fullscreen/relative mouse and existing controller mappings still work on every native OS.

## Phase 4 — Renderer proof foundation

**Objective:** validate the largest uncertainty with one representative modern draw path, not a partial full port.

**Affected:** new backend boundary after `SceneDataBuilder`, CPU matrix library, `igtl_qmesh` data access, TGA decoding/upload, minimal shaders and buffers.

**Prerequisites:** phases 0–3 and selected WebGL baseline.

**Work:**

- Load one shipped `.gsm` through the real GSM parser and one shipped `.tga` through the real/shared decoder.
- Upload indexed mesh data and texture to VBO/IBO/texture; render with basic model/view/projection matrices and representative winding/depth/cull state.
- Render the same fixture through the shared path on browser, Linux, macOS and Windows—or, for Option A, compare browser backend with the retained native reference.
- Exercise context/resource teardown and recreation.

**Unknowns:** actual GSM conventions, texture restrictions/alignment, GLSL dialect strategy, macOS context floor and visual parity.

**Success criteria:** textured representative mesh and camera appear correctly on all four targets; pixel/reference differences are understood; no legacy emulation is required.

**Desktop gate:** PoC is opt-in until all three native builds pass; old native backend remains default if Option A is used.

## Phase 5 — Resource and asset pipeline

**Objective:** make all required resources load through one manifest/ownership model.

**Affected:** `ResourceManager`, `TextureHandler`, `DisplayList`, `igtl_qmesh`, SDL_ttf font ownership, filesystem paths, runtime package.

**Prerequisites:** modern resource primitives from Phase 4.

**Work:**

- Separate CPU decode/load from GPU upload; replace display lists with mesh handles/buffers.
- Resolve the GSM-versus-OBJ loader mismatch based on baseline evidence.
- Define a resource root and manifest for levels, metadata, models, textures, fonts and active audio.
- Preload package before engine initialization with progress/errors; retain synchronous C++ reads afterward.
- Centralize font lifecycle and create cached glyph/atlas resources.
- Add WebGL context-loss recreation descriptions.

**Unknowns:** which legacy assets are active, NPOT texture dimensions/wrap needs, font glyph set, desired progressive versus monolithic download.

**Success criteria:** all baseline-required assets resolve by logical name in WASM and each desktop; decode tests and leak/error tests pass; unused codecs/assets are identified, not silently removed.

**Desktop gate:** same manifest/loaders and visual assets work on Linux/macOS/Windows; native package layout remains supported or migration is explicitly backward compatible.

## Phase 6 — Terrain, cameras, and split-screen

**Objective:** render complete levels with correct view/projection and viewport behavior.

**Affected:** `TerrainRenderer`, `LevelHandler::populateTerrainRenderData`, `CameraManager`, `ViewportManager`, `RenderingPipeline`.

**Prerequisites:** resource system and modern opaque/textured material path.

**Work:**

- Convert terrain surfaces, walls, boundary geometry, floating elements and water into static/dynamic batches.
- Reproduce level-dependent texture/color selection and winding.
- Compute view/projection matrices in C++; derive aspect per viewport.
- Implement one- and two-player viewport/scissor/clear behavior, resize, and high-DPI drawing-buffer policy.

**Unknowns:** which terrain portions change at runtime and should be rebuilt, current split-screen aspect quirks, water blend/depth semantics.

**Success criteria:** representative title/game levels match native references; collision terrain remains unchanged; resize/DPI and split-screen show correct aspect/seams.

**Desktop gate:** screenshots on all desktop OSes at standard and split-screen resolutions meet parity tolerances.

## Phase 7 — Tanks, items, bullets and effects

**Objective:** render all world objects and visual effects through the selected backend(s).

**Affected:** tank renderer variants, bullet/effect/item renderers, GSM meshes, procedural geometry, render DTOs/extractors.

**Prerequisites:** transforms/materials/batching and baseline coverage matrix.

**Work:**

- Consolidate geometry descriptions that are currently duplicated, without changing gameplay objects.
- Implement model transforms, tank body/turret/cannon variants, player/enemy colors and effects.
- Implement bullets/items, transparent/additive effects, debug lines and correct ordering/depth writes.
- Retain every visual field from render data and compare seeded scenes.

**Unknowns:** inactive duplicate variants, exact fixed-function lighting/color-material interaction, unsupported wide/stitpled-line appearance.

**Success criteria:** baseline entity/effect matrix is complete in browser and native captures; no direct GL remains in entity/simulation code; performance avoids per-vertex immediate emulation.

**Desktop gate:** native gameplay rules are untouched and visual golden/smoke tests cover Linux/macOS/Windows.

## Phase 8 — HUD, menu, text and complete rendering cutover

**Objective:** reproduce all 2D/UI output and make the new pipeline the complete path.

**Affected:** `HUDRenderer`, `MenuRenderer`, legacy HUD/menu/text methods in `GraphicsTask`, SDL_ttf/resource manager, scene UI data.

**Prerequisites:** font atlas/cache and explicit UI pass.

**Work:**

- Batch colored/textured UI quads and lines in an orthographic pass.
- Render player-local HUD per viewport and menu/debug/global UI at correct scope.
- Implement cached text from bundled fonts with alpha blending.
- Prove legacy rendering functions/resources are unreachable before disabling/removing them in a later implementation change.

**Unknowns:** intended ownership of raw `uiData`, any legacy-only debug/menu state, reference scaling at non-4:3 resolutions.

**Success criteria:** menu, FPS/debug text, bars/meters/icons and two-player HUD match behavior; resource counts stay stable over transitions.

**Desktop gate:** allow runtime/backend comparison during rollout; only switch native default after parity on all three OSes.

## Phase 9 — Audio and complete interaction lifecycle

**Objective:** preserve music/effects/positioning within browser restrictions.

**Affected:** `SoundTask`, music trigger in `App`, positional mixer calls in `Tank`, active OGG/WAV assets.

**Prerequisites:** click-to-play/user activation state.

**Work:**

- Build/test SDL_mixer and required codec support under Emscripten.
- Defer audio-context resume and initial music until activation; preserve requested state.
- Test channel allocation, pause/resume, positional calls, repeated game transitions and hidden-tab behavior.
- If SDL_mixer is unsuitable, create a C++ audio interface with maintained browser and native implementations; do not replace gameplay with JavaScript.

**Unknowns:** exact Emscripten SDL_mixer codec support/version and positional behavior.

**Success criteria:** referenced music/WAV effects play after activation with equivalent controls; failure degrades to the existing sound-disabled behavior rather than blocking play.

**Desktop gate:** existing SDL_mixer implementation/assets remain available on Linux/macOS/Windows and pass audio smoke tests.

## Phase 10 — Persistence, loading, browser polish, and release hardening

**Objective:** production-quality browser lifecycle without changing native behavior.

**Affected:** settings/logging/resource package, fullscreen, resize, focus, HTML shell/service delivery, error reporting.

**Prerequisites:** feature-complete browser build.

**Work:**

- Choose read-only defaults or persisted settings via an explicitly synchronized browser filesystem.
- Add download/loading progress, cache/version strategy, failures and retry.
- Finish fullscreen, pointer-lock state, responsive resize/high DPI, hidden-tab pause/audio, context loss and accessibility/focus cues.
- Optimize bundle/resources based on traced active manifest; serve with correct MIME/compression/cross-origin policy where relevant.
- Run browser matrix and long-session/leak/performance tests.

**Unknowns:** hosting constraints, browser/device support target, persistence requirements (no save-game system currently exists).

**Success criteria:** cold/warm start, loss/recovery, focus/fullscreen/input/audio and both game modes pass on supported browsers; release artifacts are reproducible.

**Desktop gate:** release process continues producing and smoke-testing Linux, macOS and Windows artifacts from the same revision.

## Smallest useful proof of concept

The PoC should be a temporary/isolated build path, not a half-ported game and not a browser-only architectural commitment. It should answer the high-risk questions with this vertical slice:

1. Compile portable core, GSM/TGA/level loaders, SDL2 window/input, and minimal backend with Emscripten.
2. Preload exactly the title or one representative level plus one tank `.gsm`, one `.tga`, and the needed font only if text is part of the test.
3. Initialize an SDL canvas/WebGL context after preload.
4. Register a browser-driven one-frame task callback and show that `GameTask` state can advance without blocking.
5. Load the real level data and expose its terrain dimensions/state (full terrain rendering is not required for the smallest slice).
6. Load and render one representative textured GSM mesh through shaders and VBO/IBO with a C++ camera/projection.
7. Use keyboard input to move/rotate the camera or representative tank state; acquire relative mouse only after click.
8. Render the identical fixture via the shared modern path on Linux, macOS and Windows, while keeping the existing game executable available.
9. Optionally play one WAV after the same user gesture; treat this as an audio feasibility sub-gate, not a requirement to validate rendering.

**PoC acceptance evidence:** build logs for four targets, browser console free of GL errors, loader assertions (vertex/triangle/texture dimensions), screenshots from all render targets, a task-order/timing trace, keyboard/pointer-lock observation, and green existing native tests. It should not attempt every level, HUD, effect, audio codec or persistence before the foundational go/no-go decision.

## Final acceptance matrix

A full port is complete only when:

- feature behavior covers title/menu, one/two-player and versus state, all reachable levels/entities/effects/items/HUD/debug paths;
- packaged assets and settings defaults behave equivalently;
- browser input/audio/lifecycle constraints have explicit UX;
- no required renderer depends on compatibility-only GL in the browser;
- Linux, macOS, and Windows applications are built and runtime-smoke-tested, not merely assumed from shared source;
- visual differences are either within approved tolerance or deliberately documented;
- browser and native builds are produced from one C++ gameplay codebase.
