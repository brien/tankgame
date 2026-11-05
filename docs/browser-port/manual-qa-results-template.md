# Tankgame manual QA results

Copy this file for each browser/OS/renderer combination. Follow the
[manual test plan](manual-qa-test-plan.md). Keep the original template blank.

## Run details

| Field | Value |
| --- | --- |
| Tester / date | |
| Build / commit | |
| Browser package version, if supplied | |
| Game URL or desktop package name | |
| Operating system and version | |
| Browser and version (or Desktop) | |
| Renderer: Browser modern / Desktop modern / Desktop compatibility | |
| Computer model / graphics hardware, if known | |
| Monitor resolution / OS scaling, if known | |
| Browser zoom / approximate window size | |
| Gamepads: brand/model, connection, order connected | |
| Audio output device | |
| Assigned cases / required platforms | |
| Approved reference build or screenshots | |
| Owner's loading/FPS targets, or “not supplied” | |
| Evidence folder / bug tracker | |

## Case results

Use **Pass**, **Fail**, **Blocked**, or **Not run**. For partial coverage, name the
passed and missing steps and use Blocked for an incomplete required case (or Fail
if any tested required step failed). Label “Fail — known issue” in Notes when
applicable. Do not convert known failures into passes. Unassigned cases can remain
Not run with “not assigned” in Notes.

| ID | Test | Result | Notes, failed/blocked step, bug ID, evidence filename |
| --- | --- | --- | --- |
| B01 | First load, reload, menu | Not run | |
| B02 | Gameplay and HUD | Not run | |
| B03 | Sound, music pause, audio device | Not run | |
| B04 | Focus, hidden tab, held controls | Not run | |
| B05 | Mouse capture, physical Escape, retry | Not run | |
| B06 | Fullscreen, keyboard-accessible buttons | Not run | |
| B07 | Resize, zoom, display changes | Not run | |
| B08 | Real gamepad controls | Not run | |
| B09 | Gamepad reconnect and input recovery | Not run | |
| B10 | Fresh Co-op/Versus, independent players | Not run | |
| B11 | Mode transitions/restarts, known-risk sequence | Not run | |
| B12 | Levels and graphics review | Not run | |
| B13 | Failed download/retry, owner setup | Not run | |
| B14 | Stop, close, restart | Not run | |
| B15 | 60-minute soak | Not run | |
| B16 | Diagnostic text/menu legend | Not run | |
| N01 | Desktop launch/play | Not run | |
| N02 | Desktop visual comparison | Not run | |
| N03 | Desktop lifecycle/transitions | Not run | |
| A01 | Assisted graphics loss/recovery | Not run | |
| A02 | Assisted failures/permission rejection | Not run | |

## Loading and performance observations

- First download method (private window / owner cleared site cache / other):
- First load time:
- Normal reload time:
- Soak start/end times and actual duration:
- Interrupted? When and why:

| Soak minute | Visible FPS, if shown | Scene/actions and responsiveness | Audio/visual issues or evidence |
| --- | --- | --- | --- |
| 1 | | | |
| 10 | | | |
| 20 | | | |
| 30 | | | |
| 40 | | | |
| 50 | | | |
| 60 | | | |

Do not invent a performance target. If none was supplied, record observations for
owner review. A shorter session does not pass the 60-minute case.

## B12 level coverage

Mark each map Pass/Fail/Blocked/Not run. Note how the level was identified; an
unknown map does not establish coverage of a specific level. The title is checked
in B01; playable maps are listed below.

| Level | Result | Screenshot / notes |
| --- | --- | --- |
| 0 | Not run | |
| 1 | Not run | |
| 2 | Not run | |
| 3 | Not run | |
| 4 | Not run | |
| 5 | Not run | |
| 6 | Not run | |
| 7 | Not run | |
| 8 | Not run | |
| 9 | Not run | |
| A | Not run | |
| B | Not run | |
| C | Not run | |
| D | Not run | |
| E | Not run | |
| F | Not run | |
| G | Not run | |
| H | Not run | |
| I | Not run | |
| J | Not run | |
| K | Not run | |

Objects/features observed (list evidence or write “not encountered”):

- Player body/turret, rings/stars:
- Enemy types/colours:
- Terrain textures and lighting:
- Water/transparency:
- Bullets, explosions and other effects:
- Items/pickups:
- Health/energy meters and status text:
- Menu selection/legend and FPS text:
- Single-view and split-screen HUD boundaries:
- Assigned visual-reference differences requiring owner review:

## Bug report (copy for each failure)

**Title:** [Case ID] Short description of the problem

**Severity suggestion:** Critical / Major / Minor

**Build and environment:**

**Starting state:** Fresh page/process, or preceding mode/level/actions?

**Steps to reproduce:**
1.
2.
3.

**Expected result:**

**Actual result:**

**Frequency:** ___ failures in ___ attempts; first seen after ___ minutes

**Recovery/workaround:**

**Evidence filenames / error text:**

**Known issue match, if any:**

**Bug tracker link / owner response:**

Example evidence name: `B09_Windows_Chrome_Modern_pad-reconnect_01.mp4`.
For screenshot comparisons include renderer, mode and level in both filenames.

## Handoff summary and owner disposition

- Assigned required cases:
- Passed:
- Failed (bug IDs, including known failures):
- Blocked/Not run (reason and follow-up owner):
- Assisted/automated results (identify helper/tool; separate from manual checks):
- Visual comparisons still awaiting approval:
- Other coverage gaps (devices, levels, effects, browsers):
- Evidence folder handed over:

Owner to complete:

- Open issues accepted for this release, with reason:
- Retest assignments:
- Release disposition: Ready / Not ready / Pending evidence
- Owner / date:

This sheet does not authorize compatibility-renderer retirement. That decision
requires separate visual approval and an explicit owner decision.
