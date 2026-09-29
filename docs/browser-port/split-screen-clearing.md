# Split-screen framebuffer clearing

## Verified render sequence and root cause

The defect predates the browser port: the faulty per-view clear was already present
in commit `62c42669` (2025-07-22). The current compatibility, native modern, and
WebGL paths all enter the same `GraphicsTask::RenderWithNewPipeline` and
`RenderingPipeline::RenderAllPlayerViews` orchestration, so this is a shared
pipeline issue rather than a browser-specific renderer issue.

For two players, `ViewportManager` creates player 0 at `(0, 0, width,
height / 2)` and player 1 at `(0, height / 2, width, height / 2)`. OpenGL's
bottom-left origin therefore puts player 0 in the lower half and player 1 in the
upper half. The pipeline visits indices 0 and 1 in that order, and each index is
matched to the same-index camera. Co-op and versus do not select different
viewport layouts. Both halves have the same aspect ratio (twice the full-frame
aspect at the fixed even-height resolution).

Before this fix, the sequence was:

1. `GraphicsTask::Update` cleared the full color and depth buffers.
2. Player 0 selected the lower viewport with `glViewport`, then `RenderScene`
   cleared the full color and depth buffers and drew the lower view.
3. Player 1 selected the upper viewport with `glViewport`, then `RenderScene`
   again cleared the full color and depth buffers and drew the upper view.

No code enabled `GL_SCISSOR_TEST` or called `glScissor`. `glViewport` was active
at each per-player clear, but OpenGL and WebGL clears are not bounded by the
viewport. Consequently, step 3 erased both the color and depth values produced
by player 0; only player 1's upper view survived. Source inspection therefore
confirms the manual review diagnosis exactly.

## Selected strategy

The pipeline now performs one full-frame color/depth clear at the start of
`RenderAllPlayerViews`, before selecting or drawing any player viewport. The
earlier `GraphicsTask::Update` clear and the per-player clears were removed, so
there is exactly one clear per frame.

This is preferable to scissored per-view clears because both views intentionally
share the pipeline clear color, their rectangles do not overlap, and depth tests
only compare fragments at the same framebuffer coordinates. A single initial
depth clear therefore gives each disjoint rectangle the same clean depth state.
It also avoids extra state changes and any possibility of scissor state leaking
into later world, per-player HUD, or global overlay passes. No native/browser
conditional is needed; `glClear` has the required semantics on compatibility
OpenGL, modern OpenGL, and WebGL.

Single-player still receives the same full-frame color/depth clear before its
only view. In split-screen, player 0's lower color/depth region remains intact
while player 1 draws the upper region. A future per-player clear color or
overlapping viewport feature would require revisiting this choice, but neither
semantic exists today.

Per-player UI remains ordered inside each player's viewport pass. Global menu
and overlay migration is still deferred; when introduced, a global pass must run
after all player views and must deliberately select the full framebuffer
viewport. This fix does not enable scissor state and therefore creates no hidden
constraint for that work.

## Validation

The Linux Release executable and all 84 discovered tests build successfully; the
three new platform-independent tests cover the full-frame single-player rectangle,
the non-overlapping lower/upper two-player rectangles and aspect ratios, and the
shared co-op/versus player mapping. The Emscripten Release target also builds and
links its HTML, JavaScript, WASM, and data outputs successfully. Removing two
redundant clears makes the frame perform less GPU clear work; no material frame-time
regression is expected.

This environment has no X server, headed Chromium, or virtual-display executable,
so it cannot honestly provide a new visual native or browser smoke result. The
following checks remain required on a graphical machine for both default and
`TANKGAME_RENDERER=modern` native runs:

1. Run `runtime/tankgame-linux` from `runtime/`, start single-player, and confirm
   its appearance is unchanged.
2. Start co-op and, where practical, versus; confirm the lower player 0 and upper
   player 1 views are simultaneously visible and follow their assigned cameras.
3. Exercise movement, camera input, firing, pause/restart, and shutdown; confirm
   correct depth occlusion, no stale frames or flicker, no GL errors, and clean exit.
4. Repeat steps 1-3 with `TANKGAME_RENDERER=modern` while remembering that terrain,
   enemies, lighting, HUD/menu, and other documented modern visuals remain deferred.

For browser validation:

1. From the repository root, run `python3 -m http.server 8000 --directory runtime`.
2. Open `http://localhost:8000/tankgame-linux.html` in a WebGL-capable browser.
3. Start co-op/two-player and confirm both the lower and upper views remain visible.
4. Start versus if practical and make the same visibility check.
5. Exercise both players' input and cameras, including movement and firing.
6. Confirm the developer console reports no new WebGL errors or context loss.
7. Confirm frame pacing remains near the established approximately 60 FPS baseline.
8. Return to the menu and shut down cleanly.

Remaining limitations are unchanged: modern/browser HUD and menu presentation,
terrain, enemies, effects, and lighting are deferred, resize/high-DPI behavior is
not yet validated, and an odd framebuffer height leaves the existing one-pixel
layout remainder. None is part of this framebuffer-clear correction.
