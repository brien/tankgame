# Browser lifecycle and release packaging — 2026-10-28–29

The remaining R6 implementation now includes:

- SDL_mixer OGG music and the ten referenced WAV effects. Sound starts through an
  explicit, keyboard-accessible Enable sound gesture with rejection/retry. Focus,
  visibility and graphics interruption suspend audio; resumption preserves the
  user's music pause choice. Shutdown halts channels before freeing every chunk,
  including the previously leaked landing sound, and closes the audio subsystem.
- Joystick discovery after startup, stable player slots across disconnects, bounded
  axis storage, detached-handle checks and cleanup. Browser controllers require
  neutral controls after connection/focus changes. Standard browser gamepads work
  alongside keyboard input: left stick movement, right stick aiming, A fire,
  B jump, RB special and D-pad camera. Native controller mappings remain available.
- WebGL context loss pauses simulation and releases the old rendering catalogue.
  Restoration rebuilds shaders, buffers, textures and font resources against the
  retained game world. Input/timing reset before play resumes. Failed rebuilding
  stops tasks and offers a reload; it never continues submitting invalid resources.
- Windowed canvas sizing follows the browser viewport, reserving space for help
  and controls. Fullscreen and DPI use the existing SDL drawable-size boundary.
- An accessible loading/error/retry shell with bounded diagnostic output. Settings
  retain read-only packaged defaults and reset on reload; persistence is not added.
  Per-frame browser performance logging is opt-in through `?profile`.
- All 22 shipped level maps and accompanying metadata, fonts, textures, models,
  and referenced audio are preloaded before initialization. Asset changes trigger
  relinking. Unreferenced tracker/AIFF audio is not shipped.
- A pinned Emscripten 3.1.64 CI build and reproducible versioned static releases.
  `tools/package-browser.py` hashes all four artifacts and writes a manifest into
  an atomic release directory; repeated packaging reuses identical content.

## Validation (completed 2026-10-29)

- Native and Emscripten Release builds pass. All 147 native tests pass with GL
  enabled, and all three explicit modern-renderer startup tests pass.
- All 31 Chromium/Electron harness assertions pass: packaged assets, OGG output
  samples, gesture rejection/retry, two context-loss/recovery cycles, gamepad
  firing/disconnect/reconnect with held-button suppression, next-level loading,
  responsive/DPI/fullscreen sizing, audio focus changes and clean shutdown.
- Ten seconds of steady gameplay produces no buffer uploads or GL errors. This
  short stability check does not replace the extended soak gate below.
- Blocking the JavaScript download shows retry; retry successfully starts the
  game. Forced shader allocation failure during context restoration stops the
  task loop and closes audio while presenting retry. Browser C++ exception
  handling is explicitly enabled. The platform adapter normalizes Emscripten's
  nonzero table index for a null WebGL shader before shared code uses it.
- The native audio regression verifies that a pause request made before activation
  survives activation and focus changes, and teardown is safe to repeat.
- Repeated packaging emits the same version and hashes. The final local release
  version is `53999c5ab2300804`; its manifest is saved alongside this document.

Console errors from the deliberately blocked download and failed shader allocation
are expected in the final JSON. Normal gameplay and successful recovery checks
require clean GL probes. Screenshots were reviewed for visible rendering and
control/help placement.

## Reproduction

```sh
cmake --build build -j4
TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure
EM_CACHE=/home/deck/emsdk/upstream/emscripten/cache cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
env -u ELECTRON_RUN_AS_NODE /tmp/tankgame-electron/electron --no-sandbox documentation/browser-release-2026-10-28/browser-smoke.cjs
python3 tools/package-browser.py --output /tmp/tankgame-browser-release
```

The native virtual-joystick regression uses instance IDs rather than assuming no
physical controller is present. It exercises eight-axis devices, detach/reconnect,
neutral absent-device reads and preservation of the other player's slot.
The adjacent browser harness records its assertions and console in JSON and saves
a gameplay screenshot. Synthetic gamepad events test the SDL/browser boundary;
physical hardware still needs qualification.

## Hosting

Serve the emitted version directory intact over HTTPS (localhost HTTP works for
development). Open `tankgame-linux.html` in that directory. Use `text/html` for
HTML, JavaScript MIME for `.js`, `application/wasm` for `.wasm` and
`application/octet-stream` for `.data`. Apply gzip/Brotli at the server, with correct
Content-Encoding and Vary headers; do not rename compressed files into the package.
Version directories may use immutable caching. Revalidate any landing page or
redirect that selects the current release, and publish its pointer only after all
files are available. This single-threaded build needs no SharedArrayBuffer or
cross-origin isolation headers. Deploy under a versioned subdirectory to prevent
mixing HTML/JS/WASM/data from different builds. No service worker is installed.

## Remaining release gates

This implementation does not retire the native compatibility renderer or change
its default. Visual approval, real Windows/macOS runtime qualification (currently
deferred), physical Escape/gamepad/audio-device checks, Firefox/Safari/mobile
qualification, and extended soak/performance budgets remain release gates.
The browser build workflow is added here; a local build is not evidence that the
hosted CI job has run. Deferred multiplayer/session-state robustness remains out
of scope. The local automated smoke is not a claim of cross-browser qualification.
