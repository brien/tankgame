# Tankgame browser-port feasibility

## Scope and evidence standard

This document assesses a C++/Emscripten/WebAssembly port while preserving supported Linux, macOS, and Windows desktop applications. It is a plan, not an implementation. Statements labelled **Observed** come from the repository as it stands; statements labelled **Recommendation** describe a future design. No JavaScript rewrite is proposed. Small Emscripten-generated glue is an output of the toolchain, not a rewrite of the game.

Related detail is in [architecture.md](architecture.md), [graphics-audit.md](graphics-audit.md), [migration-plan.md](migration-plan.md), and [risks.md](risks.md).

## Executive summary

- **Technically feasible:** yes. The application is C++14, already uses SDL2, has no required networking or worker threads, loads ordinary local assets, and has a frame-oriented task model. Emscripten supports the relevant language/library surface and SDL input/window integration. The browser target will require a new build configuration, a browser-owned main-loop adapter, packaged assets, audio/input lifecycle work, and—by far the largest item—a WebGL-compatible renderer.
- **JavaScript rewrite:** unnecessary. Gameplay, entity management, collision, combat, level interpretation, math, event dispatch, camera state, and render-data extraction can remain C++ and compile to WebAssembly. Browser shell/glue should be minimal and may be generated/configured by Emscripten.
- **Mostly reusable C++:** `GameWorld`, `EntityManager`, collision/combat systems, `Tank`/`Bullet`/`FX`/`Item` simulation, `PlayerManager`, most of `LevelHandler`, `TankHandler`, cameras, event bus, custom GSM parsing, and render-data DTO/extractors.
- **Adapt rather than rewrite:** `App`/`TaskHandler` lifecycle, SDL input and window setup, timing, settings/filesystem paths, SDL_mixer audio, SDL_ttf font rasterization, asset packaging, resize/fullscreen/focus behavior, and build configuration.
- **Substantial replacement:** the actual renderer. It relies throughout on OpenGL 2.1 compatibility-profile features: immediate mode, matrix stacks, fixed-function lighting/material/texture state, display lists, line stipple, GLU projection/mipmap helpers, and client-wide implicit state. None is a sound WebGL target.
- **Largest risk/work:** reproducing the visual behavior of terrain, tanks, effects, HUD/menu/text, and split-screen while replacing roughly two thousand direct GL/GLU call sites/pattern occurrences across both `GraphicsTask.cpp` and `src/rendering/`. The newer rendering pipeline creates a useful data boundary, but its renderer implementations remain legacy GL and duplicate geometry.
- **Reasonable project:** yes, as a staged renderer/lifecycle migration with native regression gates. It is not a “recompile only” port. The simulation is substantially more portable than the presentation layer, so a whole-game rewrite would be disproportionate.
- **Desktop preservation:** prefer one C++ shader/VBO renderer using an OpenGL ES 2.0/WebGL 1-compatible baseline (or a carefully defined ES 3/WebGL 2 baseline after capability validation), used through SDL-created contexts on Linux, macOS, and Windows and WebGL in Emscripten. An alternative is a render backend interface with the existing compatibility renderer retained on desktop and a modern WebGL backend; this lowers initial desktop regression risk but doubles renderer maintenance. Neither option removes any native target.

## Relative sizing

| Area | Size | Basis |
|---|---:|---|
| C++ toolchain/build target | Medium | CMake assumes native packages, OpenGL, GLU, Assimp, and native executable naming. |
| Browser lifecycle/main loop | Medium | `TaskHandler::Execute()` owns a blocking `while`, but each iteration is already frame-shaped. |
| Core simulation | Small | Predominantly platform-neutral C++ containers/math and ordinary update calls. |
| Input/browser focus | Medium | SDL maps much of it; relative mouse, pointer lock, gamepad connection, focus, and quit semantics need policy. |
| Audio | Medium | SDL_mixer API is compact, but browser codecs/autoplay and async user activation must be validated. |
| Assets/filesystem | Medium | Paths are relative and synchronous; preload packaging fits this, but settings/save persistence needs a decision. |
| Renderer | Very large | Compatibility GL and GLU are deeply used by every rendering pass and resource preparation. |
| Browser polish | Medium | Resize, high DPI, loading, fullscreen, focus pause, and split-screen need explicit behavior. |

## Recommended direction

### Preferred end state: shared modern C++ renderer

