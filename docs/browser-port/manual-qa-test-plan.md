# Tankgame browser and graphics migration: manual QA handoff

Prepared: 2026-11-04. Audience: a tester who can use a browser, unzip a folder,
and take screenshots. Reading code or compiling the game is not required.

This plan checks the browser game and the desktop graphics migration. A **renderer**
is the part that draws the game. Desktop builds may use the older **compatibility**
renderer or the newer **modern** renderer; the browser uses modern rendering.
A **HUD** is the on-screen health, energy, and status display. **Mouse capture**
keeps the pointer inside the game so moving the mouse continuously aims the turret.

Use a copy of the [results sheet](manual-qa-results-template.md) for each assigned
browser/operating-system/renderer combination. This document is a plan, not a
record of tests already passed. Earlier automated checks do not replace this run.

## 1. What the person assigning QA must supply

Complete these items before handing the build to the tester:

| Item | Information to supply |
| --- | --- |
| Build identity | Commit/build number and browser package version, if available. Both desktop comparison builds must come from this same revision. |
| Browser game | A working HTTPS link, or a running local-server link. The address should end in `tankgame-linux.html`. Do not ask the tester to double-click an HTML file. |
| Desktop game, if assigned | An extracted package with all assets and a double-click launcher for each assigned renderer. Label the launchers **Modern** and **Compatibility**. |
| Test coverage | Name required operating systems, browsers and versions, renderer modes, and cases. Mark unassigned combinations explicitly. |
| Visual reference | Approved screenshots or a reference build, with mode, level, camera and window size identified. If none exists, the tester records observations; visual approval remains pending. |
| Equipment | Keyboard, mouse, speakers/headphones, and ideally two standard gamepads. Supply controller labels/mapping if they differ from the table below. |
| Assistance | A contact for setup failures, unknown controls, missing assets, developer-assisted cases and bug reports. |
| Fault-testing link | For case B13, an intentionally broken copy that serves the page but cannot download its game JavaScript, plus a way for the owner to repair it. |
| Release decisions | Supported combinations, target loading time/frame rate, acceptable visual differences, and whether deferred multiplayer issues are accepted for this release. |

Do not treat an untested platform as supported. Windows/macOS runtime qualification
was deferred in the migration roadmap; assign those runs explicitly. The current
macOS build requires **Modern** mode: do not assign a macOS compatibility run.
Mobile/touch-only operation is not established; a phone/tablet run is exploratory
unless the owner supplies an input method and acceptance criteria.

Suggested coverage for the owner to assign:

- Primary desktop browser: all B cases, graphics review, and a 60-minute soak.
- Each additional supported browser/OS: B01–B12 and B14–B15; B13, B16 and the assisted
  recovery cases where the owner can provide the setup.
- Native Linux: N01–N03 in both renderer modes.
- Native Windows and macOS: N01–N03 when their runtime checks are assigned;
  modern on macOS, and explicitly named renderer(s) on Windows.
- Real gamepad, fullscreen/Escape, audio-device and multi-monitor checks should use
  physical devices, even if an automated browser test already passed.

Allow approximately 60–90 minutes for the primary browser checklist, another hour
for the soak, and 30–45 minutes per desktop renderer. Full level coverage and
additional browsers take extra time. Record actual duration rather than rushing.

## 2. Starting, resetting and recording a run

1. Record the build and equipment on the results sheet. Set browser zoom to 100%
   using its menu. Keep speaker volume low initially and make sure the tab is not muted.
2. Close duplicate copies of the game so their audio and input cannot interfere.
3. Open the supplied link. Wait for the title/menu and moving game image.
4. Use **Enter** to start the selected mode. A click inside the title image can
   also start play, so use the keyboard when checking menu selection.
5. For a **fresh session**, reload the browser page, or close and relaunch the
   desktop game. Returning to the menu is not a fresh session.
6. Record Pass, Fail, Blocked or Not run for each assigned case. A missing controller,
   missing build or unavailable reference is **Blocked**, never Pass. Give a reason.
7. On a failure, capture evidence before reloading. Repeat once from a fresh session
   if safe, and record both outcomes. Continue unrelated cases if possible.

