# R5 title menu text — 2026-10-27

Modern native and Emscripten now render a global mode legend using the existing
font atlas and glyph geometry. The snapshot's unused placeholder labels said
Start Game / Options / Quit; they now match GameTask's actual mode choices:
Single player / Co-op / Versus. Selected text uses cyan and a `>` marker; normal
and disabled entries use the snapshot colours. Navigation and gameplay are
unchanged.

The legend sits at the upper left, above the diagnostic rows, complementing the
title level's terrain lettering. It renders once through the existing global menu
pass and hides with the menu. Empty viewports and hidden menus produce no lines;
incomplete option arrays are bounded safely. Missing enabled flags default to
enabled. The shared text pass preserves depth and clips to the global viewport
and incoming scissor. No new GPU resources are needed.

Compatibility remains the native default and retains its previous visual output.
This completes the title menu text slice within R5. Compatibility retirement and
broader visual/platform qualification remain open; Windows/macOS runtime and
multiplayer robustness deferrals remain unchanged.

## Validation

- Native and Emscripten builds pass; all 140 GL-enabled native tests pass. Two new
  tests cover actual mode labels, selection, colours, visibility, dimensions and
  incomplete snapshots. Initial sandboxed GL tests could not initialize SDL;
  rerunning with display access passed.
- Six Linux modern fixtures cover every selection with one and two views. Each
  pipeline image exactly matches one independently submitted global menu, has
  visible glyphs, preserves depth and restores the player viewport. All GL probes
  are clean. Six compatibility fixtures pass the same global reference checks.
- Browser navigation through all three modes, gameplay, firing and shutdown pass.
  Seven probes report zero GL errors and no context loss. Menu navigation uploads
  no buffers or textures; after level entry the upload counters remain at four
  buffers and zero textures. Menu glyph submissions stop during gameplay.
- Screenshots include all browser selections and native fixture captures. Host
  Electron warnings are retained in browser-run.txt; the smoke report has no
  errors. This instrumented run is not a performance benchmark.

## Reproduction

From the repository root:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/menu-text-2026-10-27/build-fixture.py
```

From `runtime/`, with display access:

```sh
TANKGAME_RENDERER=modern ../build/menu-text-fixture
env -u TANKGAME_RENDERER ../build/menu-text-fixture
```

From the root (adjust SDK/Electron paths for the local installation):

```sh
python3 documentation/menu-text-2026-10-27/capture-png.py
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
```

With the server running:

```sh
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox documentation/menu-text-2026-10-27/browser-smoke.cjs
```
