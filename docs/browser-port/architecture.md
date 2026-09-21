# Repository architecture and portability analysis

## Runtime topology

```text
main
  -> App::Run
     -> construct VideoTask, GraphicsTask, SoundTask, GameTask,
        GlobalTimer, InputTask; construct TankHandler/LevelHandler/TaskHandler
     -> AddTask() calls Start() immediately and inserts by priority
     -> GameTask::OnResume(); SoundTask::PlayMusic()
     -> TaskHandler::Execute()
          repeated frame: Timer -> Game -> Sound -> Graphics -> Input -> Video
          then stop/delete tasks marked canKill

GameTask
  -> state machine: MENU / PLAYING / GAME_OVER
  -> GameWorld (entity managers + CollisionSystem + CombatSystem)
  -> PlayerManager
  -> LevelHandler and TankHandler singletons
  -> GraphicsTask receives GameWorld pointer

GraphicsTask
  -> SceneDataBuilder reads game/level/player state into SceneData
  -> RenderingPipeline dispatches Terrain/Tank/Bullet/Effect/Item/HUD renderers
  -> ResourceManager owns textures, meshes, fonts and display-list wrappers
  -> VideoTask swaps SDL GL window after GraphicsTask due to task priority order
```

### Ordering observation

`TaskHandler::AddTask()` inserts before the first task whose numeric priority is greater, so the configured values execute as `GlobalTimer(10)`, `GameTask(60)`, `SoundTask(70)`, `GraphicsTask(80)`, `InputTask(90)`, `VideoTask(100)`, not in the textual add order. This means a frame updates timing and simulation before rendering, polls input late for the next frame, then swaps. Any future `Tick()` extraction must preserve the *actual list order*, not the order shown in `App::Run`.

## Subsystems

### Application startup and shutdown

- `main.cpp`: platform GL headers, `main`, `App::Run`, global singleton construction, task creation/priority assignment, music start, and entry to `TaskHandler::Execute()`.
- `App.h`: service-locator-like singleton exposing raw pointers to every major task.
- `TaskHandler.{h,cpp}` / `ITask.h`: task lifecycle. `AddTask()` synchronously invokes `Start()`. `Execute()` blocks until every task is killed; killed tasks receive `Stop()` and are deleted. Escape in `GameTask::HandleMenuState()` invokes `KillAllTasks()`.
- Shutdown is task-driven rather than SDL quit-event-driven; `InputTask::Update()` calls `SDL_PumpEvents()` but not `SDL_PollEvent()`, so window-close/focus/controller events are not explicitly processed.

**Browser implication:** extract exactly one pass over tasks (plus kill cleanup) as a reusable tick. Native can loop around it; the browser registers it with `emscripten_set_main_loop_arg` or an equivalent callback. Do not unwind into a perpetual loop or call blocking sleeps. On shutdown, cancel the browser main loop, run the same `Stop()` path, and retain the page rather than assuming process exit.

### Video and window lifecycle

`VideoTask.cpp` initializes SDL video/joystick, requests a desktop OpenGL 2.1 context, reads `settings.txt` with `FILE*`, chooses fullscreen and resolution, sets relative mouse mode, creates `SDL_WINDOW_OPENGL`, makes the context current, enables swap interval, swaps each update, and destroys context/window on stop.

Browser adaptation points are canvas size, WebGL context attributes, resize/fullscreen events, and user-gesture pointer lock. `SDL_GL_SwapWindow` remains a harmless/appropriate frame boundary under Emscripten depending on selected SDL integration. The unused `SDL_Renderer*` should not be mistaken for an active SDL renderer.

### Game state and simulation

