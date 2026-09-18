# Special-attack bullet restoration

## Session handoff — 2026-09-18

Status: the original restoration plan and both subsequently requested follow-ups
are complete. No implementation task is currently in progress or assigned for
the next session. The user reported that manual gameplay testing seemed to work
correctly; that was user validation, not an automated visual check.

### Completed plan

- [x] Propagate special status through Bullet, GameWorld, and Tank creation;
  retain ordinary-shot defaults and existing firing patterns/costs/cooldowns.
- [x] Restore blue enemy piercing and activate existing purple acceleration.
- [x] Restore all yellow bounce children through world-owned prototype copies,
  retaining their identity, damage, types, colors, motion, and bounce limits.
- [x] Keep child bullets non-special and preserve queued collision timing.
- [x] Add seven headless bullet tests using real gameplay classes and the shared
  CMake source list; cover both player identities and world lifecycle behavior.
- [x] Fix the sanitizer-discovered Tank collision-array initialization bug.
- [x] Complete manual gameplay validation (user report).
- [x] Add explicit event unsubscription for CollisionSystem, CombatSystem, and
  GameWorld; support safe removal during dispatch and automatic destructor cleanup.
- [x] Add eight event-bus/lifecycle tests and remove the bullet fixture's reliance
  on `ClearAll()` for subscription cleanup.
- [x] Verify the full build, 57 CTest tests, and three shuffled full-suite runs
  both normally and under ASan/UBSan with leak detection.

### Delivered commits

| Commit | Result |
| --- | --- |
| `df3ef65` | Special status, blue collision survival, and initial bullet tests |
| `d17c98f` | Yellow secondary spawning, child/lifecycle tests, implementation note |
| `25aa90f` | Zero-initialize all 21 tank collision points; remove out-of-bounds write |
| `b34124b` | Event subscription cleanup, safe dispatch, and eight lifecycle tests |

### Resume here

Read this note and inspect `git status --short` before making changes. All known
follow-ups raised in this conversation are resolved; obtain the next task from
the user rather than repeating the restoration or starting an unrelated refactor.

Preserve existing unrelated workspace content: `runtime/applog.txt` is modified,
and the checkout contains pre-existing untracked notes, documentation, IDE files,
screenshots, runtime executables, and other runtime files. Do not bulk-stage or
clean these up as part of this task.

Compatibility boundaries remain intentional: no weapon rebalance, grey-primary
special, BulletHandler restoration, rendering changes, or respawn redesign.
Event subscriptions now have explicit ownership, but event payloads still contain
raw entity pointers and level transitions still clear pending events. The cleanup
change does not provide ownership of queued payloads or per-world event routing.

### Verification to reproduce

```sh
cmake -S . -B build
cmake --build build -j4
ctest --test-dir build --output-on-failure
./runtime/tankgame_tests --gtest_shuffle --gtest_random_seed=381 --gtest_repeat=3
```

Latest result: 57/57 tests passed; all three shuffled runs passed (seeds 381–383).
The relevant new test files are `tests/test_bullet_special.cpp` and
`tests/test_event_lifecycle.cpp`.

An isolated sanitizer build exists in `build/sanitize`, with its test executable
at `/tmp/tankgame-special-sanitize/tankgame_tests`. Recreate it if temporary files
or build outputs are missing:

```sh
cmake -S . -B build/sanitize -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer' \
  '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined' \
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=/tmp/tankgame-special-sanitize \
  -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST="$PWD/build/_deps/googletest-src"
cmake --build build/sanitize --target tankgame_tests -j4
env ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  /tmp/tankgame-special-sanitize/tankgame_tests \
  --gtest_shuffle --gtest_random_seed=381 --gtest_repeat=3
```

The sanitizer build's test-discovery step and test execution need to run outside
the process-tracing sandbox: LeakSanitizer fails under ptrace. Request tool
escalation if needed; do not disable leak detection to conceal that limitation.
All 57 tests passed three sanitizer runs with halt-on-error and leak detection
active. Last-run logs, if still present, are `/tmp/tankgame-event-build.log`,
`/tmp/tankgame-event-ctest.log`, `/tmp/tankgame-event-shuffle.log`,
`/tmp/tankgame-event-sanitize-build.log`, and
`/tmp/tankgame-event-sanitize-run.log`.

## Implementation details

`fae5714` moved tank-owned bullets into `BulletHandler` and disabled the four
secondary-spawn paths. `275715f` subsequently migrated bullets into `GameWorld`.
This implementation uses current world ownership and collision events; it does
not restore `BulletHandler`.

