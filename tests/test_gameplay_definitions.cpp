#include <gtest/gtest.h>
#include "gameplay/GameplayDefinitions.h"
#include "Tank.h"
#include "App.h"
#include "GameWorld.h"
#include "LevelHandler.h"
#include "TankHandler.h"
#include "events/Events.h"
#include "events/CollisionEvents.h"
#include "rendering/HUDDataExtractor.h"
#include <limits>
#include <stdexcept>

using namespace Gameplay;

TEST(GameplayDefinitionsTest, AllOrderedPairsHaveExpectedResolvedWeaponValues) {
    const auto definitions = BuildBuiltInDefinitions();
    // Rows are primary, columns secondary: grey, red, blue, yellow, purple.
    const float attacks[5][5] = {
        {200,200,200,200,200}, {50,100,200,120,100},
        {100,200,2200,800,500}, {300,150,400,300,500}, {150,150,500,500,300}
    };
    const float intervals[5][5] = {
        {.5f,.5f,.5f,.5f,.5f}, {.4f,.05f,.1f,.075f,.05f},
        {.5f,.5f,.5f,.5f,.5f}, {.5f,.25f,.8f,.5f,.8f}, {.25f,.25f,.8f,.8f,.5f}
    };
    const float costs[5][5] = {
        {50,50,50,50,50}, {20,10,20,12.5f,10},
        {100/1.5f,100/1.5f,100,100/1.5f,100/1.5f},
        {25,25,50,25,50}, {25,25,50,50,25}
    };
    const int bounces[5][5] = {
        {0,0,0,0,0}, {0,0,0,2,0}, {0,0,0,1,1},
        {400,64,32,400,64}, {64,64,64,64,64}
    };
    for (int p = 0; p < 5; ++p) for (int s = 0; s < 5; ++s) {
        SCOPED_TRACE(p * 5 + s);
        auto config = ResolveTankType(definitions, static_cast<TankType>(p), static_cast<TankType>(s));
        EXPECT_FLOAT_EQ(config.attack, attacks[p][s]);
        EXPECT_FLOAT_EQ(config.fireIntervalSeconds, intervals[p][s]);
        EXPECT_FLOAT_EQ(config.fireCost, costs[p][s]);
        EXPECT_EQ(config.bounces, bounces[p][s]);
        EXPECT_FLOAT_EQ(config.specialChargeCost, 2);
        EXPECT_FLOAT_EQ(config.maxEnergy, 100);
        EXPECT_FLOAT_EQ(config.energyRegen, p == 1 ? 55 : 50);
    }
}

TEST(GameplayDefinitionsTest, MissingOverridesInheritAndExplicitZeroOverridesApply) {
    auto definitions = BuildBuiltInDefinitions();
    auto& combo = definitions.combinations[{TankType::TYPE_PURPLE, TankType::TYPE_GREY}];
    EXPECT_EQ(ResolveTankType(definitions, TankType::TYPE_PURPLE, TankType::TYPE_GREY).bounces, 64);
    combo.bounces = 0;
    combo.fireIntervalSeconds = 0;
    combo.fireCost = 0;
    EXPECT_NO_THROW(ValidateDefinitions(definitions));
    const auto config = ResolveTankType(definitions, TankType::TYPE_PURPLE, TankType::TYPE_GREY);
    EXPECT_EQ(config.bounces, 0);
    EXPECT_FLOAT_EQ(config.fireIntervalSeconds, 0);
    EXPECT_FLOAT_EQ(config.fireCost, 0);
    // Resolving alternate definitions must never modify the built-in provider.
    EXPECT_EQ(TankTypeManager::GetConfig(TankType::TYPE_PURPLE, TankType::TYPE_GREY).bounces, 64);
}

