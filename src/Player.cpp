#include "Player.h"
#include "GameWorld.h"
#include "Item.h"
#include "LevelHandler.h"
#include "TankHandler.h" // For respawn delay constants
#include "Logger.h"
#include "InputHandlerFactory.h"
#include "GlobalTimer.h"
#include "App.h"
#include "FX.h"        // For FxType enum
#include "SoundTask.h" // For sound playback

// Static constexpr definitions (required in C++14 when ODR-used by std::min/max)
constexpr float Player::SPECIAL_CHARGE_MAX;
#include "events/Events.h"
#include "events/CollisionEvents.h"
#include <cmath>

Player::Player(int playerIndex, GameWorld *world)
    : playerIndex(playerIndex), gameWorld(world)
{
    Logger::Get().Write("Created Player %d\n", playerIndex);
}

void Player::TakeControlOf(Tank *tank)
{
    // Release control of current tank if any
    if (controlledTank && controlledTank != tank)
    {
        ReleaseTank();
    }

    controlledTank = tank;

    if (controlledTank)
    {
        SetupTankForPlayer(controlledTank);

        // Update collision layer through GameWorld
        gameWorld->ReregisterTankCollision(controlledTank);

        Logger::Get().Write("Player %d took control of tank\n", playerIndex);
    }
}

void Player::ReleaseTank()
{
    if (controlledTank)
    {
        controlledTank->isPlayer = false;

        // Update collision layer to enemy tanks
        gameWorld->ReregisterTankCollision(controlledTank);

        Logger::Get().Write("Player %d released tank control\n", playerIndex);
        controlledTank = nullptr;
    }
}

Tank *Player::CreatePlayerTank(float x, float y, float z)
{
    Logger::Get().Write(">>> CreatePlayerTank called for player %d at (%.2f, %.2f, %.2f)\n", playerIndex, x, y, z);

    if (!gameWorld)
    {
        Logger::Get().Write("ERROR: Player %d CreatePlayerTank - gameWorld is null!\n", playerIndex);
        return nullptr;
    }

    // Create tank through GameWorld
    Tank *tank = gameWorld->CreateTank();
    Logger::Get().Write("Player %d: GameWorld::CreateTank returned tank=%p\n", playerIndex, (void *)tank);

    if (tank)
    {
        // Set position
        tank->SetPosition(x, y, z);
        Logger::Get().Write("Player %d: Tank position set to (%.2f, %.2f, %.2f)\n", playerIndex, tank->x, tank->y, tank->z);

        // Initialize tank for player use
        tank->Init();
        Logger::Get().Write("Player %d: Tank::Init() completed\n", playerIndex);

        // Take control
        TakeControlOf(tank);
        Logger::Get().Write("Player %d: TakeControlOf completed - identity set to %s\n",
                            playerIndex, tank->identity.IsPlayer() ? "PLAYER" : "ENEMY");
    }
    else
    {
        Logger::Get().Write("ERROR: Player %d CreatePlayerTank - GameWorld::CreateTank returned null!\n", playerIndex);
    }

    return tank;
}

void Player::AddCombo(float comboPoints, int comboNum)
{
    combo += comboPoints;
    comboNumber = comboNum;

    // Add special charge based on combo
    float chargeBonus = comboPoints * 0.1f; // 10% of combo as special charge
    AddSpecialCharge(chargeBonus);
}

void Player::ResetCombo()
{
    combo = 0.0f;
    comboNumber = 0;
    hitCombo = 0;
}

void Player::AddSpecialCharge(float charge)
{
    specialCharge = std::min(specialCharge + charge, SPECIAL_CHARGE_MAX);
}

void Player::UseSpecialCharge(float cost)
{
    specialCharge = std::max(0.0f, specialCharge - cost);
}

void Player::HandleInput()
{
    static int inputLogCounter = 0;

    if (!controlledTank || !controlledTank->alive || !inputHandler)
    {
        if (inputLogCounter % 180 == 0)
        {
            Logger::Get().Write("Player::HandleInput() - Player %d SKIP: tank=%s alive=%d inputHandler=%s\n",
                                playerIndex,
                                (controlledTank ? "valid" : "null"),
                                (controlledTank ? controlledTank->alive : false),
                                (inputHandler ? "valid" : "null"));
        }
        inputLogCounter++;
        return;
    }

    if (inputLogCounter % 180 == 0)
    { // Log every 3 seconds
        Logger::Get().Write("Player::HandleInput() - Player %d processing input via NEW PlayerManager path (tank id=%d)\n",
                            playerIndex, controlledTank->identity.GetLegacyId());
    }

    // Delegate input handling to the input handler
    inputHandler->HandleInput(*controlledTank);

    inputLogCounter++;
}

