# tankgame

A small cross-platform 3D tank game built with C++, OpenGL, and SDL.

Originally started around 2003, this is a long-running personal project that I still revisit periodically for experimentation, refactoring, modernization, and testing.

Supports macOS, Linux, and Windows. Also uses SDL_ttf and SDL_mixer.

## Build

Install these prerequisites:

- CMake 3.14 or newer and a C++14 compiler.
- Git, for dependencies downloaded during configuration.
- SDL2, SDL2_mixer, and SDL2_ttf development libraries.
- OpenGL development libraries, plus GLU on Linux.

From the repository root, configure and build:

```sh
cmake -S . -B build
cmake --build build --parallel
```

CMake downloads GoogleTest and, if no suitable installed version is found,
nlohmann/json. The first configuration needs internet access for these downloads.
Tests are built by default; add `-DBUILD_TESTS=OFF` to the configure command to
build only the game.

## Run

Run from the `runtime` directory so the game can find its settings and assets.

Linux:

```sh
cd runtime
./tankgame-linux
```

macOS:

```sh
cd runtime
./tankgame-mac
```

On Windows, the current CMake target is also named `tankgame-linux`, producing
`tankgame-linux.exe`. With a multi-configuration generator such as Visual Studio,
build with `cmake --build build --config Release`, then run
`.\Release\tankgame-linux.exe` from the `runtime` directory.

## Test

From the repository root:

```sh
ctest --test-dir build --output-on-failure
```

For a multi-configuration build, add `-C Release` to match the build configuration.