TEST(GameplayDefinitionsTest, InvalidTypesDoNotInsertDefinitions) {
    auto definitions = BuildBuiltInDefinitions();
    const auto size = definitions.types.size();
    EXPECT_THROW(ResolveTankType(definitions, TankType::TANK_TYPE_COUNT, TankType::TYPE_RED), std::invalid_argument);
    EXPECT_THROW(ResolveTankType(definitions, TankType::TYPE_RED, static_cast<TankType>(-1)), std::invalid_argument);
    EXPECT_EQ(definitions.types.size(), size);
    definitions.types.erase(TankType::TYPE_GREY);
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
}

TEST(GameplayDefinitionsTest, ValidationIdentifiesInvalidFieldsAndReferences) {
    auto definitions = BuildBuiltInDefinitions();
    definitions.types.at(TankType::TYPE_RED).maxEnergy = 0;
    try {
        ValidateDefinitions(definitions);
        FAIL() << "Expected invalid capacity";
    } catch (const std::invalid_argument& error) {
        EXPECT_NE(std::string(error.what()).find("maxEnergy"), std::string::npos);
    }
    for (float invalid : {-1.f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()}) {
        definitions = BuildBuiltInDefinitions();
        definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_GREY}].fireCost = invalid;
        EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
    }
    definitions = BuildBuiltInDefinitions();
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TANK_TYPE_COUNT}] = {};
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
    definitions = BuildBuiltInDefinitions();
    definitions.combinations[{TankType::TYPE_BLUE, TankType::TYPE_RED}].bounces = -1;
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
}

TEST(GameplayDefinitionsTest, TypeApplicationUsesResolvedCostsAndPreservesLiveState) {
    Tank tank;
    tank.health = 37;
    tank.energy = 71;
    tank.fireTimer = .3f;
    tank.vy = 4;
    tank.SetType(TankType::TYPE_BLUE, TankType::TYPE_GREY);
    EXPECT_FLOAT_EQ(tank.fireCost, 100/1.5f);
    EXPECT_FLOAT_EQ(tank.fireRate, .5f);
    EXPECT_FLOAT_EQ(tank.specialChargeCost, 2);
    EXPECT_FLOAT_EQ(tank.health, 37);
    EXPECT_FLOAT_EQ(tank.energy, 71);
    EXPECT_FLOAT_EQ(tank.fireTimer, .3f);
    EXPECT_FLOAT_EQ(tank.vy, 4);
}

TEST(GameplayDefinitionsTest, DifficultyAndEnemyModifiersComposeWithoutMutatingDefinitions) {
    const auto definitions = BuildBuiltInDefinitions();
    const float health[] = {300, 150, 100};
    const float healthRegen[] = {20, 15, 10};
    for (int difficulty = 0; difficulty < 3; ++difficulty) {
        const auto config = ResolveTank(definitions, TankType::TYPE_RED, TankType::TYPE_BLUE,
                                        {false, difficulty, 1});
        EXPECT_FLOAT_EQ(config.tank.maxHealth, health[difficulty]);
        EXPECT_FLOAT_EQ(config.tank.healthRegenPerSecond, healthRegen[difficulty]);
        EXPECT_FLOAT_EQ(config.weapon.maxEnergy, difficulty == 2 ? 100 : 150);
        EXPECT_FLOAT_EQ(config.weapon.energyRegen, difficulty == 2 ? 55 : 82.5f);
        const auto enemy = ResolveTank(definitions, TankType::TYPE_BLUE, TankType::TYPE_GREY,
                                       {true, difficulty, 1.0f / 3});
        EXPECT_FLOAT_EQ(enemy.tank.maxHealth, 100);
        EXPECT_FLOAT_EQ(enemy.tank.jumpEnergyPerSecond, 0);
        EXPECT_FLOAT_EQ(enemy.weapon.attack, 100.0f / 3);
    }
    EXPECT_FLOAT_EQ(definitions.tank.maxHealth, 100);
}