**If the mouse is trapped:** press Escape once and wait. If still captured, use
Alt+Tab (Windows/Linux) or Command+Tab (macOS) to switch applications. Close the
game window/tab if necessary. Record what released it. Browser Escape may first
release capture or exit fullscreen. Check the screen after each press rather than
pressing it rapidly; Escape received at the title menu stops the game.

## 3. Controls to keep beside you

These are the current keyboard controls and standard browser-gamepad mapping.
Native controller mappings may vary; the supplied launcher/setup must identify them.

| Action | Keyboard/mouse | Standard browser gamepad |
| --- | --- | --- |
| Choose Single player | From fresh title, already selected; Left returns to it | Left stick can navigate; use keyboard Enter to confirm |
| Choose Co-op | From Single player, press Right | As above |
| Choose Versus | From Single player, press Right, then Down | As above |
| Begin selected mode | Enter | Use keyboard Enter for this plan |
| Forward/backward | W / S | Left stick up/down |
| Turn tank body | A / D | Left stick left/right |
| Aim turret | Mouse left/right after **Capture mouse**, or Left/Right arrows | Right stick left/right |
| Fire | Hold left mouse button | A / bottom face button (Xbox labels) |
| Special attack | Hold right mouse button | RB / right shoulder button |
| Jump | Space | B / right face button |
| Turbo | Left Shift while moving, with energy available | LB / left shoulder button |
| Camera | C cycles distance; Up/Down arrows change height | D-pad chooses camera presets |
| Music pause/resume | M; this does **not** mute sound effects | Use keyboard M |
| Advance level for QA | Tap T once during keyboard-controlled play | Use keyboard T with sticks/buttons released |
| Return to menu / stop | Escape during play / Escape again at menu | Use keyboard Escape |

Special attacks, jumping, turbo and firing can depend on tank type, energy or charge.
Let the energy meter refill before retrying an action. Capture a screenshot if the
state is unclear; do not mark a control broken solely because energy is empty.
There are no touch controls documented by this plan.

## 4. Browser checklist

### B01 — First load, reload and menu (5 minutes)

1. For a first-download check, use a new private window or ask the owner to clear
   only this game's site cache. Record which method you used.
2. Open the supplied URL and time how long it takes to show the menu. Observe the
   loading message. Take a title screenshot.
3. From a fresh menu press Right, Down, Up, Left, one key at a time. Check the
   Single player, Co-op and Versus selection indicator/legend.
4. Reload normally in the same window and record the second loading time.
5. Press Enter on Single player.

**Expected:** the page reaches a usable menu without a permanent loading message,
blank canvas or missing assets. Selection matches the mode chosen. Reload works
with cached files too. Gameplay starts. No music/effects play before **Enable sound**.
Compare timings to the owner's target; if no target exists, report timings without
inventing a performance pass threshold. Private-window limitations must be noted.

### B02 — Movement, aiming, attacks and HUD (5 minutes)

1. Start fresh single-player. Hold W for two seconds, release; repeat with S, A and D.
2. Aim with Left/Right arrows. Click **Capture mouse**, then aim with the mouse.
3. Hold left mouse for three seconds. Release it and allow energy to recover.
4. Try Space, Left Shift while moving, and right mouse when sufficient charge/energy
   is available. Try C twice and briefly use Up/Down to change the camera.
5. Observe health/energy bars and text, combo/special labels, projectiles, hits and
   any collected items. Let an enemy hit the tank if practical.

**Expected:** the body moves/turns and the turret aims visibly; firing produces
projectiles/effects and appropriate energy changes. Controls stop driving actions
when released (brief movement inertia or existing projectiles are not stuck input).
HUD values are readable and agree with the visible meters. Camera changes leave a
usable view. Record any attack/effect/item that could not be reached as untested.

### B03 — Sound, music pause and audio device (5 minutes)

1. Start fresh. Verify silence before activation, then click **Enable sound** once.
2. Listen for music. Enter play; fire, jump and land to listen for effects.
3. Press M: music should pause while firing effects remain audible. Press M again.
4. Press M to pause, switch to another application for 10 seconds, return, and fire.
   Music should remain paused. Press M to resume.
5. Start fresh again. Press M before enabling sound, then enable sound. Music should
   remain paused until you press M again.
6. If assigned, switch between headphones and speakers using the normal OS sound
   selector. Record whether playback continues and any recovery needed.