void Player::SetInputHandler(std::unique_ptr<InputHandler> handler)
{
    inputHandler = std::move(handler);
}

void Player::CheckItemCollisions()
{
    if (!controlledTank || !controlledTank->alive || !gameWorld)
    {
        return;
    }

    // Get all items from GameWorld
    const auto &items = gameWorld->GetItems();

    for (const auto &itemPtr : items)
    {
        if (!itemPtr->IsAlive())
            continue;

        const Item &item = *itemPtr;

        // Check collision with controlled tank
        if (controlledTank->PointCollision(item.x, item.y, item.z))
        {
            ApplyItemEffect(&item);

            // Create pickup effect
            Events::GetBus().Post(CreateFXEvent(
                static_cast<int>(FxType::TYPE_THREE),
                item.x, item.y, item.z,
                90, 0, 90,
                item.color.r, item.color.g, item.color.b, 1.0f));

            // Play pickup sound
            if (App::GetSingleton().soundTask)
            {
                App::GetSingleton().soundTask->PlayChannel(3);
            }

            // Remove item
            itemPtr->Kill();

            Logger::Get().Write("Player %d collected item\n", playerIndex);
            break; // Only collect one item per frame
        }
    }
}

void Player::Update()
{
    static int logCounter = 0;

    // Update combo decay
    UpdateComboDecay();

    // Check for item collisions
    CheckItemCollisions();

    // Handle input and physics if we have a tank
    if (controlledTank && controlledTank->alive)
    {
        if (logCounter % 120 == 0)
        { // Log every 2 seconds
            Logger::Get().Write("Player %d processing NextFrame + input for tank\n", playerIndex);
        }

        // Physics and game state update (was done by TankHandler::UpdatePlayerStates)
        controlledTank->NextFrame();

        // Input processing
        HandleInput();

        // Reset dead time (was done by TankHandler::UpdatePlayerStates)
        controlledTank->deadtime = 0.0f;
    }

    // Handle respawn if tank is dead
    if (!controlledTank || !controlledTank->alive)
    {
        if (controlledTank && !controlledTank->alive)
        {
            // Handle death logic (was in TankHandler::UpdatePlayerStates)
            controlledTank->Die();

            // Check respawn timing
            bool shouldRespawn = false;
            if (App::GetSingleton().gameTask->IsVersusMode())
            {
                if (controlledTank->deadtime > VERSUS_RESPAWN_DELAY)
                {
                    Logger::Get().Write("===== Player %d: Respawn timer expired in versus mode (deadtime=%.2f) - calling NextLevel(true) =====\n", playerIndex, controlledTank->deadtime);
                    shouldRespawn = true;
                    ReleaseTank(); // Release before NextLevel to avoid dangling pointer
                    Logger::Get().Write("Player %d: Tank released, controlledTank is now null\n", playerIndex);
                    LevelHandler::GetSingleton().NextLevel(true);
                    Logger::Get().Write("Player %d: NextLevel(true) completed\n", playerIndex);
                }
            }
            else
            {
                if (controlledTank->deadtime > PLAYER_RESPAWN_DELAY)
                {
                    Logger::Get().Write("===== Player %d: Respawn timer expired (deadtime=%.2f) - calling NextLevel(false) =====\n", playerIndex, controlledTank->deadtime);
                    shouldRespawn = true;
                    ReleaseTank(); // Release before NextLevel to avoid dangling pointer
                    Logger::Get().Write("Player %d: Tank released, controlledTank is now null\n", playerIndex);
                    LevelHandler::GetSingleton().NextLevel(false);
                    Logger::Get().Write("Player %d: NextLevel(false) completed\n", playerIndex);
                }
            }

            // Only increment deadtime and log if we didn't respawn (tank still exists)
            if (!shouldRespawn && controlledTank)
            {
                controlledTank->deadtime += GlobalTimer::dT;

                if (logCounter % 60 == 0)
                { // Log every 1 second
                    Logger::Get().Write("Player %d tank dead - deadtime: %.2f\n", playerIndex, controlledTank->deadtime);
                }
            }
        }
        else
        {
            // No tank at all - respawn immediately
            Logger::Get().Write("***** Player %d has no tank - calling Respawn() *****\n", playerIndex);
            Respawn();
            if (controlledTank)
            {
                Logger::Get().Write("Player %d: Respawn() completed - new tank created at (%.2f, %.2f, %.2f), identity=%s\n",
                                    playerIndex, controlledTank->x, controlledTank->y, controlledTank->z,
                                    controlledTank->identity.IsPlayer() ? "PLAYER" : "ENEMY");
            }
            else
            {
                Logger::Get().Write("Player %d: WARNING - Respawn() completed but controlledTank is still null!\n", playerIndex);
            }
        }
    }

    logCounter++;
}

