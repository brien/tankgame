# Shared modern enemy tanks (2026-10-07)

Enemy bodies, turret housings (called `barrel` in the compatibility code), and
cannons (called `turret`) now render through the same modern implementation on
native desktop and Emscripten. Native compatibility remains the default and
visual reference. This is a narrow continuation of resource/submission convergence;
terrain, effects, HUD/menu/text, lighting, audio, browser UX, and multiplayer
robustness remain deferred.

## Source contract and ownership

The active compatibility path is `RenderingPipeline -> TankRenderer::RenderEnemyTank`.
It uses immediate triangles, **one fixed body/barrel/turret mesh for every type
pair**, secondary body colour, and primary housing/cannon colour. The extruded
and edge GSM variants in the catalogue belong to player effects; selecting them
for enemies would change the existing visual contract.

`SimpleGeometry::CreateEnemyBody/Barrel/Turret` copies all 36/36/24 vertices,
winding, normals, and UVs from that active implementation. `ResourceManager`
adds three typed catalogue slots, initialized once and closed before context
teardown. No new mesh assets or preload entries are required.

`EnemyTankRendererImpl::BuildDraws` supplies a fixed array of three GL-free draw
descriptions, consumed directly by modern rendering:

| Component | Model (post-multiplication order) | Colour |
| --- | --- | --- |
| Body | `T(position) * Rx(body.x) * Ry(-body.y) * Rz(body.z) * S(.06)` | Secondary |
| Housing / barrel | Unscaled body transform, then `Rx(turret.x) * Ry(-turret.y) * Rz(turret.z) * S(.1)` | Primary |
| Cannon / turret | Same aim transform, then local `T(.1,0,0) * S(.1)` | Primary |

Enemies receive no player-specific height offset or 90/180-degree mesh correction.
Body scale does not affect relative aim or the cannon offset. The opaque,
untextured `BasicMaterial` preserves `(4 * channel + maxHealth / health) / 2`
for positive health/maxHealth, including values above one. Nonpositive health
or maximum uses the base colour, matching the guarded existing specialized
renderer rather than dividing by zero; dead enemies are filtered before drawing.

The pipeline retains the player pass/overlay sequence, then sets opaque depth
writes, `GL_LESS`, clockwise front faces, back-face culling, and no blending for
enemies. Alive/player routing is unchanged for compatibility. Modern enemies
submit through `RenderContext -> ModernRenderer -> catalogue GpuGeometry`, using
the existing sole program. There is no per-frame geometry upload, shader compile,
or new resource owner. The enemy factory is now shared; Emscripten compiles the
same enemy source and excludes only its retained native compatibility operations.
The unified browser compatibility renderer remains a deferred stub and is not
used for the modern enemy pass.

## Automated validation

Native Release configure/build succeeded. Eight new focused tests cover all 25
existing type pairs, colour selection and health multipliers, component scales,
relative aim and rotated cannon offsets, combined pitch/roll/yaw order, geometry
counts/bounds/attributes/clockwise winding, factory filtering, and stable distinct
catalogue identity. Existing gameplay and renderer tests remain intact.

Commands from the repository root in the supplied Linux dependency environment:

```sh
source /workspace/.tankgame-env/activate.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON \
  -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/workspace/.tankgame-env/build/_deps/googletest-src
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

The initial ordinary CTest run passed 97 tests and skipped the existing display
opt-in test. With Xvfb `:93`, `TANKGAME_TEST_GL=1`, `SDL_AUDIODRIVER=dummy`, and
`XDG_RUNTIME_DIR=/workspace/.tankgame-env/xdg`, the same CTest command passed
**98/98, zero skipped**. A separate modern process also passed:

```sh
TANKGAME_TEST_GL=1 TANKGAME_RENDERER=modern \
  ./runtime/tankgame_tests --gtest_filter=VideoStartupTest.*
```

The cached GoogleTest source avoided an initially inaccessible sandbox proxy.
Network-enabled tool execution subsequently worked. A conversion audit also
compared every enemy vertex/UV/normal against the retained active compatibility
source and found exact equality. No Windows/macOS compile or hosted CI result is
claimed by these Linux commands; the existing three-platform CI is unchanged.

## Emscripten build

The full Release application compiled, linked, and packaged `.html/.js/.wasm/.data`
with the same enemy renderer, geometry, catalogue and submission sources. The
available SDK was Debian Emscripten **3.1.69**, rather than the historical procedure's
3.1.64. Native/host include and library flags were kept out of cross-compilation.

This SDK lacked SDL2 CMake discovery. A scratch `work/browser-sdl2/SDL2Config.cmake`
contained only `set(SDL2_FOUND TRUE)`, `set(SDL2_LIBRARIES "-sUSE_SDL=2")`, and
`set(SDL2_INCLUDE_DIRS "")`. The corresponding compile flag enables port headers.
The local extracted SDK also needed its packaged Acorn dependency reachable from
Node and had a broken Debian `get_npm_cmd` HTML-minifier lookup; disabling only
HTML minification avoided that packaging helper. No production CMake/preload change
was made for these local toolchain adapters. After activating that SDK/cache and
CMake environment, the successful commands were:

```sh
emcmake cmake -S . -B build-browser -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release \
  -DSDL2_DIR=/workspace/tankgame/work/browser-sdl2 \
  -DCMAKE_CXX_FLAGS=-sUSE_SDL=2 -DCMAKE_EXE_LINKER_FLAGS=-sMINIFY_HTML=0
