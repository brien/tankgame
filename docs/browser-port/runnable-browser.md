# Runnable browser bootstrap milestone

## Baseline and lifecycle

Before this milestone the Emscripten link emitted only
`runtime/tankgame-linux.js` and `runtime/tankgame-linux.wasm`. There was no HTML
entry point or preloaded filesystem, and `App::Run` entered
`TaskHandler::Execute`, whose native `while` loop never returned while tasks
were alive. Loading that output as a browser application therefore could not
provide the runtime files startup expects or yield control to the page.

`TaskHandler::Tick` now performs exactly one ordered update pass and the
existing killed-task cleanup pass. Native `Execute` is still the owner of the
blocking loop and simply calls `Tick` until no tasks remain, preserving desktop
lifecycle and task order. In an Emscripten build, initialization runs once and
`emscripten_set_main_loop_arg` schedules `App::Tick` at the browser's animation
rate. The callback returns after every frame. When tasks finish it cancels the
browser loop and uses the normal task shutdown path.

The console prints `Browser first frame completed: WebGL clear/render/swap path
executed.` once, after the first complete task tick. This is a bootstrap
diagnostic, not proof that all scene renderers are available.

## SDL, input, and audio

Emscripten requests an OpenGL ES 2.0 context through SDL. SDL associates its
window with the Emscripten shell's `#canvas`; the configured initial drawing
size is 1280 by 720, and the initial OpenGL viewport uses the same dimensions.
Window and context failures include `SDL_GetError()` in the console log.

SDL continues to pump keyboard, mouse, and joystick state once per task tick.
Browser startup does not request relative mouse mode because pointer lock needs
a user gesture. No polished focus, pointer-lock, or resize UI is included.

Audio initialization and music playback are disabled only under Emscripten.
This prevents autoplay/user-activation policy from blocking the first frame.
Desktop builds retain their existing SDL_mixer startup and playback behavior.

## Output and packaged files

The browser executable has an `.html` suffix, so Emscripten generates its
standard HTML canvas shell and companion files in `runtime/`:

- `tankgame-linux.html` — browser entry point and canvas shell;
- `tankgame-linux.js` — loader and generated runtime;
- `tankgame-linux.wasm` — application code;
- `tankgame-linux.data` — preloaded virtual filesystem.

The data file contains only startup and first-game resources at their existing
absolute virtual paths:

- `/settings.txt`;
- `/fonts/` (the two shipped fonts and their license);
- `/texture/` (the currently eager texture-loader input set);
- `/levels/title@@.txt` and `/levels/title@@.json`;
- `/levels/level0@@.txt` and `/levels/level0@@.json`;
- `/nowbody.gsm`, `/nowturret.gsm`, `/cannon.gsm`, and `/body.gsm`.

The texture tree is packaged because startup still prepares those resources,
although the current browser shaders do not sample them. Only the title and
first playable level are included; later levels are not yet a coherent browser
runtime. Sound files are intentionally omitted because browser audio is
disabled. Emscripten finishes mounting the preload data before calling C++
`main`, so existing synchronous relative-path loads remain valid.

## Build and manual run

From the repository root, with Emscripten 3.1.64 activated:

```sh
rm -rf build-browser
emcmake cmake -S . -B build-browser -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-browser -j4
python3 -m http.server 8000 --directory runtime
```

Open <http://localhost:8000/tankgame-linux.html>. Do not use `file://`, because
the browser must fetch the `.js`, `.wasm`, and `.data` companions over HTTP.

The expected first frame is a 1280x720 dark-blue WebGL canvas. The menu is
intentionally not visible. The console should report the SDL version, context
creation, deferred renderer notices, successful pipeline/game initialization,
registration of the non-blocking loop, and the one-time first-frame message.
There should be no fatal asset, shader, or context error.

Terrain, player/enemy tanks, effects, HUD, menus, texture sampling, lighting,
audio, polished pointer lock, and responsive/high-DPI canvas resizing remain
deferred. Bullet and item submissions use the migrated backend when those
entities exist, but the initial menu state need not contain either one. The
next renderer milestone should begin only after this bootstrap page has been
manually checked in a real browser.

