# Repository guidance for Copilot

Follow [AGENTS.md](../AGENTS.md) for repository conventions and
[README.md](../README.md) for current dependencies, platform launch commands,
configuration and browser packaging.

## Architecture

- C++14 game using SDL2, SDL_mixer, SDL_ttf and OpenGL, with an Emscripten browser target.
- Gameplay lives in `src/`; rendering in `src/rendering/`; event, collision and
  combat systems in their corresponding subdirectories.
- Keep gameplay independent of graphics APIs. Render data flows through
  `SceneDataBuilder`, `SceneData` and `RenderingPipeline` to specialized renderers.
- Modern rendering is the only backend. Use the existing shader/GPU resource
  abstractions, including GeometryAsset; legacy display lists, immediate-mode
  drawing, the tank renderer factory and GLU have been removed.
- Platform contexts are OpenGL 2.1/GLSL 1.20 on Linux/Windows, OpenGL 3.2
  core/GLSL 1.50 on macOS, and WebGL 1 in browsers. `TANKGAME_RENDERER` is ignored.
- Gameplay definitions are immutable per session. Follow the
  [configuration contract](../documentation/gameplay-configuration-contract.md)
  and [file format guide](../documentation/gameplay-configuration-format.md).
  Startup loads `gameplay.json`; failed loading logs a diagnostic and falls back
  to built-in definitions. Restart to apply edits.

## Build and validation

From the repository root:

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Tests use GoogleTest in `tests/test_*.cpp`. For graphics-enabled testing, set
`TANKGAME_TEST_GL=1` with an available OpenGL display/context. Linux CI uses Xvfb,
software OpenGL and dummy audio; see the [desktop workflow](workflows/desktop-ci.yml).
The [browser workflow](workflows/browser-ci.yml) builds and packages with Emscripten.
Run native executables from `runtime/` so assets resolve.

Use four-space indentation and existing naming conventions. Log through
`Logger::Get().Write(...)`. Add focused regression coverage for behavior changes
and record relevant build/test results. Dated documentation reports describe
historical revisions; use the [current port status](../docs/browser-port/current-state-roadmap.md)
for implementation completion and outstanding qualification gaps.