**Expected:** sound starts after activation without duplicated music, sustained
crackling, or a game freeze. Focus changes preserve the music-pause choice. Device
switch failures are reported with the device names; do not assume hot-switching
is supported merely because the game offers audio activation.

### B04 — Unfocused window, hidden tab and held controls (5 minutes)

1. While playing with sound, switch to a different application for 15 seconds.
   Keep the game window visible if possible so you can observe its pause message.
2. Return, release all controls, then make a fresh movement/aim/fire input.
3. Repeat using another browser tab for 30 seconds, then minimize for 30 seconds.
4. Hold W while switching applications; release W while away, then return. Repeat
   with the firing button, switching away with the keyboard.

**Expected:** gameplay and audio suspend while the game is hidden/unfocused.
On return, there is no large simulation jump, burst of overdue effects, stuck
movement or continuous firing. The pause message clears when the game regains
focus. Fresh input works. Previously held controls may require release before reuse.
Do not require browser pause behavior from native desktop builds.

### B05 — Mouse capture, physical Escape and retry (5 minutes)

1. Start fresh gameplay. Click **Capture mouse**; move the mouse continuously in
   one direction. Confirm aiming continues beyond the normal screen edge.
2. Press physical Escape once. Record whether capture, fullscreen (if active),
   or the game state changed. Check that you can move the pointer to browser controls.
3. Capture again, switch away and return. Record capture state; explicitly use
   **Capture mouse** again if needed. Repeat this cycle three times.
4. If the browser presents a capture permission prompt, deny it once, check the
   failure/retry cue, then allow capture on a later attempt. If no prompt appears,
   mark only the denial portion Blocked and request assisted coverage.

**Expected:** capture requires an explicit action, its cue matches the actual state,
and release does not leave movement/firing stuck. Capture can be acquired again.
Denial leaves the game usable with keyboard movement/aiming and explains retry.
Do not assume every browser uses the same Escape sequence or permission prompt.

### B06 — Fullscreen and keyboard-accessible buttons (5 minutes)

1. Start windowed. Click **Enter fullscreen** without capturing the mouse first.
2. Check that the game fills the available surface, and audio/capture/fullscreen
   controls remain visible. Fullscreen alone must not capture the mouse.
3. Exit with **Exit fullscreen**. Repeat entry, then exit with physical Escape.
4. In fullscreen, capture the mouse; release it and exit fullscreen. Repeat twice.
5. In a fresh windowed session, use Tab/Shift+Tab to reach the page controls.
   Activate an available button with Enter and then with Space on a later attempt.

**Expected:** no stretched image, clipped controls, unexpected firing or stuck input.
Exit restores a canvas fitted to the current window. Focus is visibly indicated,
and buttons work by keyboard. An enabled button should not trap Tab navigation.
If fullscreen is unavailable/denied, a retry cue appears and windowed play continues.
Record unsupported/denied fullscreen separately from a successful entry/exit test.

### B07 — Window size, zoom and display changes (5 minutes)

1. In gameplay, resize the window from large to roughly half-screen and back.
2. Through the browser menu set zoom to 125%, then 80%, then back to 100%.
3. Enter/exit fullscreen at 125% once, then restore 100%.
4. If available, move the window to a second monitor with different scaling and back.
5. Repeat the resize/zoom checks in fresh Co-op (B10).

**Expected:** the image fits, aiming still works, text is readable, and controls/help
remain reachable. Split-screen has no stale strips, overlap or gap at its divider.
Minor text antialiasing differences are acceptable observations; disappearing HUD,
cut-off buttons or severe aspect distortion are failures. Record sizes/zoom values
that fail. A missing second monitor blocks only step 4.

### B08 — Real gamepad controls (5 minutes)

1. Start fresh single-player with the controller disconnected. Confirm keyboard play.
2. Connect a standard gamepad while the game is running. Press and release a face
   button to let the browser detect it, then release all controls for a moment.
3. Exercise each gamepad action in the controls table. Release the sticks between tests.
4. Put the controller down and test keyboard movement/aiming again.

**Expected:** the pad works without reloading; released controls do not keep firing
or turning. Keyboard play remains available when the pad is idle. Record brand,
model, USB/Bluetooth connection and incorrect physical-button mappings.