Build on the existing `SceneDataBuilder -> SceneData -> RenderingPipeline` direction, but replace its legacy-GL implementation with explicit resources and commands:

1. Keep simulation independent from graphics APIs.
2. Make a renderer-facing immutable snapshot containing camera, terrain, entity, effects, and UI data.
3. Introduce explicit mesh/texture/font handles and render passes; no GL types in simulation-facing headers.
4. Use shaders, vertex/index buffers, vertex attributes, explicit uniforms, and CPU-computed matrices.
5. Target a declared common subset supported by Emscripten WebGL and native SDL GL contexts on Linux, macOS, and Windows.
6. Preserve visual semantics with golden screenshots and deterministic scene fixtures on every target.

This is cleaner and avoids maintaining two descriptions of every effect. A conservative WebGL 1 / OpenGL ES 2 baseline maximizes reach; WebGL 2 / ES 3 may simplify formats and instancing but should only be selected after the PoC validates required browser/device support. Native platforms can request a compatible core context and load functions in a cross-platform way; macOS's deprecated desktop OpenGL still supports sufficient shader/VBO functionality for this game's scope, though exact context/version policy must be tested.

### Transitional direction: two maintained backends

Define the same renderer-facing snapshot and resource interfaces, leave the existing OpenGL 2.1 implementation as the desktop backend, and implement a WebGL backend. This keeps Linux/macOS/Windows behavior stable during bring-up. It should be treated as a transition or an intentional long-term cost: every visual fix must be made and verified twice, and current rendering code must first be disentangled from global OpenGL state.

## Feasibility conditions / go-no-go gates

Proceed to a full port only after a proof of concept confirms all of the following:

- Emscripten compiles the non-rendering C++ graph without replacing gameplay with JavaScript.
- An SDL canvas and browser-scheduled frame callback can execute one task-list tick without recursive/blocking control flow.
- A real existing `.txt` level and representative `.gsm` mesh load from a preloaded virtual filesystem.
- A shader/VBO path renders the mesh with an existing TGA texture and the expected winding/coordinate conventions.
- Keyboard plus user-gesture pointer lock work, and at least one supported browser gamepad path is observed.
- SDL_ttf can rasterize the bundled font, and SDL_mixer (or a portable C++-facing alternative) can start after user activation.
- The same renderer or retained native backend continues to build and render on Linux, macOS, and Windows.

Failure of an optional audio codec or a browser gamepad mapping is a scoped adaptation, not evidence that WebAssembly is infeasible. Failure to reproduce representative geometry/effects through a modern renderer would materially change the project size and is therefore the most important early gate.

## Portability classification

### A. Likely reusable unchanged or nearly unchanged

| Subsystem/files | Repository evidence | Caveat |
|---|---|---|
| Entity storage: `Entity.h`, `EntityManager.h`, `GameWorld.{h,cpp}` | Uses C++ ownership/containers and calls entity updates, collision, combat, and events; no GL calls. | Audit lifetime/iteration behavior under sanitizers before relying on identical WASM behavior. |
| Collision/combat: `collision/*`, `combat/*`, `TankCollisionHelper.*` | Pure game-state operations and event integration; no platform APIs found. | Floating-point results can differ slightly; preserve tolerances. |
| Events: `events/*` | Header-level C++ event types/bus with no native dependency. | Clear queued events at the same lifecycle points. |
| Entity simulation: most of `Bullet.cpp`, `FX.cpp`, `Item.cpp`, `Tank.cpp` | Movement, AI, damage, spawning, and state updates are C++. `Tank.cpp` includes GL headers but the audited file has no active GL calls; that include can ultimately be decoupled. | Tank input references SDL scancodes and sound positioning, so the class is not completely platform-pure. |
| Player/enemy management: `PlayerManager.*`, most of `TankHandler.*`, `TankTypeManager.*` | C++ arrays/state/spawn policy; no active drawing in the handler. | Global singleton/App coupling remains an architecture issue, not a WebAssembly blocker. |
| Cameras/math: `Camera.*`, `math.h`, `Color.h` | Plain numeric state. | Projection/view matrix construction must move from GLU/fixed function into the renderer. |
| Render DTOs/extractors: `rendering/RenderData.h`, `*DataExtractor.*`, much of `SceneDataBuilder.*` | Already copies gameplay state into rendering-specific structures. | `SceneDataBuilder` still reaches through `App` to `GraphicsTask::cams` and owns a raw `uiData` pointer; strengthen the boundary later. |
| GSM data/parser portions: `igtl_qmesh.*` | Custom mesh vectors, GSM load/save, normals and mesh processing are C++. | Its `Draw*` methods directly invoke immediate-mode GL and must be separated/replaced. |