Every projectile emitted by `Tank::Special` now carries special status. Ordinary
shots default to non-special. Blue player specials survive damaging enemy hits
and gain 100 power after applying damage. Player collisions still consume them,
including healing and self-hits. Existing purple special angular acceleration is
now reachable: magnitude increases by 1000 degrees/second per second, retaining
the sign (zero turns positive). This activates dormant behavior as well as
restoring spawning; historical constructors did not reliably establish special
status.

Yellow player specials create children on successful bounces:

| Secondary | Count | Behavior |
| --- | ---: | --- |
| Red | 20 | Red/red, red colors, headings 0–342 degrees in 18-degree steps, no bounces |
| Blue | 2 | Inherited types/colors, reflected heading and existing lateral offsets, four bounces |
| Yellow or grey | 1 | Explicit grey/grey types, inherited colors, negative original heading, four bounces |
| Purple | 1 | Inherited types/colors, reflected heading, twice parent power, angular velocity 10, sixteen bounces |

First-bounce power increases happen before child damage is assigned. Children
start at age and bounce count zero, retain the firing player's identity, and are
non-special to prevent recursive splitting. The prototype creation overload
copies configured values without reapplying purple constructor damage changes,
then attaches the world and registers the ordinary bullet collision shape/layer.

Collision events remain processed at the start of the next gameplay tick, before
world updates. Children are inserted during that event phase and participate in
the ensuing update and normal world cleanup. No additional spawn queue is used.
Firing patterns (including overlapping yellow shots), charge costs, cooldowns,
damage calculations/limits, and repeated enemy-hit behavior are preserved. No
grey-primary special was added.

## Restoration verification history

The headless fixture links real gameplay sources, initializes `App`, `GameTask`,
a separate world and player-manager connection, clears queued events (subscription
cleanup is now owned by each world/system), initializes both level grids with `Load(nullptr)` then flattens terrain,
fixes/restores delta time, and disables sound. It never starts the graphical app.
Tests cover both player identities and every yellow combination, child values,
full bounce allowances, non-recursion, special eligibility, queued wall events,
child updates/cleanup, firing counts/charge/cooldowns, blue survival, and purple
positive/negative/zero acceleration.

- Full configure/build and CTest suite: 49/49 passed.
- New tests shuffled three times: seeds 173, 174, 175 passed.
- Follow-up (2026-09-18): `Tank::collisionPoints` now uses zero initialization
  at its declaration; the constructor's out-of-bounds write to index 21 was
  removed. Full game/test builds and all 49 CTest tests pass after the fix.
- ASan/UBSan: all seven bullet tests now pass three shuffled runs (seeds 173,
  174, 175) with halt-on-error enabled and leak detection active. The sanitizer
  executable was built separately in `build/sanitize` with
  `-fsanitize=address,undefined -fno-omit-frame-pointer`; it ran outside the
  sandbox because LeakSanitizer cannot run under ptrace.
- Manual gameplay validation: the user reported that testing was working
  correctly after the constructor fix.

## Event-subscription cleanup follow-up (2026-09-18)

`EventBus::Subscribe` accepts an optional owner, and `Unsubscribe(owner)` removes
only that owner's callbacks across event types. Existing unowned subscriptions
remain supported. CollisionSystem, CombatSystem, and GameWorld's FX handler now
register their owners, unsubscribe during shutdown, and call shutdown from their
destructors. Repeated initialization replaces callbacks without duplication;
repeated shutdown is safe. These subscribers cannot be copied or moved because
their callbacks capture their addresses.

Synchronous and queued dispatch share a callback snapshot. Removing a subscription
marks it inactive so it cannot run later in that snapshot; its callable remains
alive while an in-flight callback finishes. New subscriptions participate in the
next dispatch, including nested publishes. Shutdown preserves other subscribers
and queued events. Event payload lifetimes and level-transition queue clearing
remain unchanged.

Eight new tests cover owner isolation, removal during dispatch, nested dispatch,
local-bus clearing, repeated world shutdown/destruction, queued delivery after
destruction, reinitialization, standalone systems, and another live world's
subscriptions. The global lifecycle fixture does not clear subscriptions or the
queue; the bullet fixture no longer uses `ClearAll()` either.

Verification: full game/test build and all 57 CTest tests pass. The entire suite
also passed three shuffled runs (seeds 381–383), both normally and with ASan/UBSan
halt-on-error and leak detection enabled.
