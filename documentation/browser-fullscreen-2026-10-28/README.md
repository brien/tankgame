# Browser fullscreen — 2026-10-28

This R6 slice adds an explicit Enter/Exit fullscreen button. It fullscreens the
canvas container, keeping pause, mouse capture and exit controls inside the
fullscreen surface. Fullscreen does not request pointer lock. Rejected or
unsupported requests show a retry cue and leave windowed play available. The
standard generated shell's competing fullscreen controls are hidden while the
application owns these controls. Browser settings no longer request startup
fullscreen; native settings and renderer defaults remain unchanged.

Fullscreen transitions reset input/timing through the existing activity boundary.
Before rendering, VideoTask synchronizes SDL's logical window dimensions with the
fullscreen container, using SDL's existing device-pixel-ratio policy for the
buffer and shared viewport/projection updates. Exiting restores the prior logical
window size. CSS dimensions are synchronized too: SDL 2.28.4 otherwise retains
stale CSS sizes when zoom returns to 1x. No new DPI cap or windowed responsive
layout policy is introduced.

Button keyboard events stop before reaching SDL's window listener, which otherwise
cancels Space/Tab defaults. Native button activation and focus traversal are
preserved for both fullscreen and mouse capture. Tab from the focused canvas
can reach the fullscreen control without being swallowed by SDL. Shutdown removes listeners and
controls, restores shell controls and releases fullscreen owned by the game.

## Validation

- Native and Emscripten builds pass; all 145 GL-enabled native tests pass.
- The Chromium/Electron harness checks gesture entry, button exit, externally
  initiated exit, permission rejection/retry, Tab traversal from the canvas and Space activation, mouse capture in
  fullscreen, and shutdown while fullscreen. It verifies that fullscreen alone
  never captures the pointer.
- Zoom at 1.25x and restoration to 1x exercise DPI/resize. Assertions compare CSS
  dimensions, canvas buffer dimensions, device-pixel ratio and the GL viewport.
- Final screenshots and JSON results are adjacent to this file. All ten WebGL probes
  report zero errors/no context loss; the four observed buffer uploads remain
  unchanged during transitions.
- Initial smoke runs exposed the SDL keyboard-default issue; screenshot review
  exposed stale CSS dimensions after zoom restoration. Both are fixed and the
  final harness checks both behaviors.
- Physical browser-chrome Escape handling, physical display/DPI changes and other
  browsers still require manual qualification. The shutdown check uses an injected
  DOM Escape event after entering fullscreen from the title menu, because browser
  chrome can consume a physical Escape before it reaches the game.

## Reproduction

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
```

In another terminal with desktop display access:

```sh
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox documentation/browser-fullscreen-2026-10-28/browser-smoke.cjs
```

For manual validation, enter gameplay and fullscreen, capture/release the mouse,
exit using the button and browser Escape, and repeat after zoom/display changes.
Check keyboard focus and activation of both buttons. Confirm that returning to
windowed mode restores the original canvas size and that controls do not fire the
tank. Audio, gamepad lifecycle, context recovery, packaging, windowed responsive
layout and the broader release/platform matrix remain open R6 work.