### B. Reusable with minor/moderate adaptation

| Subsystem/files | Why adaptation is needed | Cross-platform preservation |
|---|---|---|
| Startup/task scheduling: `main.cpp`, `App.h`, `TaskHandler.*` | Replace the browser build's blocking `Execute()` ownership with initialize/tick/shutdown hooks registered using Emscripten's main-loop API. Keep task priority and update ordering. | Native `Execute()` remains, or both native/browser use the same one-frame `Tick()` with different schedulers. |
| SDL window/context: `VideoTask.*` | Browser canvas sizing/context attributes/fullscreen differ; current code requests GL 2.1, hides cursor conditionally, and swaps each frame. | Compile-time platform configuration behind `VideoTask`; SDL continues to create native windows on all desktops. |
| Input: `InputTask.*`, `KeyboardMouseInputHandler.*`, joystick handlers | SDL keyboard is suitable, but pointer lock requires a user gesture and gamepads are hot-plugged/browser-mapped. The current fixed arrays and polling need bounds/event review. | Keep SDL APIs and existing native mappings; browser-specific policy only around acquisition/focus/mapping. |
| Timing: `GlobalTimer.*` | SDL ticks are portable, but tab throttling and long suspension gaps require clamping/pause semantics. | Use one timing policy expressed in C++, with browser visibility feeding pause/resume. |
| Audio: `SoundTask.*`, positional calls in `Tank.cpp` | SDL_mixer availability/build flags, browser user activation, codec support, and channel positioning need validation. | Preserve SDL_mixer native path; either build its Emscripten port/codecs or put audio behind a common C++ interface. |
| Fonts: `GraphicsTask`, `ResourceManager`, HUD renderers | SDL_ttf can provide pixels, but current upload/drawing is legacy GL and font init/ownership is duplicated. | Shared atlas/raster path can run on all targets; retain bundled TTF files. |
| Level/metadata loading: `LevelHandler.*` | Synchronous `FILE*`/streams and relative paths work once preloaded, but package paths and optional JSON behavior need normalization. | Use a shared virtual/resource root abstraction whose desktop implementation resolves the same files. |
| Logging/settings: `Logger.*`, `VideoTask.*` | `applog.txt` and `settings.txt` assume writable/current working directory. Browser persistence is not automatic. | Native paths remain unchanged behind platform storage policy; browser uses memory or explicitly synced persistent storage. |
| CMake/project files | Native `find_package`/`find_library`, GLU, Assimp and executable naming are not Emscripten-ready. | Add, in a future implementation, an Emscripten toolchain branch without weakening existing native branches or Visual Studio support. |

### C. Substantial redesign/replacement

- `GraphicsTask.cpp`: legacy setup, GLU projection, fixed lighting, matrix/state manipulation, immediate HUD/menu/text, blending, display-list construction, and legacy compatibility helpers.
- `rendering/BaseRenderer.cpp`, `RenderingPipeline.cpp`, `TerrainRenderer.cpp`, `TankRenderer.cpp`, `PlayerTankRenderer.cpp`, `EnemyTankRendererImpl.cpp`, `BulletRenderer.cpp`, `EffectRenderer.cpp`, `ItemRenderer.cpp`, `HUDRenderer.cpp`, and `MenuRenderer.cpp`: structurally separated classes but still direct fixed-function GL.
- `ResourceManager.cpp`, `DisplayList.*`: desktop display lists are absent from OpenGL ES/WebGL and cannot be emulated as a durable architecture; replace them with persistent vertex/index buffers and resource handles.
- GL drawing portion of `igtl_qmesh.cpp`: upload its CPU mesh data rather than issuing `glBegin`/`glVertex` per triangle.
- `TextureHandler.cpp`: retain/repair the CPU TGA decode as appropriate, but replace `gluBuild2DMipmaps`, legacy `GL_CLAMP`, implicit formats/state, and direct ownership with WebGL-safe upload plus `glGenerateMipmap` where legal.