- `GameTask.cpp` owns the high-level state machine. `Start()` loads the title level, initializes `GameWorld`, and wires it to graphics/handlers. `SetUpGame()` loads `levels/level0@@.txt`, initializes tanks, and spawns players. `Update()` handles common commands then menu, play, or game-over behavior.
- `GameWorld.cpp` updates collision, all entity managers, and interaction handling. `collision/` and `combat/` provide separated systems; `events/` carries collision/game notifications.
- `PlayerManager.cpp` owns player tanks/input selection and per-frame player work. `TankHandler.cpp` creates enemy tank state. `Tank.cpp`, `Bullet.cpp`, `FX.cpp`, and `Item.cpp` implement most behavior.
- `LevelHandler.cpp` parses level grids and optional metadata, serves collision/height data, creates items/effects, and populates `TerrainRenderData`.

The simulation does not require browser APIs. Coupling through global singletons is testability debt but not a port blocker.

### Rendering

The active path in `GraphicsTask::Update()` clears/configures fixed-function state, updates cameras, builds a `SceneData` snapshot, and calls `RenderingPipeline::RenderAllPlayerViews()`. This is encouraging separation at the data-flow level. However, the called renderer classes still use direct compatibility OpenGL; “new pipeline” means organization, not modern graphics APIs.

Principal roles:

- `rendering/RenderData.h`: data transfer objects for tanks, bullets, effects, items, terrain, cameras and UI.
- `SceneDataBuilder.*` plus extractors: convert mutable game objects into render data.
- `RenderingPipeline.*`: pass/order orchestration, viewport/camera setup, lighting, objects and UI.
- `ViewportManager.*` / `CameraManager.*`: per-player view configuration and camera state.
- `ResourceManager.*`: duplicated texture/font/mesh loading and display-list construction.
- `TerrainRenderer`, tank renderer variants, bullet/effect/item renderers, HUD/menu renderers: visual implementation.
- `GraphicsTask.cpp`: still contains a large older renderer, display-list construction and text/HUD/menu methods. Much appears unused by the active pipeline, but must be proven with call-graph/runtime tests before deletion or exclusion.

### Rendering boundary trace

| Origin | Current rendering relationship | Direct GL observed? |
|---|---|---|
| `GameTask` | Sets flags on `GraphicsTask`, supplies `GameWorld`; no geometry calls. | No. |
| `TankHandler` | Supplies enemy tank state to extractors/fallback. | It includes platform GL headers, but no active GL calls found. |
| `LevelHandler` | Supplies terrain grids/flags via `populateTerrainRenderData`; old comments mention former drawing. | No active GL calls found. |
| Effects/entities | `GameWorld` state is copied by `EffectDataExtractor`, `BulletDataExtractor`, `ItemDataExtractor`, `TankDataExtractor`. | No direct entity GL calls found in `Tank.cpp`, `Bullet.cpp`, `FX.cpp`, or `Item.cpp`. |
| Mesh class | Stores/parser mesh data and also exposes `Draw*`. | Yes, `igtl_qmesh.cpp` uses immediate mode extensively. |
| Texture class | Loads TGA and creates GL textures. | Yes, `TextureHandler.cpp`. |
| Graphics orchestration | Builds scene and invokes renderer classes. | Yes, `GraphicsTask.cpp`, `RenderingPipeline.cpp`, `BaseRenderer.cpp`. |
| Concrete renderer classes | Terrain, tank, bullet, effect, item, HUD/menu. | Yes, throughout `src/rendering/*.cpp`. |
| Display-list wrapper | Encapsulates list allocation/compile/call. | Yes, `DisplayList.cpp`. |

Thus GL is **not spread through gameplay logic**, which makes a second backend practical, but it is spread broadly through presentation/resource code rather than isolated behind one API. The minimum useful boundary is immediately after `SceneDataBuilder`: a platform-neutral `IRenderBackend::Render(const SceneData&, const FrameContext&)` plus resource interfaces. Camera matrices, viewport rectangles and UI commands should be data, not hidden global GL mutations. Mesh draw methods should become CPU data access/upload; textures/fonts should yield backend-neutral handles.

### Input