cmake --build build-browser -j4
```

The initial link failures were local Acorn/HTML-minifier discovery errors, not
enemy compilation or GL-symbol failures. JavaScript/WASM still use Release
optimization; only shell HTML minification was disabled. Testing the historical
3.1.64 SDK and normal hosted/toolchain packaging remains a reproducible follow-up,
not a result claimed here. The repository build change is limited to removing
`EnemyTankRendererImpl.cpp` from Emscripten's excluded compatibility-source list.

## Runtime and visual evidence

Evidence and replay harnesses live in
[`documentation/enemy-renderer-2026-10-07`](../../documentation/enemy-renderer-2026-10-07/README.md).
Linux compatibility and modern both entered level-0 single-player gameplay,
accepted injected keyboard/mouse input, and exited through Escape with status 0.
Modern captures show enemy bodies/housings/cannons, their changing screen positions,
the player and ring/star overlays, and firing. Compatibility captures show the
retained terrain/player/HUD reference. No GL/shader failure was recorded in either
native gameplay log. Release gameplay does not poll `glGetError`; the explicit
GL probes belong to the separate fixture and startup test. These are Xvfb/software runs, not hardware or frame-pacing
qualification. Small distant gameplay enemies do not establish precise aim parity.

Headed Chromium **151.0.7922.173** with ANGLE/SwiftShader under Xvfb also entered
single-player gameplay, captured idle/enemy-motion/moved/firing frames, and stopped
cleanly through the existing Escape sequence. The replay driver used the existing
canvas click-to-start input and waited for actual enemy draws before taking frames.
All four probes returned `getError() == 0` and no context loss; no JavaScript,
shader, or GL diagnostic was recorded. The only console error was `/favicon.ico`
HTTP 404, confirmed in the server log; game HTML/JS/WASM/data requests succeeded.
Enemy 36/24-vertex draw samples showed depth testing and writes enabled, blending
disabled, and `GL_CW` front faces. Captures show changing enemy positions and aim,
and the player/overlays remain visible. This software-rendering run is not the
historical hardware-browser 60 FPS result; hardware frame pacing, precise distant
turret parity, and browser/device breadth remain unqualified. Items were not
isolated in these captures; no new item visual-parity claim is made.

The separate close-up fixture renders three enemies twice through the production
renderer/catalogue, changing position and relative aim in the second pose. Both
backends returned `GL_NO_ERROR` after every frame. Background/foreground masks
match exactly between backends in both poses (43,440 and 44,203 foreground pixels;
zero silhouette mismatches). This verifies the tested geometry/transform/depth
silhouettes; it is not a full pixel-colour or gameplay parity claim. The comparison
visibly shows correct housing-over-body occlusion and rotated cannon placement.

Modern enemy surfaces remain unlit and can look flat or washed out where the
preserved health colour formula saturates. Compatibility applies directional
lighting and separates same-colour faces with shading. Normals/UVs remain in the
geometry for future work; this milestone adds neither lighting nor textures.
Modern terrain, effects, and HUD absence remains expected. Broader desktop GPU,
browser/device, and variant screenshot qualification remains manual.

## Windows and browser manual checks

Windows owner evidence: the owner reports the Windows build runs correctly, but
**the renderer mode was unspecified**. This is not Windows modern-renderer
qualification. From the runtime directory in PowerShell:

```powershell
$env:TANKGAME_RENDERER = 'modern'
.\tankgame-linux.exe
```

1. Enter single-player level 0. Confirm visible enemy bodies, housings, and cannons
   for the available colour variants; move and watch their aim change relative to
   the body. Fire while checking player meshes, bullets, and ring/star overlays.
2. Check depth/occlusion, opaque colours, and logs for loader/shader/GL errors;
   capture idle/moving/firing images. Expect unlit faces and missing deferred passes.
3. Escape to the menu, then Escape to exit. Remove the environment variable and
   repeat in compatibility for reference: `Remove-Item Env:TANKGAME_RENDERER`.

Browser: activate the repository's Emscripten toolchain, then:

```sh
emcmake cmake -S . -B build-browser -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-browser -j4
python3 -m http.server 8766 --directory runtime
```

1. Open `http://localhost:8766/tankgame-linux.html`, focus the canvas, press Enter.
   Verify visible enemy bodies/housings/cannons alongside the player.
2. Wait for enemy movement/aim, then move/aim/fire. Inspect overlapping parts,
   colour, bullets and player overlays; capture frames and inspect the console.
   Probe the canvas WebGL context for `getError() === 0` and no context loss.
3. Escape twice and confirm `Browser frame loop stopped`. Terrain/UI absence is
   expected; later unpreloaded levels and multiplayer remain outside this check.

The owner explicitly deferred macOS runtime validation and authorized this narrow
migration. macOS source/build support and CI checks remain preserved, but core
runtime validation is still open because other deferred categories retain
compatibility calls. Full lighting parity and multiplayer robustness remain open.
