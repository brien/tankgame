# Special-attack bullet restoration

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

## Verification

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
