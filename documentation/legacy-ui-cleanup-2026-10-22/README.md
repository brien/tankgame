# R5 unreachable legacy UI and font cleanup — 2026-10-22

Removed `GraphicsTask::DrawHUD`, `DrawMenu`, `DrawTextTest`, `RenderText`, the
undefined `RenderLegacyUIElements` declaration, and their private font lifecycle.
The old constructor opened DroidSansMono at 256 points before renderer startup,
called `exit(1)` if it failed, and never balanced `TTF_Init` with `TTF_Quit`.
The catalogue's modern atlas now provides the only production font loading;
`FontAtlas::Load` already closes its font and balances SDL_ttf on success/failure.
Modern atlas errors still flow through `ResourceManager::Initialize` failure.
Compatibility no longer requires a font it never renders.

Removed the `drawHUD`/`drawMenu` fields and four write-only assignments in GameTask.
Visibility continues to come from game state through SceneDataBuilder. No visible
output or gameplay behavior is intentionally changed.

## Reachability audit

Before deletion, repository searches found no callers of the four removed draw
methods. RenderText appeared only in its declaration/definition and commented-out
calls inside the obsolete methods. RenderLegacyUIElements had only a declaration.
The defaultFont pointer was used only by that constructor, Stop, and dead text
method. The visibility flags had definitions and assignments, with no reads.

The active path is `GraphicsTask::Update` → `RenderWithNewPipeline` →
`RenderingPipeline::RenderAllPlayerViews`. SceneDataBuilder::ExtractUIData derives
menu/HUD state from GameTask and PlayerManager. RenderUIElements submits player HUD,
targeting and FPS; RenderGlobalUI submits menu and diagnostics. Compatibility uses
its active HUDRenderer; modern native/browser use HUDMeterRenderer/TextRenderer.
The pipeline's reachable DrawSky placeholder is retained. HUDRenderer and browser
deferred implementations are retained because they still supply active interfaces.

This is the roadmap R5 legacy GraphicsTask UI/font cleanup, not completion of R5.
Additional text, active compatibility UI consolidation and explicit compatibility
retirement remain. Compatibility remains the native default. Windows/macOS runtime
qualification and multiplayer robustness remain deferred.

## Validation

- Native and Emscripten Release builds succeeded. Browser linking required access
  to the SDK cache outside the workspace sandbox.
- **136/136 native tests passed with GL enabled**, including the new construction
  regression and existing atlas success/failure lifetime tests. The first sandbox
  run passed 134 tests but could not initialize SDL video for two GL tests; the
  full run passed with display access. See native-tests.txt and native-build.txt.
- The Electron browser smoke passed all four WebGL probes with no errors or
  context loss. Player FPS/global debug glyphs, targeting and meters/icons were
  observed; firing reduced the displayed energy. Buffer uploads stayed at four
  after level entry and texture uploads stayed at zero during observation.
  Shutdown completed cleanly. See browser-smoke.json and retained screenshots.
- Source search now finds all production SDL_ttf initialization/font ownership
  exclusively in FontAtlas.cpp. Native/browser links prove no removed methods
  remain required. `git diff --check` passes.

This validates the narrow removal and the active browser UI path; it does not
establish new visual parity, native full-game smoke, cross-platform runtime
qualification, or general leak freedom. Electron emitted host D-Bus/GTK module
warnings; the gameplay report contains no errors.

## Reproduction

From the repository root:

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
```

With the server running, execute browser-smoke.cjs using Electron with
ELECTRON_RUN_AS_NODE unset and display access. The retained smoke is adapted from
the previous global-UI slice and checks title, gameplay, firing, cached resources,
WebGL state and shutdown. It writes its JSON report and screenshots beside itself.
