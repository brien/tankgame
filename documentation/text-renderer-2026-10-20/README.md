# R5 cached diagnostic text — 2026-10-20–21

Modern native and Emscripten now display player-local FPS and global debug text
using the bundled DroidSansMono font, the resource catalogue and the existing
shared shader. Compatibility remains the native default and is unchanged visually;
its text methods were placeholders, so these captures establish new text behavior
rather than claiming parity with an existing visible reference.

`FontAtlas` builds a GL-free RGBA image and glyph geometry for printable ASCII.
SDL_ttf/font/surface lifetimes are scoped to atlas construction, including failure
paths. Unsupported bytes map to `?`. ResourceManager owns one atlas texture and
95 immutable glyph buffers, released before the renderer/context. Text uses the
existing program, straight-alpha blending, no lighting/depth test/depth writes,
and restores changed texture/blend/depth/cull state. Changing numeric strings
requires no texture/buffer uploads; submission currently uses one draw per visible
glyph, not one batch per string. The atlas uses padded cells, linear sampling,
clamp-to-edge and no mipmaps, including on WebGL 1 NPOT textures.

FPS appears at the upper right of each player viewport in 18-drawable-pixel text.
Debug lines render once after all player views against the whole drawable; they
are enabled by the existing title-menu I control. Menu selection scope is unchanged.
The diagnostics display existing DTO fields: Calls/Tris are still zero placeholders
from HUDDataExtractor, not newly implemented renderer telemetry. Responsive font
scaling, localization/Unicode, additional UI text, menu/global UI consolidation,
legacy GraphicsTask font retirement, context-loss recovery and compatibility
retirement remain follow-ups. Windows/macOS runtime and multiplayer robustness
remain deferred.

The recreation fixture also exposed an existing catalogue lifecycle bug: Close
nulled geometry facades subsequently reused by Initialize. Cleanup now replaces
these with empty facades, releasing GPU objects while allowing reinitialization.
This does not implement browser context-loss recovery or redesign texture cleanup.

## Validation

- Native and Emscripten Release builds pass.
- **133/133 native tests pass with OpenGL enabled**, including three new tests for
  real font decoding/UV bounds/alpha coverage, failure cleanup and fallback bytes,
  and player/global diagnostic visibility flags.
- Linux modern fixtures at 801x601 show one/two local FPS regions, a global debug
  block and upper/lowercase glyphs. They assert visible pixels, stable atlas identity,
  unchanged depth, restored depth/cull/blend state and GL_NO_ERROR. Cleanup followed
  by reinitialization and another text draw exits successfully. PNGs retain output.
- Electron browser title/start/firing/idle probes show text, existing HUD/targeting,
  clean WebGL/context checks, stable uploads after level entry and clean shutdown.
  See `browser-smoke.json` and the gameplay screenshots. This is one browser/runtime
  smoke, not broad device or performance qualification.

## Reproduction

From the repository root:

```sh
cmake -S . -B build
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/text-renderer-2026-10-20/build-fixture.py
cmake -S . -B build-browser
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
```

From runtime with display access: `TANKGAME_RENDERER=modern ../build/text-fixture`.
The fixture writes BMPs; retained PNGs are lossless conversions using the preceding
HUD slice's stdlib BMP/PNG helpers. Serve runtime on port 8766, then run:

```sh
env -u ELECTRON_RUN_AS_NODE /path/to/electron --no-sandbox documentation/text-renderer-2026-10-20/browser-smoke.cjs
```
