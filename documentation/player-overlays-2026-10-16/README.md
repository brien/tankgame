# R4 modern player-overlay ordering — 2026-10-16

The modern pipeline previously rendered each player's additive energy and targeting
overlays immediately after its mesh. Subsequent opaque enemy/item/bullet draws
could overwrite an overlay even where that overlay was in front of those objects.

`ITankRenderer` now supports separate opaque and overlay submissions, with default
methods retaining the existing combined behavior for other renderers.
`PlayerTankRendererImpl` implements the split while retaining combined `Render`
for direct callers. The modern pipeline submits player overlays after water and
effects, so effect outlines have also finished writing depth. Overlays retain
additive blending and `LEQUAL` depth testing without depth writes; afterward they
restore depth writes, `LESS`, disabled blending, standard blend factors and back
face culling with clockwise front faces. Existing animation advancement is retained.
Compatibility rendering and the native default are unchanged.

## Validation

- Native Release and Emscripten Release build/link succeed.
- All 119 native CTest cases pass with `TANKGAME_TEST_GL=1`.
- The Linux modern fixture uses a player and overlapping enemy viewed from two
  camera directions. The complete pipeline matches explicit opaque-then-overlay
  submission exactly, for both single and split viewports at 601x501.
- Reproducing the former interleaved submission differs at 228 pixels in the
  single view and 135 pixels in split views. This makes the fixture sensitive to
  restoring the old ordering, rather than merely checking that rendering succeeds.
- Overlay submission leaves depth pixels unchanged and restores depth-write,
  depth-function and blend-enable state. All fixture frames report GL_NO_ERROR.
- `modern-1.png` and `modern-2.png` capture the corrected pipeline.

The sandbox initially blocked SDL startup; the recorded passing checks ran with
desktop display access. The browser build needed SDK cache write access for linking.
No new browser runtime, Windows/macOS runtime or multiplayer stability claim is
made. The fixture isolates player/enemy overlap with an uncharged targeting ring;
charged-star animation, all item/bullet/effect combinations and full gameplay
remain broader coverage work. This advances R4 without closing it.

## Reproduction

From the repository root:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/player-overlays-2026-10-16/build-fixture.py
```

From `runtime/`, with desktop display access:

```sh
TANKGAME_RENDERER=modern ../build/overlay-fixture
```

The fixture writes BMP captures; checked-in PNGs are lossless RGB conversions.
Build the configured Emscripten target using its SDK environment:

```sh
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
```
