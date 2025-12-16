# Tank Game - AI Agent Instructions

## Project Overview
A 3D tank combat game built with OpenGL, SDL2, and C++14. Originally started in 2003, currently undergoing architectural modernization with focus on decoupling rendering from game logic and implementing event-driven systems.

## Architecture

### Task-Based Execution Model
The game uses a priority-based task system (`TaskHandler`) that runs a main update loop executing tasks in priority order:
- Tasks inherit from `ITask` and implement `Start()`, `Update()`, `Stop()` lifecycle methods
- Priority order: VideoTask(100) → InputTask(90) → GraphicsTask(80) → SoundTask(70) → GameTask(60) → GlobalTimer(10)
- Tasks can be suspended/resumed via `OnSuspend()` and `OnResume()`
- Main execution loop in `main.cpp`: instantiate singletons → add tasks → call `TaskHandler::Execute()`

### Singleton Pattern Usage (LEGACY - Being Phased Out)
**Important:** The project is actively moving AWAY from singleton pattern for game entity management. Prefer dependency injection and composition over singletons for new code.

Core managers currently use template-based `Singleton<T>` pattern (see `Singleton.h`):
- Manual lifecycle: instantiate with `new TankHandler()` in `main.cpp`, cleanup at shutdown
- Access via `ClassName::GetSingleton()` or `::GetSingletonPtr()`
- Examples: `TankHandler`, `LevelHandler`, `FXHandler`, `TaskHandler`
- **Exception:** `PlayerManager` is NOT a singleton - owned by `GameTask` (modern pattern)

**Migration Strategy:**
- New entity management goes through `GameWorld` (NOT singletons)
- Handlers are transitioning to interface roles, not entity owners
- When adding features, inject dependencies rather than accessing singletons directly
- Only Tasks and core infrastructure should remain as singletons

### Entity-Component-System Evolution
The project is transitioning from direct entity management to centralized systems:

**GameWorld** (`GameWorld.h`) - Central coordinator owning:
- `EntityManager<T>` templates for Tanks, Bullets, FX, Items (see `EntityManager.h`)
- `CollisionSystem` - spatial queries, layer-based collision detection
- `CombatSystem` - damage application, bullet-tank interactions
- **Pattern:** Handlers (TankHandler, etc.) now act as interfaces to GameWorld, not direct entity owners

**Entities** inherit from `Entity` base class with:
- Position/rotation/velocity, `IsAlive()` state, `Update()` and `OnDestroy()` lifecycle
- Examples: `Tank`, `Bullet`, `Item`, `FX` (effects)

### Event-Driven Communication
Modern code uses `EventBus` for decoupled communication (see `events/EventBus.h`):
```cpp
// Publishing events
EventBus::Get().Publish(BulletCollisionEvent{bullet, tank});

// Subscribing to events
EventBus::Get().Subscribe<BulletCollisionEvent>([](const auto& e) { /* handle */ });
```
Event types in `events/CollisionEvents.h`, `events/Events.h`. Systems communicate via events rather than direct calls.

### Rendering Architecture (Recently Refactored)
**Critical:** Rendering is now fully data-driven and decoupled from game logic (Phases 4-6 complete):

**Data Flow:** Game State → `SceneDataBuilder` → `SceneData` → `RenderingPipeline` → Specialized Renderers

**Key Components:**
- **SceneDataBuilder** (`rendering/SceneDataBuilder.h`) - Extracts all renderable data from game handlers into pure data structs
- **RenderData Structures** (`rendering/RenderData.h`) - POD structs: `TankRenderData`, `BulletRenderData`, `TerrainRenderData`, `UIRenderData`, etc.
- **RenderingPipeline** (`rendering/RenderingPipeline.h`) - Orchestrates rendering order, viewport setup, delegates to specialized renderers
- **Specialized Renderers:** `TerrainRenderer`, `BulletRenderer`, `TankRenderer`, `HUDRenderer`, `MenuRenderer`
- **ResourceManager** - Manages OpenGL display lists, textures, fonts
- **ViewportManager** - Split-screen layout (single-player vs 2-player)
- **CameraManager** - Per-player camera positioning

**Important:** Game logic classes (Tank, Bullet, etc.) should NOT contain OpenGL calls. Extract data to RenderData structs.

### Tank Renderer Polymorphism
Tank rendering uses strategy pattern:
- `ITankRenderer` interface → `PlayerTankRenderer` / `EnemyTankRendererImpl`
- Created via `TankRendererFactory` based on `TankRenderData.isPlayer`
- Player tanks show health bars; enemies render differently

## Development Workflows

### Building
**From `src/` directory:**
```bash
./build.sh  # Auto-detects OS (macOS/Linux), compiles all .cpp recursively
# Output: ../runtime/tankgame-{mac|linux}
```
Build uses: g++ -std=c++14, links SDL2, SDL2_mixer, SDL2_ttf, OpenGL, Assimp

**Alternative:** VS Code task "C/C++: g++ build active file" (builds single file, less useful)

### Running
```bash
cd runtime
./tankgame-linux  # or ./tankgame-mac
# Loads levels from ./levels/, textures from ./texture/, sounds from ./sounds/
```
Working directory MUST be `runtime/` for asset loading.

### Debugging with Distrobox (SteamDeck/Flatpak environments)
Task available: "Clang-Tidy in Distrobox" - runs clang-tidy via `flatpak-spawn --host distrobox`

