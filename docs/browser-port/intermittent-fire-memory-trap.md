# Intermittent browser firing trap

**Status: fixed and verified (2026-09-28).**

## Cause

Bullet collision events store raw `Entity*` pointers. Each gameplay tick processes queued events first, then updates bullets, then updates players. A bullet update can queue a collision after event processing. If the player dies in that tick, respawn calls `LevelHandler::NextLevel`, which clears and destroys world entities while the bullet event remains queued. The next tick dispatches it to `CombatSystem`, which dynamically casts the now-dangling bullet pointer.

This is a heap use-after-free during level teardown. It is distinct from the fixed 1 MiB WASM stack issue. It does not require bullet counts or memory to grow: world clearing removes the bullets. The failure is intermittent because the queue only contains a stale bullet when a collision and respawn teardown coincide. Reuse of the freed address can change an invalid access from a trap into apparently normal behavior.

## Resolved stack

A native ASan/UBSan regression queued a `BulletTimeoutEvent`, reloaded the level (destroying its bullet), and processed the queue. Before the fix it failed in `__dynamic_cast` from `CombatSystem::OnBulletTimeout`, reached through `EventBus::ProcessQueuedEvents`.

A deterministic browser ASan reproduction queued a `BulletLevelCollisionEvent` for a world-owned bullet and performed the same reload. It trapped with `RuntimeError: memory access out of bounds`. The resolved WASM stack was:

```text
is_equal(std::type_info const*, std::type_info const*, bool)
__dynamic_cast
CombatSystem::Initialize()::$_1::operator()(BulletLevelCollisionEvent const&)
EventBus::Subscribe<BulletLevelCollisionEvent>(...)::lambda(Event const&)
EventBus::Dispatch
EventBus::ProcessQueuedEvents
__original_main
main
```

The browser stack/overflow check was enabled; there was no stack overflow. The failing access was the freed bullet passed to `__dynamic_cast`.

The original report observed one failure in two manual attempts, within roughly the first 20 seconds of firing. Eleven additional unmodified 60-second trials did not hit the rare timing window. This is consistent with the short collision-event/respawn overlap and allocator-dependent symptoms. The deterministic lifecycle regression failed before the fix and passed after it.

## Fix

`LevelHandler::NextLevel` now calls `Events::Clear()` before `GameWorld::Clear()` destroys the departing level's entities. `GameTask::SetUpGame` already follows this policy for a new game. Clearing queued events leaves subscriptions intact, and fresh events on the reloaded level continue to dispatch normally. No firing, collision, renderer, memory limit, stack size, or entity cap changed.

The focused regression also posts fresh bullet and FX events after reload and verifies that the new-level timeout and effect handling still work.

## Validation

- Optimized Emscripten Release build links.
- Five headed Chromium runs held left mouse to fire continuously for at least 60 seconds each: **300+ seconds total**. Every run crossed multiple player-death respawns and level reloads (bullet counts ranged from 0 to 21); none trapped. Escape stopped the frame loop in every run.
- Release performance averaged **59.73–59.86 FPS** per run; typical tick time was **0.4–0.7 ms**. Every WebGL probe returned `NO_ERROR`, with no lost context.
- WASM heap stayed at **18,350,080 bytes** for each run. JavaScript heap samples varied with garbage collection.
- Fixed diagnostic ASan/stack-check WASM reload reproduction completed and printed `RELOAD REGRESSION PASSED`; no ASan memory or stack error was reported for the stale-event path.
- The 20-run native ASan/UBSan regression passed. Before the fix, its first run trapped in `__dynamic_cast`.
- Native Release build and **81/81 tests** passed.

The ASan WASM build reserves much more memory and runs more slowly than Release. It also reports existing shutdown leaks in the full game harness; the focused reload reproduction exits without an ASan memory/stack report. The normal Release heap remained flat throughout sustained fire.

## Changed files and scope

- `src/LevelHandler.cpp`: discard queued old-level events before entity destruction.
- `tests/test_event_lifecycle.cpp`: regression for queued events across reload and normal event handling afterward.
- `docs/browser-port/intermittent-fire-memory-trap.md`: cause, stack, fix, and validation record.

No renderer or split-screen work was done. Linux modern mode was rebuilt and launched from `runtime`; native firing was exercised in the existing manual run earlier in this review. The existing local modification to `runtime/applog.txt` was refreshed by game runs. No commit was made.
