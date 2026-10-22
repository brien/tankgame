# R5 owned UI snapshots and global menu — 2026-10-21–22

SceneData now owns an immutable UI snapshot with shared lifetime. Scene copies
retain the snapshot; Clear, replacement and destruction release it. SceneDataBuilder
transfers its unique allocation directly into that owner instead of releasing a raw
pointer that leaked on every frame. This keeps the forward-declared UI type and
existing copyable scene API without introducing GL into the DTO.

The pipeline now draws menu selection and global diagnostics once, after all
player views, using the full drawable's projection. Player HUD meters, targeting
and FPS remain local. Both native backends use this scope, correcting duplicated
menu selection in split-screen. The global pass restores viewport/scissor state
and modern camera context; compatibility also preserves fixed-function attributes
and matrices. Zero-size drawables and absent UI skip the pass.

This completes the ownership/global-scope slice, not R5 or renderer retirement.
Compatibility remains the default; its diagnostic text remains a placeholder.
Additional UI text, legacy GraphicsTask UI/font reachability cleanup, explicit
retirement approval, deferred Windows/macOS runtime and multiplayer robustness
remain open. No general memory-leak or context-recovery claim is made.

## Validation

- Native and Emscripten Release builds succeeded.
- **135/135 native tests passed with GL enabled.** New ownership tests cover scene
  copies, Clear, destruction and 1,000 frame replacements.
- Six Linux fixture cases per backend (all three selections, one/two viewports at
  801x601) match a separately submitted full-drawable menu reference exactly.
  Each asserts that the menu visibly changes the frame, preserves depth, restores
  the last player viewport, and returns GL_NO_ERROR. Compatibility/modern RGB
  comparisons differ by at most one channel value in one pixel; split-screen
  captures match exactly. See comparison.json and retained PNGs.
- Electron browser title/start/firing/idle probes passed with four clean WebGL
  checks, no context loss, stable buffer/texture uploads after level entry, and
  clean shutdown. Menu draw probes verify a full-drawable viewport. Screenshots
  preserve title and gameplay output. This is a single-player browser smoke;
  split-screen scope is covered by the deterministic Linux fixtures.

## Reproduction

From the repository root:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
python3 documentation/global-ui-2026-10-21/build-fixture.py
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
```

From runtime, with display access:

```sh
TANKGAME_RENDERER=modern ../build/global-ui-fixture
env -u TANKGAME_RENDERER ../build/global-ui-fixture
```

Then run `python3 documentation/global-ui-2026-10-21/compare.py` from the root.
Serve runtime on port 8766 and run the retained browser-smoke.cjs with Electron
and ELECTRON_RUN_AS_NODE unset. BMPs are generated locally; PNGs retain examples.