### Adding New Systems
1. Create system class (CollisionSystem pattern is canonical example)
2. Initialize in `GameWorld::Initialize()`
3. Connect to EventBus if needed (see `GameWorld::SetupEventHandlers()`)
4. Update in `GameWorld::Update()` or relevant Task's `Update()`

## Code Conventions

### Naming and Style
- Classes: `PascalCase` (Tank, GameWorld)
- Methods: `PascalCase` (GetPlayer, IsAlive)
- Variables: `camelCase` (playerIndex, isPlayer)
- Constants: `UPPER_SNAKE_CASE` (MAX_PLAYERS)
- Member variables: No prefix (not `m_` or `_`)
- Enums: `enum class EnumName { VALUE_ONE, VALUE_TWO }`

### File Organization
- Headers: `ClassName.h` with `#pragma once`
- Source: `ClassName.cpp` includes own header first
- Subsystems in folders: `rendering/`, `collision/`, `events/`, `combat/`
- Forward declare when possible to reduce compile dependencies

### Memory Management
- Raw pointers for non-owning references (Tank*, Player*)
- `std::unique_ptr` for owned entities (EntityManager uses this internally)
- Manual `new`/`delete` for singletons (legacy pattern being maintained)
- No smart pointers for OpenGL resources (use RAII wrappers like DisplayList)

### OpenGL and Graphics
- Use immediate mode OpenGL (glBegin/glEnd) - legacy but project standard
- Display lists via `DisplayList` class for cached geometry
- OpenGL code ONLY in `src/rendering/` directory and GraphicsTask
- Textures via `TextureHandler`, fonts via SDL_ttf

### Logging
Use `Logger` singleton:
```cpp
Logger::Get().Write("Message with formatting: %d\n", value);
```
Logs to `applog.txt` in working directory.

### Platform Compatibility
- Windows: `#ifdef _WIN32`
- macOS: `#ifdef __APPLE__` 
- Linux: `#else` (default)
- Use SDL2 for cross-platform window/input/audio

## Common Patterns

### Creating Entities
Always create through GameWorld, not directly:
```cpp
Tank* tank = gameWorld.CreateTank();
Bullet* bullet = gameWorld.CreateBullet(id, attack, ...);
```

### Accessing Game State
- From Tasks: `TankHandler::GetSingleton().GetTanks()` (legacy - transitioning to GameWorld)
- From GameWorld: `gameWorld.GetTanks()` returns `const vector<unique_ptr<Tank>>&`
- Player-specific: `playerManager.GetPlayer(index)` (PlayerManager owned by GameTask)

### Adding Rendering for New Objects
1. Define data struct in `rendering/RenderData.h` (POD only, no methods)
2. Create `ObjectDataExtractor.h/.cpp` to extract data from entity
3. Create `ObjectRenderer.h/.cpp` inheriting `IRenderer`
4. Update `SceneDataBuilder` to include new data extraction
5. Update `RenderingPipeline` to call new renderer in proper order

### Input Handling
- `InputTask` reads SDL events, stores state
- `PlayerManager::HandleInput()` processes input for players
- `InputHandlerFactory` creates appropriate handler (keyboard/mouse vs joystick types)
- Tank control via `InputHandler` interface implementations

## Known Quirks

1. **Tank Health vs Energy:** Tank now has separate `health` (survival) and `energy` (actions like firing)
2. **Level Loading:** Levels use custom `.txt` format (parsed by LevelHandler) with companion `.json` for some data
3. **Display Lists:** Built in `GraphicsTask::BuildDisplayLists()` - expensive, done once at startup
4. **Collision Layers:** Use `CollisionLayer` enum (LAYER_PLAYER, LAYER_ENEMY, LAYER_TERRAIN, etc.) with bitmask operations
5. **Rendering Fallback:** GraphicsTask has both new RenderingPipeline and legacy direct rendering (gradual migration)
6. **Asset Paths:** Relative to `runtime/` directory, hardcoded strings like `"./levels/level0@@.txt"`

## Current Refactoring Status

**Completed (Phases 1-6):**
- ✅ Viewport and camera management extracted
- ✅ Data-driven rendering pipeline
- ✅ All game objects (terrain, tanks, bullets, effects, items, HUD, menu) render via centralized pipeline
- ✅ OpenGL removed from game logic classes

**In Progress:**
- Transitioning entity ownership from Handlers to GameWorld
- Expanding event-driven communication patterns

**Future Work:**
- Complete migration from Handler-based architecture to GameWorld-centric
- Modernize asset loading (remove hardcoded paths)
- Replace immediate-mode OpenGL with modern vertex buffers

## Key Files Reference

- `main.cpp` - Entry point, task instantiation
- `GameTask.cpp` - Game loop, owns PlayerManager and GameWorld
- `GraphicsTask.cpp` - Rendering orchestration (1552 lines, consider reading in chunks)
- `GameWorld.h/.cpp` - Central entity and system coordinator
- `TaskHandler.cpp` - Priority-based execution loop
- `rendering/RenderingPipeline.h` - Rendering architecture overview
- `events/EventBus.h` - Event system template
- `Singleton.h` - Singleton pattern template used throughout

## Testing and Validation

No automated tests currently. Validate changes by:
1. Build with `./build.sh` from `src/`
2. Run from `runtime/` directory
3. Check `applog.txt` for errors/warnings
4. Test gameplay: menu navigation, tank movement, shooting, split-screen
5. Verify no OpenGL errors (check console output)
