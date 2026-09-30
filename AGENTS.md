# Repository Guidelines

## Project Structure & Module Organization
- `src/`: Main C++ game code (tasks, gameplay, rendering, input, audio, events, collision/combat systems).
- `src/rendering/`: Newer data-driven rendering pipeline (`SceneDataBuilder`, `RenderingPipeline`, renderers).
- `src/events/`, `src/collision/`, `src/combat/`: Event bus and gameplay systems.
- `tests/`: GoogleTest test sources (`test_main.cpp`, `test_player.cpp`).
- `runtime/`: Runtime assets and built executables (`tankgame-linux`, `tankgame-mac`, sounds, textures, levels, fonts).
- `documentation/`: Refactor plans and implementation notes.

## Build, Test, and Development Commands
- Configure: `cmake -S . -B build`
  - Generates build files and finds SDL2/OpenGL/assimp dependencies.
- Build: `cmake --build build -j`
  - Builds game and test binaries.
- Run game (Linux): `./runtime/tankgame-linux`
- Run game (macOS): `./runtime/tankgame-mac`
- Run tests: `ctest --test-dir build --output-on-failure`
  - Executes discovered GoogleTest suites.

## Coding Style & Naming Conventions
- Language: C++14 (`CMAKE_CXX_STANDARD 14`).
- Indentation: 4 spaces; keep brace style and spacing consistent with surrounding code.
- Types/classes: `PascalCase` (`GameWorld`, `PlayerManager`).
- Methods/functions: `PascalCase` for class members (`SetGameWorld`, `NextFrame`).
- Files: typically `PascalCase.cpp/.h` matching class names.
- Prefer existing logging path: `Logger::Get().Write(...)` (avoid ad-hoc `printf`).

## Testing Guidelines
- Framework: GoogleTest via CMake FetchContent.
- Add tests in `tests/test_*.cpp`; keep names descriptive (e.g., `PlayerTest, AddScore_IncreasesScore`).
- Prefer focused unit tests around `Player`, `GameWorld`, and system boundaries.
- Run tests locally before opening a PR; use `--output-on-failure` for actionable logs.

## Commit & Pull Request Guidelines
- Commit style in history is short, imperative, and scoped (e.g., `Refactor Player tank ids`).
- Keep commits focused; separate refactors from behavior fixes when practical.
- PRs should include:
  - What changed and why.
  - Risk/impact notes (rendering, input, gameplay behavior).
  - Test evidence (`ctest` output or equivalent).
  - Screenshots/videos for visible rendering or UI changes.