TEST(GameplayDefinitionsTest, AlternateSessionDefinitionsReachEntitiesAndSurviveMoves) {
    auto definitions = BuildBuiltInDefinitions();
    definitions.tank.maxHealth = 240;
    definitions.tank.jumpThrust = 23;
    definitions.types[TankType::TYPE_RED].r = .25f;
    GameWorld world(definitions);
    auto* tank = world.CreateTank();
    EXPECT_FLOAT_EQ(tank->maxHealth, 240);
    EXPECT_FLOAT_EQ(tank->health, 240);
    EXPECT_FLOAT_EQ(tank->jumpRate, 23);
    EXPECT_FLOAT_EQ(tank->GetPrimaryColor().r, .25f);
    tank->ConfigureStats({false, 0, 1});
    Tank moved(std::move(*tank));
    moved.SetType(TankType::TYPE_BLUE, TankType::TYPE_RED);
    EXPECT_FLOAT_EQ(moved.maxHealth, 720);
    Tank assigned;
    assigned = std::move(moved);
    assigned.SetType(TankType::TYPE_RED, TankType::TYPE_BLUE);
    EXPECT_FLOAT_EQ(assigned.maxHealth, 720);
    EXPECT_FLOAT_EQ(assigned.jumpRate, 23);
    Tank standalone;
    EXPECT_FLOAT_EQ(standalone.maxHealth, 100);
}

TEST(GameplayDefinitionsTest, InvalidCommonOrComposedValuesRejectSessionCreation) {
    auto definitions = BuildBuiltInDefinitions();
    definitions.tank.jumpThrust = -1;
    EXPECT_THROW(GameWorld world(definitions), std::invalid_argument);
    definitions = BuildBuiltInDefinitions();
    definitions.difficulties[0].energy = std::numeric_limits<float>::max();
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
    definitions = BuildBuiltInDefinitions();
    EXPECT_THROW(ResolveTank(definitions, TankType::TYPE_RED, TankType::TYPE_BLUE,
                             (TankContext{false, 3, 1})), std::invalid_argument);
}

class GameplayLifecycleTest : public ::testing::Test {
protected:
    App app;
    GameTask task;
    SoundTask sound;
    GameWorld world;
    void SetUp() override {
        Events::GetBus().Clear();
        app.gameTask = &task;
        app.soundTask = &sound;
        sound.disable = true;
        LevelHandler::Create();
        LevelHandler::GetSingleton().Load(nullptr);
        task.GetPlayerManager()->Initialize(&world);
    }
    void TearDown() override {
        Events::GetBus().Clear();
        task.GetPlayerManager()->Shutdown();
        world.Clear();
        LevelHandler::Destroy();
    }
};

TEST_F(GameplayLifecycleTest, DifficultySurvivesSpawnTypeChangesAndRespawnWithoutStacking) {
    auto* manager = task.GetPlayerManager();
    manager->SetDifficulty(0); // No tank exists yet.
    auto* player = manager->GetPlayer(0);
    auto* tank = player->CreatePlayerTank(30, 2, 30);
    ASSERT_NE(tank, nullptr);
    EXPECT_FLOAT_EQ(tank->health, 300);
    EXPECT_FLOAT_EQ(tank->energy, 150);
    tank->health = 73;
    tank->energy = 42;
    tank->vy = 3;
    tank->fireTimer = .23f;
    manager->SetDifficulty(0);
    manager->SetDifficulty(0);
    tank->SetType(TankType::TYPE_BLUE, TankType::TYPE_RED);
    EXPECT_FLOAT_EQ(tank->maxHealth, 300);
    EXPECT_FLOAT_EQ(tank->health, 73);
    EXPECT_FLOAT_EQ(tank->maxEnergy, 150);
    EXPECT_FLOAT_EQ(tank->energy, 42);
    EXPECT_FLOAT_EQ(tank->energyRegen, 75);
    EXPECT_FLOAT_EQ(tank->vy, 3);
    EXPECT_FLOAT_EQ(tank->fireTimer, .23f);
    manager->SetDifficulty(2);
    EXPECT_FLOAT_EQ(tank->maxHealth, 100);
    EXPECT_FLOAT_EQ(tank->health, 73);
    EXPECT_FLOAT_EQ(tank->maxEnergy, 100);
    EXPECT_FLOAT_EQ(tank->healthRegen, 10);
    manager->SetDifficulty(1);
    tank->alive = false;
    player->Respawn();
    tank = player->GetControlledTank();
    EXPECT_FLOAT_EQ(tank->health, 150);
    EXPECT_FLOAT_EQ(tank->energy, 150);
    EXPECT_EQ(tank->type1, TankType::TYPE_BLUE);
    EXPECT_EQ(tank->type2, TankType::TYPE_RED);
}