### B09 — Gamepad disconnect, reconnect and held-input recovery (5 minutes)

1. While moving/firing with the pad, disconnect it. Confirm keyboard play still works.
2. Reconnect, press/release a button if needed, release all controls, and try again.
3. Switch applications while holding a pad button/stick; release it while away.
   Return and check for unwanted movement/firing, then issue a fresh input.
4. Where practical, reconnect while holding Fire. Keep it held briefly, then release
   **all** controls and press Fire again. Repeat disconnect/reconnect three times.

**Expected:** no crash or stuck action. Held controls after connection/focus changes
are suppressed until released/neutral, then fresh input works. If the device cannot
be reconnected while held, record that step as Blocked rather than inferring a pass.

### B10 — Fresh Co-op and Versus, independent players (10 minutes)

1. Connect two gamepads and let the browser detect both by pressing/releasing a button
   on each. Label them Pad 1 and Pad 2. Start a fresh Co-op session using Right, Enter.
2. Check two views/HUDs. Player 1 is normally the lower view; player 2 the upper.
   Move/fire with keyboard/mouse, then each pad separately. Record which view each
   controls. The second connected pad is normally player 2's pad; a single pad
   does not establish full player-2 control coverage.
3. Keep one player idle while the other moves, aims, fires, jumps and changes camera.
   Repeat for the other player. Disconnect/reconnect one pad and check that the
   other player's device has not changed player assignment.
4. Reload for a fresh session. Select Versus with Right, Down, Enter and repeat.
5. In each mode resize once and check that HUD text/effects stay in the correct view.

**Expected:** both views continue updating, controls affect the intended player,
HUD values belong to that player, and neither view erases the other. Record gameplay
or respawn failures even though multiplayer robustness is a known deferred area.
Without two pads, perform the visual checks and mark independent-player checks Blocked.

### B11 — Mode transitions and repeated restart (10 minutes; known-risk test)

1. From a fresh session play Single player, return toward the menu with Escape,
   select Co-op (Right, Enter), then return and select Single player (Left, Enter).
   Check the selected label before pressing Enter. Record if menu/legend fails to return.
2. Repeat with Versus, then Single player. Continue for five mode changes; fire and
   move in every session. Where possible include a death/respawn.
3. If the game crashes, save evidence and reproduce from a fresh session once.

**Desired result:** selecting Single player restores one view; each new mode starts
correctly, menu indication is usable, and transitions do not crash or retain stale
player state. Historical runs retained two views after selecting Single player and
crashed in some multiplayer/respawn sequences. Record recurrence as **Fail — known
issue**, not Pass. The owner decides whether that failure is acceptable for release.
A fresh-mode pass in B10 does not clear this case.

### B12 — Levels and graphics review (10–30 minutes)

1. Start fresh single-player using keyboard/mouse. Photograph or screenshot the
   first level and HUD before moving the camera.
2. Move, fire, jump and look around. Observe terrain textures, enemy bodies/turrets,
   player rings/stars, bullets, items, explosions, shadows/lighting where present,
   and transparent surfaces such as water where present.
3. With controls released, tap T once to advance. Wait for the scene to settle;
   record a screenshot and repeat the basic movement/fire check.
4. For full coverage, work through the 21 playable maps: `0–9`, then `A–K` (title is
   the 22nd packaged map). Stop after K; end-of-list behavior is a separate observation.
   Use a level checklist. If play/death changes the level unexpectedly or the map
   cannot be identified, ask the owner to identify it from the log; do not guess.
5. Compare assigned scenes with the approved reference at the same camera/window
   size. Record any object/effect you could not encounter for an assisted follow-up.

**Expected:** each loaded map is playable and has visible terrain/entities/HUD.
No persistent blank regions, missing textures, flashing surfaces, vanished tanks,
incorrect transparency, unreadable text or effects leaking across viewports.
Visual differences need owner review when no tolerance/reference is supplied.
Loading every map alone does not prove every enemy/effect variant was exercised.

### B13 — Failed download and retry (owner-provided setup; 5 minutes)

1. Open the owner's intentionally broken test URL in a fresh window. The HTML page
   must load while its game JavaScript download fails; an entirely offline browser
   error page does not exercise the in-game retry control.
