# QA B02: flickering dark terrain streaks — 2026-11-05

## Report and reproduction

The tester reported flickering dark streaks across platform tops in Chrome
155.0.8059.84 on Linux, with renderer:

```text
ANGLE (AMD, AMD Custom GPU 0405 (radeonsi vangogh ACO), OpenGL ES 3.2 Mesa 26.2.4 (git-96cb431210))
```

Original evidence: [tester screenshot](../manual-qa-2026-11-05/B2-screenshot.png).
The deployed HTML, JS, WASM and data were downloaded read-only from
`https://briensmithmartinez.com/tankgame/tankgame-linux.html` and its companion
files. All four SHA-256 hashes matched the previously built release
[`53999c5ab2300804`](../browser-release-2026-10-28/release-manifest.json).
The site did not expose `manifest.json`, but the actual file hashes established
that this was not a mixed-artifact upload.

The issue was reproduced in a visible Electron/Chromium window on the same AMD
GPU family. Turning left to look diagonally across the first level reveals the
streaks; the earlier near-axis-aligned smoke views did not adequately expose them.
The default framebuffer has 24 depth bits. Its reported vertex precision is
10 bits for mediump and 23 bits for highp. See the adjacent probe JSON files.

| Original deployed build | Rebuilt high-precision vertex shader |
| --- | --- |
| [Diagonal view with streaks](deployed/default.png) | [Diagonal view without reproduced streaks](fixed/default.png) |
| [Additional 16-bit-depth probe](deployed/depth16.png) | [Additional 16-bit-depth probe after fix](fixed/depth16.png) |

These captures follow the same timed input sequence, not a deterministic simulation
snapshot. Small position/jump differences are expected; they are visual evidence,
not a pixel-equality assertion. The 16-bit probe uses an offscreen framebuffer and
is supplemental. The original artifact also reproduces in the normal 24-bit canvas.

## Cause and fix

The WebGL vertex shader explicitly reduced its float precision to `mediump`.
That includes world-position attributes, the combined camera/projection matrix,
and the intermediate arithmetic that computes clip-space depth. At diagonal
views of distant terrain, rounding causes walls and surfaces to overlap visibly.
This is shader transform precision, rather than a missing texture or bad upload.

`BuildVertexShader` now uses `highp` for WebGL vertex arithmetic. Its color and UV
varyings remain explicitly `mediump` to match the fragment stage. The fragment
shader retains its existing precision, so the fix does not require optional
WebGL 1 fragment high-precision support. Desktop shader dialects, terrain geometry,
projection near/far planes and depth-buffer configuration are unchanged.

The relevant precision rules are defined in the
[Khronos GLSL ES specification](https://www.khronos.org/files/opengles_shading_language.pdf).

## Validation

- Native and Emscripten builds pass.
- All 148 native tests pass with `TANKGAME_TEST_GL=1`.
- A shader-contract regression protects high-precision transforms and matching
  medium-precision varyings on WebGL.
- The reproduced diagonal-view corruption is absent in the fixed-build captures;
  normal and offscreen probes report zero GL errors.
- All 31 browser lifecycle/failure-path smoke checks pass on the rebuilt artifact,
  including shader linking, context recreation, audio, gamepads and shutdown.
- ZIP integrity was checked. The replacement package and manifest identify release
  `3badb10675e04d13`.

The tester's exact Chrome 155 session still needs a manual B02 retest after upload.
The public site was not changed by this investigation. Broader release qualification
and previously deferred multiplayer issues remain open.

## Reproduce the visual probe

After building, serve `runtime/` on port 8766. Run one visible browser test at a time
so another test window does not steal focus:

```sh
QA_CASE=fixed QA_URL=http://127.0.0.1:8766/tankgame-linux.html \
  env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox \
  documentation/terrain-depth-2026-11-05/probe.cjs
```

For the old build, serve a preserved copy of release `53999c5ab2300804` on another
port and set `QA_CASE=deployed` and `QA_URL` accordingly. The script's default URL
points to the public site and will cease being an old-build reference once updated.

For the full browser smoke, with the fixed build served on port 8766:

```sh
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox \
  documentation/terrain-depth-2026-11-05/browser-smoke.cjs
```

## Replacement package and retest

Local ZIP: `build-browser/releases/tankgame-browser-3badb10675e04d13.zip`.
It contains the `3badb10675e04d13/` directory with all four application files and
`manifest.json`. Upload that entire version directory, open its HTML directly,
and then update the public play link after verification. This avoids cached JS/WASM
from the old unversioned URL being combined with the replacement.

Repeat B02 in the reported Chrome/Linux setup: enter Single player, turn until the
platforms run diagonally across the view, move/jump, and aim while looking toward
the distant platforms. Check that dark triangular streaks no longer flicker over
the tops. Include a fullscreen check and capture a replacement screenshot. Record
any remaining texture shimmer separately from the original dark wall-shaped streaks.