TEST_F(GameplayLifecycleTest, DifficultyReductionClampsResourcesWithoutHealing) {
    auto* manager = task.GetPlayerManager();
    manager->SetDifficulty(0);
    auto* tank = manager->GetPlayer(0)->CreatePlayerTank(30, 2, 30);
    manager->SetDifficulty(2);
    EXPECT_FLOAT_EQ(tank->health, 100);
    EXPECT_FLOAT_EQ(tank->energy, 100);
    tank->health = -5;
    tank->energy = 9;
    manager->SetDifficulty(0);
    EXPECT_FLOAT_EQ(tank->health, -5); // Retuning cannot resurrect a tank.
    EXPECT_FLOAT_EQ(tank->energy, 9);
}

TEST_F(GameplayLifecycleTest, PickupsUsePreviousTypeAndKeepDifficultyBonuses) {
    auto* manager = task.GetPlayerManager();
    manager->SetDifficulty(0);
    auto* player = manager->GetPlayer(0);
    auto* tank = player->CreatePlayerTank(30, 2, 30);
    tank->health = 85;
    tank->energy = 0;
    auto* mismatch = world.CreateItem(30, 2.1f, 30, TankType::TYPE_BLUE);
    player->CheckItemCollisions();
    EXPECT_FALSE(mismatch->IsAlive());
    EXPECT_FLOAT_EQ(tank->energy, 75);
    EXPECT_FLOAT_EQ(tank->maxEnergy, 150);
    EXPECT_FLOAT_EQ(tank->maxHealth, 300);
    EXPECT_FLOAT_EQ(tank->health, 85);
    EXPECT_EQ(tank->type2, TankType::TYPE_RED);
    tank->energy = 0;
    auto* match = world.CreateItem(30, 2.1f, 30, TankType::TYPE_BLUE);
    LevelHandler::GetSingleton().SetGameWorld(&world);
    LevelHandler::GetSingleton().ItemCollision();
    EXPECT_FALSE(match->IsAlive());
    EXPECT_FLOAT_EQ(tank->energy, 150);
    EXPECT_EQ(player->GetScore(), 200);
}

TEST_F(GameplayLifecycleTest, InitUsesSameDefaultsAndResetsTransientPhysics) {
    Tank tank;
    EXPECT_FLOAT_EQ(tank.healthRegen, 10);
    EXPECT_FLOAT_EQ(tank.jumpRate, 18);
    tank.vy = 7;
    tank.jumpTime = .4f;
    tank.fireTimer = .5f;
    tank.Init();
    EXPECT_FLOAT_EQ(tank.healthRegen, 10);
    EXPECT_FLOAT_EQ(tank.jumpRate, 18);
    EXPECT_FLOAT_EQ(tank.jumpCost, 250);
    EXPECT_FLOAT_EQ(tank.fallRate, 16);
    EXPECT_FLOAT_EQ(tank.fireCost, 20);
    EXPECT_FLOAT_EQ(tank.vy, 0);
    EXPECT_FLOAT_EQ(tank.jumpTime, 0);
    EXPECT_FLOAT_EQ(tank.fireTimer, 0);
    tank.ConfigureStats({true, 0, 1});
    tank.SetType(TankType::TYPE_YELLOW, TankType::TYPE_GREY);
    EXPECT_FLOAT_EQ(tank.jumpCost, 0);
    EXPECT_FLOAT_EQ(tank.maxHealth, 100);
}