2. Check for an understandable failure message and **Reload and retry** button.
3. Ask the owner to restore the missing download. Click **Reload and retry**.
4. Verify the menu appears and Single player starts. Save the failure screenshot.

**Expected:** failure is visible rather than an endless blank/loading page, and
retry successfully starts the repaired build. Mark Blocked if no fault-testing
setup is supplied. Do not delete game files or change system network settings.

### B14 — Stop, close and start again (3 minutes)

1. With sound enabled, release capture and exit fullscreen. Return toward the menu
   with Escape, then press Escape again only after checking the game state.
2. Confirm gameplay stops, game-created audio/capture/fullscreen controls disappear,
   the pointer is free and audio stops. Reload and start again.
3. Close the tab during play with sound enabled; reopen the supplied URL.

**Expected:** no continuing game sound or trapped mouse after stopping/closing.
A reload starts a fresh game and requires sound activation again. Browser settings
are intentionally temporary: previous level/progress/music-pause choice need not
survive reload. Do not expect a save-game or settings-persistence feature.

### B15 — Sustained play / soak (60 minutes)

1. Start a fresh Single player session; enable sound. Record the time and visible
   FPS number (frames per second), if shown, after the first minute.
2. Play for 60 minutes. Every 10 minutes record FPS, responsiveness and any audio or
   visual degradation. Alternate movement, firing, jumping and normal level progress.
3. At minutes 15 and 45, switch tabs for 30 seconds and return. At minute 30, enter/
   exit fullscreen and resize. Keep the build and settings consistent throughout.
4. Finish with a screenshot and clean shutdown. If a crash interrupts the run,
   report elapsed time; do not label a shorter run a completed soak.

**Expected:** no crash, accumulating audio, progressively worse controls, or visible
corruption. Compare similar scenes when reporting FPS drops; extra enemies/effects
can legitimately change performance. Use the owner's performance target if supplied.
This visual test cannot prove absence of memory leaks; memory profiling is an
engineering follow-up. Multi-hour and multiplayer soaks require separate assignment.

### B16 — Diagnostic text and menu legend (3 minutes)

1. Start fresh at the title. Press I once to enable diagnostic information; this
   key may also play a short sound if sound is enabled. Take a screenshot.
2. Start Single player, move and fire, and observe the FPS/debug text. Reload,
   press I at the title, and enter Co-op for a second screenshot.

**Expected:** text is legible without covering the essential player meters.
Global diagnostics/menu information appears once, while player HUDs stay in their
own views. The mode legend belongs to the menu and should hide during play.
When Calls/Tris counters are shown in Modern, they should report nonzero work for
an active scene; their exact values can vary. Compare text placement with the
owner's reference. Legacy Compatibility text/counters have known limitations.

## 5. Desktop checks (only assigned builds)

### N01 — Launch and play (10 minutes per renderer)

1. Extract the entire supplied package to a local folder. Keep assets beside the
   executable as supplied; use the owner's launcher, not a copied executable.
2. Record **Modern** or **Compatibility** from the launcher. Have the owner confirm
   the `Renderer mode:` log line if uncertain. Do not infer mode from appearance.
3. Run B01's menu navigation and B02's gameplay steps, ignoring browser-only buttons.
   Native builds may capture the mouse and play music immediately; that is expected.
4. Run the music-pause portion of B03, resize the window if supported, and quit with
   Escape through the menu. Relaunch and repeat a short movement/fire check.

**Expected:** the selected renderer launches, assets load, gameplay/HUD are usable,
audio works, and shutdown releases the pointer/audio. Browser-specific activation,
pause cues and focus suspension are not native requirements.

### N02 — Visual comparison (10–20 minutes per renderer)

1. Repeat B12's assigned reference scenes, B10's fresh split-screen visual checks
   and B16's diagnostic text check.
2. Match level, mode, approximate tank/camera position and window size to the reference.
3. Save matching screenshots labelled with platform and renderer. Describe the
   affected object/area rather than saying only “looks different.”

**Expected:** required world objects, effects and usable HUD are present. Legacy
compatibility text has known differences/placeholders; text visible only in Modern
is not automatically a regression. Rendering differences need owner acceptance;
the tester does not approve removal of the compatibility renderer.

### N03 — Lifecycle and deferred sequences (10 minutes, plus soak if assigned)

