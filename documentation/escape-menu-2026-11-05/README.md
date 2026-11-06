# Escape menu visibility regression

Escape transitioned GameTask to MENU while the independent gameStarted flag
remained true. SceneDataBuilder consequently hid the menu despite menu input
being active. IsGameStarted now derives from the current state, preserving the
gameplay view during GAME_OVER and restoring the menu on either return path.

Validation:
- `cmake --build build -j 4` passed.
- `TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure`: 152/152 passed.
- New state/UI regression tests cover repeated play/menu transitions and the
  game-over return path, frozen entity motion/age over menu frames, and immediate
  simulation stop on the Escape input frame.
- Native modern-renderer smoke: Enter, Escape, Right, Enter, Escape, Escape.
  Menu visibility and selection were inspected in screenshots after single-player
  and co-op; a three-second menu wait confirmed stationary world entities.
  The process exited with status 0 after the final Escape.
- Browser runtime was not retested.

The initial visibility fix still called GameWorld::Update from the menu handler.
The follow-up removes that call and handles Escape before any gameplay updates,
so movement, combat, and effect lifetimes stop immediately and stay frozen while
the menu is open. Selecting a mode starts a new game; this is not a resume menu.

The menu still overlays the world with the existing HUD and split-screen layout.
Rendering continues (including cosmetic targeting animation and FPS displays);
simulation is stopped. Screenshots record that behavior.
