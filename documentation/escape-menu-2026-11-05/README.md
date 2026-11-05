# Escape menu visibility regression

Escape transitioned GameTask to MENU while the independent gameStarted flag
remained true. SceneDataBuilder consequently hid the menu despite menu input
being active. IsGameStarted now derives from the current state, preserving the
gameplay view during GAME_OVER and restoring the menu on either return path.

Validation:
- `cmake --build build -j 4` passed.
- `TANKGAME_TEST_GL=1 ctest --test-dir build --output-on-failure`: 150/150 passed.
- New state/UI regression tests cover repeated play/menu transitions and the
  game-over return path.
- Native modern-renderer smoke: Enter, Escape, Right, Enter, Escape, Escape.
  Menu visibility and selection were inspected in screenshots after single-player
  and co-op; the process exited with status 0 after the final Escape.
- Browser runtime was not retested.

The menu overlays the existing world. Existing world updates, gameplay HUDs,
and split-screen layout continue behind it; this change does not implement pause
or redesign the menu. Screenshots record that behavior.