1. Run the B11 transition sequence and record results for this desktop renderer.
2. Connect/disconnect a controller if the supplied native control setup supports it.
3. Repeat quit/relaunch three times; perform B15 if assigned for this platform.

**Expected:** no new crash, missing resources or lost device control. Record
known multiplayer failures separately. Passing Linux does not qualify Windows or
macOS, and passing Compatibility does not qualify Modern.

## 6. Developer-assisted checks

These cases are part of release coverage but do not require the tester to debug
code. If no helper is available, mark them Blocked and continue the ordinary cases.

### A01 — Graphics interruption and recovery

The developer can simulate losing the browser's graphics connection without
restarting the game. Start Single player, enable sound and note the current scene.
Have the developer run the snippet below in that game's browser console, then
return focus to the game. Do not disable browser security controls to paste code.

```js
const qaCanvas = document.querySelector('#canvas');
const qaGL = qaCanvas && qaCanvas.getContext('webgl');
const qaLoss = qaGL && qaGL.getExtension('WEBGL_lose_context');
if (!qaLoss) {
    console.log('QA BLOCKED: context-loss simulation unavailable');
} else {
    setTimeout(() => {
        qaLoss.loseContext();
        setTimeout(() => qaLoss.restoreContext(), 8000);
    }, 3000);
}
```

**Tester observes:** after roughly three seconds the game pauses with a graphics
interruption message and audio suspends. About eight seconds later rendering returns
without a fresh game. Release controls, then move/fire. No catch-up jump, missing
HUD/textures or stuck input should remain. Repeat twice and once in split-screen.
A normal “return to this window” pause caused by console focus is not proof of
successful graphics recovery. Record an unavailable extension as Blocked.

### A02 — Recovery failure and permission rejection

Ask the developer to use the checked-in browser harness or an isolated fault setup
for shader-allocation failure, rejected audio activation, and pointer/fullscreen
rejection that the browser cannot reproduce through ordinary permission prompts.
Record these as **assisted/automated** evidence, separate from physical manual results.

**Expected:** failed graphics rebuilding stops play and audio and offers reload;
rejected audio/capture/fullscreen actions show a retry explanation and leave other
controls usable. After removing the injected failure, retry/reload works. Never
modify the release files in place to perform these cases.

## 7. Reporting failures and handing results back

For each failure, use the bug template in the results sheet and include:

- Exact build/URL, browser/OS version, renderer, controller and test-case ID.
- Whether it started fresh or followed another mode/level; steps in order, including
  Escape, blur/tab changes, controller order, and approximate time playing.
- Expected and actual behavior, frequency (for example 2 of 3 attempts), and what
  restored operation. “Could not reproduce on retry” does not erase the first failure.
- Screenshot or short video showing the whole game and relevant controls. Include
  sound in a recording for audio bugs if your recorder supports it.
- If available, the error text or log attached by the owner. A tester without console
  experience can ask the owner to collect it; a useful report does not require a stack trace.

Suggested severity: **Critical** = crash/freeze/cannot load; **Major** = required
input, sound, recovery or player view unusable; **Minor** = cosmetic issue with a
usable workaround. The owner sets final priority and release impact.

Hand back the results sheet, evidence folder and bug list. Release sign-off needs
all assigned required cases passing or an explicit owner acceptance of each open
failure. Blocked/Not run cases remain coverage gaps. Known deferred bugs are still
failures; no tester should silently waive them. Visual approval and compatibility
retirement remain separate owner decisions.

## 8. Reference material for the build owner

- [Current roadmap](current-state-roadmap.md): current scope, deferrals and release gates.
- [Browser build/hosting and automated evidence](../../documentation/browser-release-2026-10-28/README.md).
- [Historical multiplayer failures](../../documentation/manual-review-2026-09-29/README.md).

The owner should launch native builds with the working directory set to `runtime`,
where `settings.txt`, textures, levels and sounds live. On Linux/macOS, a modern
launch is `TANKGAME_RENDERER=modern ./tankgame-linux` or
`TANKGAME_RENDERER=modern ./tankgame-mac` from that directory. Compatibility on
supported desktops requires the variable unset. Windows launchers should set the
same environment variable and use the actual executable filename in the package.
Supply ready-to-run launchers to the tester rather than requiring shell knowledge.
