# Renderer retirement final validation — 2026-11-19

The final feature-branch implementation uses modern rendering exclusively, with
platform-only context/shader policy, shared GeometryAsset ownership and no GLU
linkage or fixed-function drawing paths.

## Results

- Native configure/build passed; graphics-enabled CTest **238/238 passed** with
  `TANKGAME_TEST_GL=1 LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy`.
- Emscripten configure/build passed. Configuration was regenerated after deleting
  RendererMode.cpp because the source glob is evaluated at configure time.
- Existing browser lifecycle smoke script: **31/31 checks passed**, no reported
  script error. The JSON report and script are retained here.
- Gameplay capture inspected: terrain, player, HUD and overlays are visible.
  This is a smoke capture, not a pixel-equivalence or all-platform visual review.
- `git diff --check` passed.

The browser flow ran in local Electron/Chromium against the rebuilt runtime
served on localhost:8766. It exercises startup/assets, repeated context loss and
recovery, synthetic gamepad reconnect/fire, level advance, resize/DPI/fullscreen,
audio focus, steady play without buffer uploads or GL errors, shutdown/audio
closure, download retry and failed graphics recovery. The final failure path is
intentional and verifies that retry is offered and tasks stop. The temporary
browser process and HTTP server were stopped afterward.

Run the script using an Electron executable with ELECTRON_RUN_AS_NODE unset and
runtime served on port 8766, as described in its first line. Outputs are written
beside the script; copy it to a scratch directory to preserve this evidence.

No fresh macOS/Windows execution, Firefox/Safari/mobile qualification, physical
controller matrix or extended soak was performed. Earlier owner-reported platform
acceptance remains distinct from these final local checks. No release deployment
or branch commit is included in this validation.
