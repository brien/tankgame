# tankgame

A small cross-platform 3D tank game built with C++, OpenGL, and SDL.

Originally started around 2003, this is a long-running personal project that I still revisit periodically for experimentation, refactoring, modernization, and testing.

Supports macOS, Linux, Windows, and browsers through Emscripten/WebAssembly.
Also uses SDL_ttf and SDL_mixer.

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
TANKGAME_RENDERER=modern ./tankgame-mac
```

On Windows, the current CMake target is also named `tankgame-linux`, producing
`tankgame-linux.exe`. With a multi-configuration generator such as Visual Studio,
build with `cmake --build build --config Release`, then run
`.\Release\tankgame-linux.exe` from the `runtime` directory.

macOS requires the modern renderer. Linux and Windows retain compatibility
rendering by default; set `TANKGAME_RENDERER=modern` to select the shared modern
renderer. The browser always uses modern rendering.

## Browser build and hosting

With Emscripten 3.1.64 activated (the version pinned in browser CI):

```sh
emcmake cmake -S . -B build-browser -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build-browser --parallel 4
python3 -m http.server 8000 --directory runtime
```

Open <http://localhost:8000/tankgame-linux.html>. Serve the files over HTTP/HTTPS;
opening the HTML directly with `file://` does not work. Use the page controls to
enable sound, capture the mouse or enter fullscreen. Settings reset on reload.

To package the completed build:

```sh
python3 tools/package-browser.py --output /tmp/tankgame-browser-release
```

The command prints the HTML path in a content-versioned directory containing the
matching HTML, JavaScript, WASM, data and hash manifest. Deploy that directory
intact over HTTPS. See the [hosting guide](documentation/browser-release-2026-10-28/README.md#hosting)
for MIME types and caching, and the [current port status](docs/browser-port/current-state-roadmap.md)
for acceptance and deferred qualification work.

## Test

From the repository root:

```sh
ctest --test-dir build --output-on-failure
```

For a multi-configuration build, add `-C Release` to match the build configuration.