void Player::OnTankDestroyed()
{
    if (controlledTank)
    {
        Logger::Get().Write("Player %d tank destroyed\n", playerIndex);
        controlledTank = nullptr;

        // TODO: Start respawn timer
    }
}

void Player::Respawn()
{
    Logger::Get().Write(">>> Player::Respawn() called for player %d\n", playerIndex);

    if (controlledTank && controlledTank->alive)
    {
        Logger::Get().Write("Player %d already has living tank - skipping respawn\n", playerIndex);
        return; // Already have a living tank
    }

    // Get spawn position from LevelHandler
    float spawnX = LevelHandler::GetSingleton().start[0];
    float spawnZ = LevelHandler::GetSingleton().start[1];
    float spawnY = LevelHandler::GetSingleton().GetTerrainHeight(static_cast<int>(spawnX), static_cast<int>(spawnZ));

    Logger::Get().Write("Player %d spawning at position (%.2f, %.2f, %.2f)\n", playerIndex, spawnX, spawnY, spawnZ);

    // Create new tank
    Tank *newTank = CreatePlayerTank(spawnX, spawnY, spawnZ);

    if (newTank)
    {
        Logger::Get().Write("Player %d respawn SUCCESS - tank created with identity=%s, isPlayer=%d\n",
                            playerIndex, newTank->identity.IsPlayer() ? "PLAYER" : "ENEMY", newTank->isPlayer);
    }
    else
    {
        Logger::Get().Write("Player %d respawn FAILED - CreatePlayerTank returned null!\n", playerIndex);
    }
}

void Player::ApplyDifficultySettings(int difficulty)
{
    if (!controlledTank)
        return;

    // Apply difficulty multipliers based on TankHandler constants
    switch (difficulty)
    {
    case 0: // Easy
        controlledTank->maxHealth *= 3.0f;
        controlledTank->health = controlledTank->maxHealth;
        controlledTank->healthRegen *= 2.0f;
        controlledTank->maxEnergy *= 1.5f;
        controlledTank->energy = controlledTank->maxEnergy;
        controlledTank->energyRegen *= 1.5f;
        break;

    case 1: // Normal
        controlledTank->maxHealth *= 1.5f;
        controlledTank->health = controlledTank->maxHealth;
        controlledTank->healthRegen *= 1.5f;
        controlledTank->maxEnergy *= 1.5f;
        controlledTank->energy = controlledTank->maxEnergy;
        controlledTank->energyRegen *= 1.5f;
        break;

    default: // Hard - no modifications
        break;
    }
}

void Player::UpdateComboDecay()
{
    if (combo > 0.0f)
    {
        combo -= COMBO_DECAY_RATE * GlobalTimer::dT;
        if (combo <= 0.0f)
        {
            ResetCombo();
        }
    }
}

void Player::ApplyItemEffect(const Item *item)
{
    if (!controlledTank || !item)
        return;

    // Set tank type based on item
    controlledTank->SetType(item->type, controlledTank->type1);

    // Restore energy based on item type match
    if (item->type == controlledTank->type1)
    {
        // Full energy for matching type
        controlledTank->energy += controlledTank->maxEnergy;
    }
    else
    {
        // Half energy for non-matching type
        controlledTank->energy += controlledTank->maxEnergy / 2;
    }

    // Cap energy at maximum
    if (controlledTank->energy > controlledTank->maxEnergy)
    {
        controlledTank->energy = controlledTank->maxEnergy;
    }

    // Add score for item collection
    AddScore(100); // Base score for item pickup
}

void Player::SetupTankForPlayer(Tank *tank)
{
    if (!tank)
        return;

    tank->isPlayer = true;
    tank->identity = TankIdentity::Player(playerIndex);

    // Set up input handler ID for tank
    tank->jid = playerIndex; // Joystick ID matches player index
}