class GameplayRulesTest : public GameplayLifecycleTest {
protected:
    std::unique_ptr<GameWorld> configuredWorld;
    float savedDelta = 0;
    void SetUp() override {
        GameplayLifecycleTest::SetUp();
        savedDelta = GlobalTimer::dT;
        GlobalTimer::dT = .1f;
        auto definitions = BuildBuiltInDefinitions();
        definitions.projectile.speed = 12;
        definitions.projectile.lifetimeSeconds = .25f;
        definitions.projectile.selfHitDelaySeconds = .1f;
        definitions.projectile.purpleMinPower = 40;
        definitions.projectile.purpleMaxPower = 600;
        definitions.projectile.purplePrimaryTurnScale = 40;
        definitions.projectile.purpleSpecialTurnAcceleration = 100;
        definitions.projectile.purpleSecondaryPowerScale = .25f;
        definitions.projectile.purpleSecondaryChargeDivisor = 4;
        definitions.projectile.firstBouncePowerMultiplier = 1.25f;
        definitions.projectile.yellowRedChildCount = 3;
        definitions.player.spawnHeight = 20;
        definitions.player.respawnDelaySeconds = .2f;
        definitions.player.versusRespawnDelaySeconds = .3f;
        definitions.player.comboChargeFraction = .25f;
        definitions.player.comboDecayPerSecond = 2;
        definitions.player.specialChargeMax = 40;
        definitions.player.pickupScore = 250;
        definitions.player.otherPickupEnergyFraction = .25f;
        definitions.combat.healthReturnDivisor = 4;
        definitions.combat.energyReturnDivisor = 5;
        definitions.combat.blueSpecialHitPowerBonus = 17;
        definitions.combat.hitComboDivisor = 5;
        definitions.tank.turboEnergyPerSecond = 70;
        definitions.tank.turretInputScale = 5;
        definitions.tank.turretMaxStepDegrees = 1;
        definitions.tank.maxForwardInput = .5f;
        task.GetPlayerManager()->Shutdown();
        configuredWorld.reset(new GameWorld(definitions));
        configuredWorld->Initialize();
        TankHandler::Create();
        TankHandler::GetSingleton().SetGameWorld(configuredWorld.get());
        task.GetPlayerManager()->Initialize(configuredWorld.get());
        task.GetPlayerManager()->SetNumPlayers(2);
    }
    void TearDown() override {
        task.GetPlayerManager()->Shutdown();
        configuredWorld->Clear();
        TankHandler::Destroy();
        configuredWorld.reset();
        GlobalTimer::dT = savedDelta;
        GameplayLifecycleTest::TearDown();
    }
    Bullet* Shot(TankType primary, TankType secondary, bool special = false, float pressed = 1) {
        return configuredWorld->CreateBullet(TankIdentity::Player(0), 200, primary, secondary,
            4, pressed, Color(1,0,0), Color(0,0,1), 30, 2, 30, 0, 0, 0, special);
    }
    void Hit(Bullet* bullet, Tank* target) {
        Events::GetBus().Publish(BulletCollisionEvent(bullet, target, 30, 2, 30));
    }
};

TEST_F(GameplayRulesTest, ProjectileMotionLifetimeAndCopiesUseSessionRules) {
    auto* bullet = Shot(TankType::TYPE_GREY, TankType::TYPE_GREY);
    EXPECT_FLOAT_EQ(bullet->GetMoveRate(), 12);
    bullet->Update();
    EXPECT_NEAR(bullet->GetX(), 31.2f, .0001f);
    Events::ProcessQueuedEvents();
    EXPECT_TRUE(bullet->IsAlive());
    bullet->Update();
    Events::ProcessQueuedEvents();
    EXPECT_TRUE(bullet->IsAlive());
    bullet->Update();
    Events::ProcessQueuedEvents();
    EXPECT_FALSE(bullet->IsAlive());

    auto* purple = Shot(TankType::TYPE_PURPLE, TankType::TYPE_GREY, false, 2);
    EXPECT_FLOAT_EQ(purple->GetPower(), 400);
    auto* copy = configuredWorld->CreateBullet(*purple);
    EXPECT_FLOAT_EQ(copy->GetPower(), 400); // No second charge multiplication.
    EXPECT_FLOAT_EQ(copy->GetMoveRate(), 12);
    GameWorld other;
    auto* crossWorldCopy = other.CreateBullet(*purple);
    EXPECT_FLOAT_EQ(crossWorldCopy->GetMoveRate(), 12);
    EXPECT_FLOAT_EQ(crossWorldCopy->GetGameplayDefinitions().projectile.purpleMaxPower, 600);
}