- `InputTask` snapshots SDL keyboard state, relative mouse deltas/buttons, and up to four `SDL_Joystick*` devices.
- `KeyboardMouseInputHandler`, `GenericJoystickInputHandler`, and `GameCubeInputHandler` translate state into tank actions.
- `InputHandlerFactory` chooses implementations; `Tank::HandleInput()` also contains debug/scancode handling.

Emscripten SDL supplies keyboard/mouse/gamepad integration, but current startup requests relative mouse immediately, before a user gesture. Browser code needs a click-to-play/focus state and pointer-lock failure handling. SDL event polling/hot-plug support should be added conceptually without changing native mappings. Fixed-size arrays and `joynames[2]` versus four joystick slots deserve validation.

### Audio

`SoundTask` initializes SDL audio and SDL_mixer, loads one OGG music track and WAV effects, allocates 16 channels, and offers music/channel controls. `Tank.cpp` calls `Mix_SetPosition` for spatial behavior. There is no audio thread owned by the game; SDL_mixer/browser audio implementation handles playback. Browser audio cannot begin until user activation, so `App::Run`'s immediate `PlayMusic(0)` must become a resumable pending request in the browser path. Native behavior stays immediate.

### Timing

`GlobalTimer` uses `SDL_GetTicks()`, computes `dT` and FPS, and is itself a task. Movement code consumes `GlobalTimer::dT`. Browser background tabs may throttle callbacks and produce a large next delta; the migration must define pause-on-hidden or clamp/reset on resume. Desktop timing remains SDL-based.

### Levels, assets, and filesystem

- Runtime assumes its current directory is `runtime/`; asset paths are relative (`levels/...`, `texture/...`, `sounds/...`, `fonts/...`, top-level `.gsm`).
- `LevelHandler::Load()` uses `FILE*` parsing for `.txt`; `LoadMetadata()` derives/opens companion `.json` files. Existing levels include text for all shipped levels and JSON only for a subset.
- `TextureHandler` has a custom TGA loader for uncompressed/RLE 16-, 24-, and 32-bit data and uploads pixels directly.
- `igtl_QGLMesh` loads/saves custom `.gsm` and supports OBJ import/export in C++ streams. Shipped models are `.gsm`.
- `GraphicsTask::PrepareMesh()` correctly opens a named file and calls `LoadGSM(FILE*)`; `ResourceManager::PrepareMesh()` currently calls `LoadOBJ(fileName)` even though callers pass `.gsm`. This active-path inconsistency is an early rendering/resource risk to verify, not fix in this analysis.
- Fonts are bundled TTF. Both `GraphicsTask` and `ResourceManager` initialize/open fonts, creating duplicated ownership/lifecycle.
- `Logger` writes `applog.txt`; `VideoTask` reads `settings.txt`. No save-game system was found.

Emscripten's preloaded virtual filesystem can preserve synchronous reads after startup. Package `runtime` at its expected virtual root or introduce a shared resource-root abstraction. Preload completion must precede task `Start()`, and a page loading/progress state should cover download/mount. Persistent settings, if desired, need IDBFS/OPFS synchronization; native remains normal files.

### Platform-specific and external behavior

- `_WIN32`, `__APPLE__`, and Linux branches largely select GL/GLU headers; Windows includes `windows.h` first.
- `main.cpp` includes `<mach-o/dyld.h>` but no use was found in the inspected source.
- CMake links SDL2, SDL2_mixer, SDL2_ttf, desktop OpenGL, Assimp, `CMAKE_DL_LIBS`, and Linux GLU. Assimp headers appear in `GraphicsTask.cpp`, but no active Assimp import call was found there; establish whether the dependency is actually required before solving it for Emscripten.
- No application networking and no explicit `std::thread`/pthread use were found. Dynamic-loading behavior is only a build-system link (`CMAKE_DL_LIBS`), not an observed game call.
- `GraphicsTask` calls `exit(1)` when its font fails. Browser lifecycle should return initialization failure and display an error instead; native can retain equivalent failure semantics.

