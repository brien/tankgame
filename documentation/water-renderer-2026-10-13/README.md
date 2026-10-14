# R4 water slice, implemented 2026-10-13; validated 2026-10-14

The shared modern renderer now draws cached additive terrain ribbons after opaque
world objects. Compatibility retains its existing water submission inside terrain
rendering. This completes the initial water slice; the broader R4 milestone remains open.

## Implementation

- `TerrainGeometry` preserves the four legacy traversal directions, previous-height
  carry across rows, two-unit ribbon height, UVs, colours and upward lighting normal.
- `ResourceManager` owns the water geometry and material alongside the opaque
  terrain batches. The existing content cache covers water edits and reloads.
  Legacy texture slot 12 is `TEXTURE_EXIT`; material alpha is 0.5.
- Modern water uses depth testing, no depth writes, no culling, and
  `SRC_ALPHA, ONE` blending. It establishes depth writes, disabled blending,
  standard alpha blend factors, back-face culling and clockwise winding afterward.
  This is an explicit pass baseline, not arbitrary incoming-state restoration.
- Both water paths use `glPolygonOffset(-1, -1)` while drawing ribbons and disable
  polygon offset afterward. Adjacent unequal-depth pits place ribbons directly on
  wall planes; the offset avoids unstable depth rejection on those shared planes.
  This intentionally corrects the compatibility reference too, without changing
  geometry or blend/depth-write semantics. The original unmodified reference had
  up to 79 differing pixels per affected fixture, with channel differences up to 65.
- The shared shader now retains material alpha when RGB vertex colours exist.
  Previously those vertices forced alpha to 1, doubling water intensity.
- Two CPU tests cover isolated negative cells, all four ribbon directions/UVs,
  previous-height carry and cache invalidation. Gameplay code is unchanged.

## Validation

Native Release and Emscripten Release builds pass. All 113 native tests pass with
`TANKGAME_TEST_GL=1`, including GL startup. Six Linux fixture pairs return zero GL
errors. Modern water leaves the depth buffer unchanged, restores the documented
pass state (including disabled polygon offset), and retains GPU object identity
across repeated catalogue lookups.

All six image comparisons pass the unchanged strict lighting-fixture tolerance.
Foreground silhouettes match exactly. Frames 0, 1, 3 and 5 differ by at most one
8-bit channel value; frames 2 and 4 each have one pixel differing by two values.
No tolerance relaxation was needed. This compares the two renderers with the
coplanar-depth correction applied to both, not an unchanged legacy golden image.

The additional two-viewport fixture draws opaque cubes in front of and behind
water. The nearer cube blocks all water (zero changed pixels); water remains
visible in front of the farther cube (19,919 changed pixels). Each water draw
preserves the other viewport. This tests the rendering boundary with cube geometry,
not multiplayer gameplay/session robustness or every entity renderer.

The headed Electron/Chromium smoke runs on AMD hardware through ANGLE. Six probes
report zero GL errors and no context loss, with clean shutdown after movement,
firing and restart. Instrumentation observed seven water draws during title/start
transition with depth test on, depth writes off, additive blending, culling off and
polygon offset enabled. The first gameplay level has no further water draws;
this smoke does not claim sustained water rendering during gameplay or browser
pixel parity. GPU uploads remain at 27 after level entry through restart.
See `browser-smoke.json`, `browser-*.png` and `browser-run.txt`.

Windows/macOS runtime qualification, browser/device coverage, resize/DPI and
context-loss recovery remain deferred. The depth bias is GPU-dependent, so the
coplanar fixtures should also run when those platform gates resume.

## Reproduction

From the repository root, build the game and then run:

```sh
python3 documentation/water-renderer-2026-10-13/build-fixture.py
```

From `runtime`, run each fixture sequentially:

```sh
../build/water-fixture
TANKGAME_RENDERER=modern ../build/water-fixture
```

Then run `python3 documentation/water-renderer-2026-10-13/compare-fixtures.py`
from the root. It converts BMP captures to PNG, writes comparison metrics and
exits successfully when the strict comparison passes.

For the browser smoke, serve `runtime/` on port 8766 and run:

```sh
env -u ELECTRON_RUN_AS_NODE /path/to/electron documentation/water-renderer-2026-10-13/browser-smoke.cjs --no-sandbox
```

Remaining R4 work: general effects, debug lines, overlay ordering, resize/DPI,
full pipeline split-screen captures and broader platform acceptance. Compatibility
continues drawing water within terrain; modern water follows opaque objects.