TEST_F(GameplayRulesTest, PurpleChargingAndYellowChildrenUseConfiguredParameters) {
    EXPECT_FLOAT_EQ(Shot(TankType::TYPE_PURPLE, TankType::TYPE_GREY, false, 0)->GetPower(), 40);
    EXPECT_FLOAT_EQ(Shot(TankType::TYPE_PURPLE, TankType::TYPE_GREY, false, 9)->GetPower(), 600);
    EXPECT_FLOAT_EQ(Shot(TankType::TYPE_RED, TankType::TYPE_PURPLE, false, 2)->GetPower(), 75);
    auto* turning = Shot(TankType::TYPE_PURPLE, TankType::TYPE_GREY, true, 1);
    turning->Update();
    EXPECT_NEAR(turning->GetRY(), 5, .0001f); // (40 + 100 * .1) * .1
    auto* yellow = Shot(TankType::TYPE_YELLOW, TankType::TYPE_RED, true);
    const auto before = configuredWorld->GetBullets().size();
    yellow->HandleLevelCollision(1, 0, 0);
    ASSERT_EQ(configuredWorld->GetBullets().size(), before + 3);
    for (size_t i = 0; i < 3; ++i) {
        const auto* child = configuredWorld->GetBullets()[before + i].get();
        EXPECT_FLOAT_EQ(child->GetMoveRate(), 12);
        EXPECT_FLOAT_EQ(child->GetPower(), 250);
        EXPECT_FLOAT_EQ(child->GetRY(), i * 120);
    }
}

TEST_F(GameplayRulesTest, CombatHealingPowerBonusesAndComboUseSessionRules) {
    auto* player = task.GetPlayerManager()->GetPlayer(1);
    auto* target = player->CreatePlayerTank(45, 2, 45);
    target->health = 20;
    Hit(Shot(TankType::TYPE_GREY, TankType::TYPE_GREY), target);
    EXPECT_FLOAT_EQ(target->health, 70); // 200 / 4 healing.
    target->health = target->maxHealth * 2;
    target->energy = 0;
    Hit(Shot(TankType::TYPE_GREY, TankType::TYPE_GREY), target);
    EXPECT_FLOAT_EQ(target->energy, 40); // 200 / 5 energy return.

    Tank enemy;
    enemy.identity = TankIdentity::Enemy(10);
    enemy.health = 1000;
    auto* blue = Shot(TankType::TYPE_BLUE, TankType::TYPE_GREY, true);
    Hit(blue, &enemy);
    EXPECT_FLOAT_EQ(enemy.health, 800);
    EXPECT_FLOAT_EQ(blue->GetPower(), 217);
    auto* shooter = task.GetPlayerManager()->GetPlayer(0);
    EXPECT_FLOAT_EQ(shooter->GetCombo(), .2f);
    EXPECT_FLOAT_EQ(shooter->GetSpecialCharge(), .05f);
}

TEST_F(GameplayRulesTest, SelfHitGracePeriodUsesSameRuleInCombat) {
    auto* player = task.GetPlayerManager()->GetPlayer(0);
    auto* target = player->CreatePlayerTank(45, 2, 45);
    target->health = 20;
    auto* early = Shot(TankType::TYPE_GREY, TankType::TYPE_GREY);
    early->Update();
    Hit(early, target);
    EXPECT_FLOAT_EQ(target->health, 20); // Strictly greater than the grace period.
    auto* late = Shot(TankType::TYPE_GREY, TankType::TYPE_GREY);
    late->Update();
    late->Update();
    Hit(late, target);
    EXPECT_FLOAT_EQ(target->health, 70);
}

