# R5 active HUD consolidation — 2026-10-23

HUDRenderer now orchestrates modern targeting, meters and cached FPS/debug text
on both native and Emscripten. RenderingPipeline performs the same player-ID
lookup and delegates without a second modern HUD dispatch path. Global diagnostic
scope and menu ownership remain unchanged.

Compatibility meters use HUDLayout's draw descriptions, eliminating duplicate
bar, cost, combo, special and debug-meter calculations. A small fixed-function
submission adapter preserves the compatibility backend and targeting reference.
Its meter pass preserves depth, matrix mode/stacks, blend state, texture state,
line width and enable flags. This also applies the existing modern finite-value
bar guard to compatibility. Targeting retains its existing depth-write policy.

HUDRenderer borrows ResourceManager explicitly. It no longer looks up App or
caches texture names. Uncalled RenderCompleteUI/RenderAllPlayerHUDs/RenderMenu
helpers and placeholder text formatting are removed. The real HUD source now
compiles in the browser and its deferred substitute is removed; the unified
compatibility tank substitute remains. Compatibility text is still not drawn.

## Validation

- Native and Emscripten Release builds pass; all 136 GL-enabled native tests pass.
- Ten Linux one-/two-viewport fixture pairs have zero RGB differences. All ten
  compatibility captures are byte-identical to captures taken before the change
  (baseline-comparison.json). Cases include depleted/overfilled bars, zero maxima
  and costs, hidden bars/meters, icons, alpha specials and debug meters.
- The updated fixture exercises HUDRenderer in both modes, checks unchanged
  depth, restores nondefault blend factors/equations, texture unit/binding,
  enables, CPU camera and fixed-function camera matrices. GL probes are clean.
- Browser title, playing, firing and cached probes report GL_NO_ERROR and no
  context loss. FPS/global debug text, targeting and icons/meters draw; firing
  reduces displayed energy. Buffer uploads remain four after level entry and
  texture uploads remain zero. Escape transitions finish with clean shutdown.
  Screenshots and browser-smoke.json retain evidence. Browser runtime logs have
  host DBus/GTK module warnings, with no game error reported by the harness.
- The old HUD fixture needed its obsolete defaultFont cleanup removed after the
  preceding font-ownership slice; its constructor now receives the catalogue.

This closes the active HUD consolidation slice. Additional text and explicit
renderer retirement remain R5 work. Compatibility stays the native default;
Windows/macOS runtime and multiplayer robustness remain deferred.

## Reproduction

From the repository root:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/hud-consolidation-2026-10-23/build-fixture.py
```

From runtime/, run `../build/hud-fixture` and
`TANKGAME_RENDERER=modern ../build/hud-fixture` with display access. Then from
the root run `python3 documentation/hud-consolidation-2026-10-23/compare.py`.
This writes BMPs and representative PNGs and checks all ten image pairs.

Build the configured browser target with
`EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4`
(or the local SDK cache), serve runtime/ on port 8766 and run:

```sh
env -u ELECTRON_RUN_AS_NODE /path/to/electron --no-sandbox documentation/hud-consolidation-2026-10-23/browser-smoke.cjs
```