## Asset inventory and compatibility

| Asset | Shipped | Loader/use | Browser disposition |
|---|---:|---|---|
| `.tga` | 30 | Custom `TextureHandler::Load_TGA`; GL upload | Package directly; test RLE/16/24/32-bit cases and WebGL unpack alignment. Prefer shared offline conversion to PNG only if it benefits all targets and visual equivalence is tested. |
| `.gsm` | 9 | `igtl_QGLMesh::LoadGSM(FILE*)` | Package directly; parser is C++. Convert CPU vectors into VBO/index buffers in the renderer. Validate endianness/field widths from parser rather than assuming. |
| level `.txt` | 26 | `LevelHandler::Load()` | Package/preload directly; synchronous access is suitable once mounted. |
| level `.json` | 9 | `LevelHandler::LoadMetadata()` | Package as optional companions. Confirm parser behavior and default equivalence for levels without JSON. |
| `.ttf` | 2 (+ license text) | SDL_ttf | Package directly; rasterize into texture atlas/surfaces and upload through common renderer. |
| `.wav` | 16 | SDL_mixer effects | Usually package directly; validate Emscripten mixer/backend decode. |
| `.ogg` | 3 | SDL_mixer music (one referenced) | Validate browser codec/mixer build; convert to an equally supported shared format only if required. |
| tracker/audio (`.it`, `.xm`, `.s3m`, `.aiff`) | 12 | No active load path found except WAV/OGG references | Do not package blindly; confirm whether legacy/unreferenced. Browser codec support is not assumed. |
| `.jpg` | 1 | No active load found | Confirm whether legacy/unreferenced. |
| `settings.txt` | 1 | `VideoTask` fixed-line parser | Defaults can ship read-only; user changes need browser persistence policy. |

## Browser behavior matrix

| Behavior | Repository relevance | Browser plan | Preserve desktop |
|---|---|---|---|
| Fullscreen | `settings.txt` toggles `SDL_WINDOW_FULLSCREEN`. | Request only from a user gesture; handle denial and canvas fallback. | Keep SDL native fullscreen setting. |
| Pointer lock | Relative mode is requested in constructors/startup. | Click-to-play acquires pointer lock; release on Escape/menu; show state if denied/lost. | Keep current relative SDL mouse behavior. |
| Keyboard focus | Polling assumes active app. | Focus canvas on activation; prevent browser shortcuts only where safe; clear stale state on blur. | Native window focus semantics unchanged. |
| Gamepads | SDL joystick polling and special GameCube mapping. | Enumerate/connect after user interaction; test SDL mapping, hot-plug, axes/hats and browser privacy behavior. | Preserve native SDL joystick implementations and mappings. |
| Audio autoplay | Music starts before interaction. | Defer/resume audio/music after click/key activation; make repeated resume idempotent. | Native music may continue to start during initialization. |
| Save/settings | Reads settings; no save-game path found. | Decide read-only defaults vs persistent IDBFS settings and explicit sync. | Keep normal native files. |
| Loading | Many synchronous assets are loaded during task start. | Preload package before engine initialization and present progress/error UI. | Native starts synchronously as now. |
| Resize/high DPI | Width/height are static settings; no resize event path. | Track CSS vs drawable pixels, update canvas, viewport and projection, with chosen pixel-ratio cap. | Native fixed-size behavior can remain; shared resize support must not regress it. |
| Split screen | `ViewportManager` supports one/two-player layouts; game clamps players to two. | Compute viewports from current framebuffer dimensions; validate seams/aspect after resize/DPI. | Use identical viewport algorithm on desktops. |
| Lost focus/tab hidden | No explicit pause path. | Pause/reset timer and audio or define continued simulation; avoid huge `dT` on return. | Apply only on focus events; native pause policy can remain current unless intentionally unified. |