TEST_F(GameplayRulesTest, PlayerRewardsSpawnAndHudUseSessionRules) {
    auto* player = task.GetPlayerManager()->GetPlayer(0);
    player->Respawn();
    auto* tank = player->GetControlledTank();
    ASSERT_NE(tank, nullptr);
    EXPECT_FLOAT_EQ(tank->y, 20);
    player->AddCombo(10, 1);
    EXPECT_FLOAT_EQ(player->GetSpecialCharge(), 2.5f);
    player->UpdateComboDecay();
    EXPECT_NEAR(player->GetCombo(), 9.8f, .0001f);
    player->AddSpecialCharge(100);
    EXPECT_FLOAT_EQ(player->GetSpecialCharge(), 40);
    tank->SetPosition(30, 2, 30);
    tank->energy = 0;
    configuredWorld->CreateItem(30, 2.1f, 30, TankType::TYPE_BLUE);
    player->CheckItemCollisions();
    EXPECT_FLOAT_EQ(tank->energy, tank->maxEnergy * .25f);
    EXPECT_EQ(player->GetScore(), 250);
    const auto hud = HUDDataExtractor::ExtractPlayerHUD(*tank, 0);
    EXPECT_FLOAT_EQ(hud.maxSpecialCharge, 40);
}

TEST_F(GameplayRulesTest, MovementRulesAreIndependentOfJumpCost) {
    Tank* tank = configuredWorld->CreateTank();
    tank->SetPosition(30, 2, 30);
    EXPECT_FLOAT_EQ(tank->GetTurboEnergyPerSecond(), 70);
    EXPECT_FLOAT_EQ(tank->jumpCost, 250);
    tank->RotTurret(100);
    EXPECT_FLOAT_EQ(tank->rty, 1);
    tank->Move(1.0f);
    EXPECT_NEAR(tank->x, 30.25f, .0001f);
    tank->ConfigureStats({true, 2, 1});
    EXPECT_FLOAT_EQ(tank->GetTurboEnergyPerSecond(), 0);
}

TEST_F(GameplayRulesTest, RespawnSchedulingUsesConfiguredModeDelays) {
    auto* manager = task.GetPlayerManager();
    manager->SetNumPlayers(1);
    auto& level = LevelHandler::GetSingleton();
    level.SetGameWorld(configuredWorld.get());
    level.SetPlayerManager(manager);
    for (bool versus : {false, true}) {
        ASSERT_TRUE(level.Load("levels/level0@@.txt"));
        manager->SetVersusMode(versus);
        auto* player = manager->GetPlayer(0);
        auto* tank = player->CreatePlayerTank(30, 2, 30);
        tank->alive = false;
        const float delay = versus ? .3f : .2f;
        tank->deadtime = delay - .05f;
        GlobalTimer::dT = .025f;
        manager->NextFrame();
        level.ProcessPendingLevelChange();
        EXPECT_FALSE(player->GetControlledTank()->alive);
        GlobalTimer::dT = .1f;
        manager->NextFrame();
        level.ProcessPendingLevelChange();
        ASSERT_NE(player->GetControlledTank(), nullptr);
        EXPECT_TRUE(player->GetControlledTank()->alive);
        EXPECT_FLOAT_EQ(player->GetControlledTank()->y, 20);
    }
}

TEST(GameplayDefinitionsTest, InvalidRuleRangesRejectConfiguration) {
    auto definitions = BuildBuiltInDefinitions();
    definitions.projectile.lifetimeSeconds = 0;
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
    definitions = BuildBuiltInDefinitions();
    definitions.projectile.purpleMinPower = 1001;
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
    definitions = BuildBuiltInDefinitions();
    definitions.combat.healthReturnDivisor = 0;
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
    definitions = BuildBuiltInDefinitions();
    definitions.projectile.yellowRedChildCount = 1025;
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
    definitions = BuildBuiltInDefinitions();
    definitions.player.specialChargeMax = std::numeric_limits<float>::infinity();
    EXPECT_THROW(ValidateDefinitions(definitions), std::invalid_argument);
}
