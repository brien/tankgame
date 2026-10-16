# R4 shared effects slice — 2026-10-14

The next unfinished feature after water was shared effect rendering. This slice
implements all eight existing effect types on native modern and Emscripten using
one EffectRenderer, the existing ResourceManager catalogue and shared program.
Compatibility remains the default and comparison backend. R4 as a whole remains
open: debug lines, player-overlay ordering, full-pipeline split-screen captures,
resize/DPI and deferred platform qualification are separate follow-ups.

## Behavior and state

The effect path preserves position (+0.2 Y), X/negative-Y/Z rotation order,
type-specific scale, extracted color/alpha, horizontal quads, and the ZERO/DEATH
outlines. TYPE_THREE uses the existing heart texture (slot 16); TYPE_STAR uses
star (slot 19), not the player's diamond overlay. No meshes, textures or programs
are created per frame. Browser effect substitutes and the modern pipeline skip
are removed. The pipeline no longer redundantly wraps the effect renderer's own
state setup/cleanup.

Both backends now explicitly render effects unlit. Previously compatibility
lighting depended on the preceding renderer. This is an intentional correction
to make emissive effects deterministic; comparisons use the corrected reference.
The existing compatibility transform/draw implementation remains available.

Modern submission preserves vector order and each solid outline followed by its
additive quad. It uses depth LESS, disables culling, and blends SRC_ALPHA/ONE.
Outlines retain depth writes and opaque alpha; quads never write depth. The pass
leaves depth testing/writes enabled, blending disabled, standard alpha factors,
back-face culling and clockwise winding. This is an explicit pipeline baseline,
not arbitrary incoming-state restoration. The baseline is restored on exceptions.
No transparency sorting is added for additive quads, and no gameplay changes are
included.

## Validation

- Native and Emscripten Release builds pass.
- 116/116 native tests pass with TANKGAME_TEST_GL=1, including startup.
  Three new tests cover all type-specific scales/textures/outlines, legacy
  rotation order/negative yaw, and zero-scale geometry.
- Six Linux fixture pairs cover every effect type, six alpha values, alternate
  views, and tilted geometry. Every draw reports zero GL errors. Captures have
  identical silhouettes. Most differences are at most one 8-bit channel value.
- The original water tolerance (at most one larger pixel difference) fails for
  two poses. Investigation identifies at most two coplanar outline/quad samples
  per image where depth interpolation admits additive color in only one backend.
  Maximum channel difference is 102, confined to those samples. The effect-specific
  comparator permits at most two such samples, requires an exact unblended outline
  color on one side, and retains strict silhouette/interior checks. This is a
  documented narrow tolerance exception, not exact pixel parity.
- Modern fixture verifies unchanged depth for all additive quads and outgoing
  pass state. In two viewports, a nearer opaque cube blocks all effect pixels;
  a farther cube allows 8,061 changed pixels. Neither draw changes the other view.
- Headed Electron/Chromium on AMD through ANGLE completes title, gameplay,
  movement, ten seconds of firing, restart and shutdown. Six probes show zero GL
  errors/no context loss. Instrumentation records 34,021 additive six-vertex,
  unculled, non-water draws by restart, with depth testing on and depth writes
  off. Buffer uploads stay at 27 after level entry through restart. This is
  browser gameplay/state evidence, not deterministic browser pixel parity.

Windows/macOS runtime gates remain deferred. No new platform qualification,
multiplayer robustness, context-loss recovery, or broad browser matrix is claimed.

## Reproduction

From the root:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/effect-renderer-2026-10-14/build-fixture.py
```

From runtime, run `../build/effect-fixture` and then
`TANKGAME_RENDERER=modern ../build/effect-fixture`. From the root, run
`python3 documentation/effect-renderer-2026-10-14/compare-fixtures.py` to generate
PNG captures and comparison metrics from BMP output.

Build the configured Emscripten build, serve runtime on port 8766, then run:

```sh
env -u ELECTRON_RUN_AS_NODE /path/to/electron documentation/effect-renderer-2026-10-14/browser-smoke.cjs --no-sandbox
```

Logs, captures, metrics and reusable fixture/smoke scripts accompany this report.