## First-frame stack overflow diagnosis

The initial bootstrap build reached loop registration but trapped on its first
frame. The original Release artifacts reproduced these exact offsets in a
locally HTTP-served Chromium window:

| WASM offset | Resolved function |
| --- | --- |
| `0x2e36` | `Logger::Get()` |
| `0xbb955` | `GraphicsTask::Update()` (inlined scene/terrain construction) |
| `0xfb6fd` | `BrowserFrame(void*)` (inlined `App::Tick` / `TaskHandler::Tick`) |

Relinking with `--emit-symbol-map` changes optimized function ordering; its
indices cannot be applied directly to the old binary. Matching the original
and symbolized functions' instruction sequences resolves the offsets above.
An unoptimized build with stack checks exposes the earlier failure:

```text
BrowserFrame
  App::Tick
    TaskHandler::Tick
      GraphicsTask::Update
        GraphicsTask::RenderWithNewPipeline
          stack overflow while reserving the local SceneData
```

`SceneData::terrain` contains two `int[128][128]` arrays (131,072 bytes
combined). WASM disassembly shows a 131,248-byte stack frame for
`RenderWithNewPipeline` and a nested 131,232-byte frame in
`SceneDataBuilder::BuildScene` for the terrain temporary. These are bounded
allocations, not recursion or a dangling callback. They exceed Emscripten
3.1.64's default 65,536-byte stack. Terrain data is still constructed even
though terrain drawing is deferred.

Without stack checks, the original optimized build writes below its stack
into static data. Its eventual trap is in `Logger::Get`, called by the
inlined `LevelHandler::populateTerrainRenderData` color diagnostic, after
scene/terrain initialization has already corrupted memory. With
`STACK_OVERFLOW_CHECK=2`, execution aborts at the render function's stack
reservation, before that corruption.

At the original trap, the logger initialization guard at address 439364 and
the stream-construction table entry at address 413764 both read zero. The
instruction at `0x2e36` loads through that table entry minus 12, producing the
invalid address `0xfffffff4`. With the stack fix, the guard is 1 and the table
entry remains 413788. The logger is the victim of the overflow.

The browser target now explicitly reserves a 1 MiB stack, accommodating the
roughly 257 KiB of nested render frames plus callers and library work.
`ALLOW_MEMORY_GROWTH` does not enlarge the stack. This target-only link setting
preserves the native code, lifecycle, and rendering scope. No task priority,
input handling, timing, or renderer implementation changes are needed. Update
order remains timer (10), game (60), sound (70), graphics (80), input (90),
and swap (100); the callback's `App` remains heap-allocated.

### Debug reproduction

With the SDK activated, configure a separate build directory:

```sh
emcmake cmake -S . -B build-browser-debug \
  -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_CXX_FLAGS_DEBUG=-O0 -gsource-map -g' \
  '-DCMAKE_EXE_LINKER_FLAGS=-sASSERTIONS=2 -sSTACK_OVERFLOW_CHECK=2 -gsource-map -g'
cmake --build build-browser-debug -j4
python3 -m http.server 8000 --directory runtime
```

Both build directories write to `runtime/`; the last linked configuration is
the one served. Keep the `.wasm.map` alongside its matching WASM file. The
debug flags supply function names, debug information, a source map, and
checks on stack-pointer changes; SAFE_HEAP is not needed for this diagnosis.
Removing only the explicit `STACK_SIZE` link setting reproduces the overflow.
Restore it before verification.

Verification after the fix: the native Linux game and test executable build,
and all 77 CTest tests pass. A headed Chromium run of the instrumented Debug
build completed 1,709 frames over approximately 30 seconds without a WASM
exception or stack-check failure. The first-frame diagnostic appeared once,
and visual inspection confirmed the expected 1280x720 dark-blue canvas.
The optimized Release build also ran over 1,700 frames without an exception.
Pressing Escape exercised task cleanup and callback cancellation; the frame
counter remained at 1,736 on subsequent samples.
