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

#include "events/Events.h"
#include "events/CollisionEvents.h"
#include <cmath>

const Gameplay::PlayerRules& Player::GetRules() const
{
    return gameWorld ? gameWorld->GetGameplayDefinitions()->player : Gameplay::BuiltInDefinitions().player;
}

Player::Player(int playerIndex, GameWorld *world)
    : playerIndex(playerIndex), gameWorld(world),
      savedType1(GetRules().startingPrimaryType), savedType2(GetRules().startingSecondaryType)
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

void Player::RememberTankTypes()
{
    if (controlledTank)
    {
        savedType1 = controlledTank->GetPrimaryType();
        savedType2 = controlledTank->GetSecondaryType();
    }
}

void Player::ReleaseTank()
{
    if (controlledTank)
    {
        RememberTankTypes();
        controlledTank->isPlayer = false;
        controlledTank->ConfigureStats({true, 2, 1});

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

    RememberTankTypes();

    // Create tank through GameWorld
    Tank *tank = gameWorld->CreateTank();
    Logger::Get().Write("Player %d: GameWorld::CreateTank returned tank=%p\n", playerIndex, (void *)tank);

    if (tank)
    {
        // Initialize tank for player use
        tank->Init();
        // Restore both the colors and weapon stats from the previous tank.
        tank->SetType(savedType1, savedType2);
        // Init sets the default spawn; apply the requested player/versus spawn last.
        tank->SetPosition(x, y, z);
        Logger::Get().Write("Player %d: Tank::Init() completed\n", playerIndex);

        // Take control
        TakeControlOf(tank);
        // Only a new spawn refills resources; taking control or retuning does not.
        tank->health = tank->GetStats().tank.maxHealth;
        tank->energy = tank->GetStats().weapon.maxEnergy;
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
    float chargeBonus = comboPoints * GetRules().comboChargeFraction;
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
    specialCharge = std::min(specialCharge + charge, GetRules().specialChargeMax);
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
            TANKGAME_LOG_DEBUG("Player::HandleInput() - Player %d SKIP: tank=%s alive=%d inputHandler=%s\n",
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
        TANKGAME_LOG_DEBUG("Player::HandleInput() - Player %d processing input via NEW PlayerManager path (tank id=%d)\n",
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
    // GameWorld owns physics; PlayerManager owns death timers and round changes.
    if (controlledTank && controlledTank->alive)
    {
        CheckItemCollisions();
        HandleInput();
    }
}

void Player::OnTankDestroyed()
{
    if (controlledTank)
    {
        Logger::Get().Write("Player %d tank destroyed\n", playerIndex);
        RememberTankTypes();
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
    float spawnY = GetSpawnHeight();

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

void Player::ApplyDifficultySettings(int value)
{
    difficulty = std::max(0, std::min(value, 2));
    if (controlledTank) controlledTank->ConfigureStats({false, difficulty, 1});
}

void Player::UpdateComboDecay()
{
    if (combo > 0.0f)
    {
        combo -= GetRules().comboDecayPerSecond * GlobalTimer::dT;
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

    const bool matchingType = item->type == controlledTank->GetPrimaryType();
    controlledTank->SetType(item->type, controlledTank->GetPrimaryType());

    // Restore energy based on item type match
    if (matchingType)
    {
        // Full energy for matching type
        controlledTank->energy += controlledTank->GetStats().weapon.maxEnergy * GetRules().matchingPickupEnergyFraction;
    }
    else
    {
        // Half energy for non-matching type
        controlledTank->energy += controlledTank->GetStats().weapon.maxEnergy * GetRules().otherPickupEnergyFraction;
    }

    // Cap energy at maximum
    if (controlledTank->energy > controlledTank->GetStats().weapon.maxEnergy)
    {
        controlledTank->energy = controlledTank->GetStats().weapon.maxEnergy;
    }

    // Add score for item collection
    AddScore(GetRules().pickupScore);
}

void Player::SetupTankForPlayer(Tank *tank)
{
    if (!tank)
        return;

    tank->ConfigureStats({false, difficulty, 1});
    tank->isPlayer = true;
    tank->identity = TankIdentity::Player(playerIndex);

    // Set up input handler ID for tank
    tank->jid = playerIndex; // Joystick ID matches player index
}
