#include <gtest/gtest.h>
#include <cmath>
#include "App.h"
#include "Bullet.h"
#include "LevelHandler.h"
#include "events/Events.h"

class BulletSpecialTest : public ::testing::Test {
protected:
    App app;
    GameTask task;
    SoundTask sound;
    GameWorld world;
    float oldDT = GlobalTimer::dT;

    void SetUp() override {
        Events::GetBus().Clear();
        app.gameTask = &task;
        app.soundTask = &sound;
        sound.disable = true;
        GlobalTimer::dT = 0.01f;
        LevelHandler::Create();
        // Load(nullptr) initializes floating blocks as well as terrain without assets.
        LevelHandler::GetSingleton().Load(nullptr);
        LevelHandler::GetSingleton().Flatten(0);
        world.Initialize();
        task.GetPlayerManager()->Initialize(&world);
        task.GetPlayerManager()->SetNumPlayers(2);
    }
    void TearDown() override {
        Events::GetBus().Clear();
        task.GetPlayerManager()->Shutdown();
        world.Clear();
        world.Shutdown();
        LevelHandler::Destroy();
        GlobalTimer::dT = oldDT;
    }
    Bullet* Shot(TankType primary, TankType secondary, bool special = true,
                 int player = 0, int bounces = 4, float pressed = 1) {
        return world.CreateBullet(player < 0 ? TankIdentity::Enemy(5) : TankIdentity::Player(player),
            200, primary, secondary, bounces, pressed, Color(1, 1, 0), Color(0.2f, 0.3f, 0.4f),
            30, 2, 30, 0, 30, 0, special);
    }
    void Hit(Bullet* bullet, Tank* target) {
        Events::GetBus().Publish(BulletCollisionEvent(bullet, target, 30, 2, 30));
    }
};

TEST_F(BulletSpecialTest, FiringPatternsChargeAndCooldown) {
    Tank* tanks[2] = {world.CreateTank(), world.CreateTank()};
    for (int p = 0; p < 2; ++p) {
        tanks[p]->SetGameWorld(&world);
        tanks[p]->alive = true;
        tanks[p]->x = tanks[p]->z = 30;
        tanks[p]->y = 2;
        task.GetPlayerManager()->GetPlayer(p)->TakeControlOf(tanks[p]);
    }
    for (int p = 0; p < 2; ++p) {
        auto* player = task.GetPlayerManager()->GetPlayer(p);
        Tank& tank = *tanks[p];
        for (auto primary : {TankType::TYPE_RED, TankType::TYPE_BLUE, TankType::TYPE_YELLOW, TankType::TYPE_PURPLE}) {
            for (auto secondary : {TankType::TYPE_GREY, TankType::TYPE_RED, TankType::TYPE_BLUE, TankType::TYPE_YELLOW, TankType::TYPE_PURPLE}) {
                SCOPED_TRACE(static_cast<int>(primary));
                SCOPED_TRACE(static_cast<int>(secondary));
                tank.SetType(primary, secondary);
                tank.energy = 1000;
                tank.fireTimer = tank.fireRate + 1;
                size_t start = world.GetBullets().size();
                tank.Fire(1);
                ASSERT_EQ(world.GetBullets().size(), start + 1);
                EXPECT_FALSE(world.GetBullets().back()->GetIsSpecial());
                EXPECT_FLOAT_EQ(tank.energy, 1000 - tank.fireCost);
                tank.fireTimer = tank.fireRate + 1;
                player->SetSpecialCharge(100);
                start = world.GetBullets().size();
                tank.Special(1);
                size_t count = primary == TankType::TYPE_RED ? 13 : primary == TankType::TYPE_PURPLE ? 4 :
                    primary == TankType::TYPE_BLUE ? (secondary == TankType::TYPE_RED ? 1 : 2) :
                    (secondary == TankType::TYPE_YELLOW ? 1 : 2);
                ASSERT_EQ(world.GetBullets().size(), start + count);
                for (size_t i = start; i < start + count; ++i) {
                    EXPECT_TRUE(world.GetBullets()[i]->GetIsSpecial());
                    EXPECT_EQ(world.GetBullets()[i]->GetOwnerIdentity(), TankIdentity::Player(p));
                }
                EXPECT_FLOAT_EQ(player->GetSpecialCharge(), 100 - tank.fireCost / 5);
                EXPECT_FLOAT_EQ(tank.fireTimer, 0);
                tank.Special(1);
                tank.Fire(1);
                EXPECT_EQ(world.GetBullets().size(), start + count);
                tank.fireTimer = tank.fireRate + 1;
                player->SetSpecialCharge(0);
                tank.Special(1);
                EXPECT_EQ(world.GetBullets().size(), start + count);
            }
        }
    }
    for (int p = 0; p < 2; ++p) task.GetPlayerManager()->GetPlayer(p)->ReleaseTank();
}

TEST_F(BulletSpecialTest, BluePiercesEnemiesUsingPowerBeforeIncrease) {
    for (int p = 0; p < 2; ++p) {
        auto* bullet = Shot(TankType::TYPE_BLUE, TankType::TYPE_GREY, true, p);
        Tank first, second;
        first.identity = TankIdentity::Enemy(10);
        second.identity = TankIdentity::Enemy(11);
        first.health = second.health = 1000;
        Hit(bullet, &first);
        EXPECT_FLOAT_EQ(first.health, 800);
        EXPECT_TRUE(bullet->IsAlive());
        EXPECT_FLOAT_EQ(bullet->GetPower(), 300);
        Hit(bullet, &second);
        EXPECT_FLOAT_EQ(second.health, 700);
        EXPECT_TRUE(bullet->IsAlive());
        EXPECT_FLOAT_EQ(bullet->GetPower(), 400);
    }
}

TEST_F(BulletSpecialTest, BlueDoesNotPierceOrdinaryEnemyOrPlayerContacts) {
    Tank target;
    target.identity = TankIdentity::Enemy(10);
    target.health = 1000;
    for (auto bullet : {Shot(TankType::TYPE_BLUE, TankType::TYPE_GREY, false),
                        Shot(TankType::TYPE_BLUE, TankType::TYPE_GREY, true, -1)}) {
        Hit(bullet, &target);
        EXPECT_FALSE(bullet->IsAlive());
        EXPECT_FLOAT_EQ(bullet->GetPower(), 200);
    }
    target.isPlayer = true;
    for (int p = 0; p < 2; ++p) {
        target.identity = TankIdentity::Player(p);
        auto* bullet = Shot(TankType::TYPE_BLUE, TankType::TYPE_GREY);
        Hit(bullet, &target);
        EXPECT_FALSE(bullet->IsAlive());
        EXPECT_FLOAT_EQ(bullet->GetPower(), 200);
    }
}

TEST_F(BulletSpecialTest, PurpleAccelerationPreservesSignAndOrdinaryMotion) {
    for (float pressed : {-1.0f, 0.0f, 1.0f}) {
        for (bool special : {false, true}) {
            auto* bullet = Shot(TankType::TYPE_PURPLE, TankType::TYPE_GREY, special, 0, 4, pressed);
            float speed = 320 * pressed;
            for (int step = 0; step < 2; ++step) {
                float angle = bullet->GetRY();
                if (special) speed += pressed < 0 ? -10 : 10;
                bullet->Update();
                EXPECT_NEAR(bullet->GetRY(), angle + 0.01f * speed, 0.0001f);
            }
        }
    }
}

namespace {
void ExpectColor(const Color& actual, const Color& expected) {
    EXPECT_FLOAT_EQ(actual.r, expected.r);
    EXPECT_FLOAT_EQ(actual.g, expected.g);
    EXPECT_FLOAT_EQ(actual.b, expected.b);
    EXPECT_FLOAT_EQ(actual.a, expected.a);
}
}

TEST_F(BulletSpecialTest, YellowChildrenPreserveEveryCombinationForBothPlayers) {
    for (int p = 0; p < 2; ++p) {
        for (auto secondary : {TankType::TYPE_RED, TankType::TYPE_BLUE, TankType::TYPE_YELLOW, TankType::TYPE_GREY, TankType::TYPE_PURPLE}) {
            SCOPED_TRACE(p);
            SCOPED_TRACE(static_cast<int>(secondary));
            auto* parent = Shot(TankType::TYPE_YELLOW, secondary, true, p);
            float initialPower = parent->GetPower();
            const size_t count = secondary == TankType::TYPE_RED ? 20 : secondary == TankType::TYPE_BLUE ? 2 : 1;
            for (int bounce = 0; bounce < 4; ++bounce) {
                const size_t start = world.GetBullets().size();
                // Simulate crossing an X cell boundary. The handler rewinds movement.
                float originalAngle = parent->GetRY();
                parent->HandleLevelCollision(1, 0, originalAngle);
                ASSERT_EQ(world.GetBullets().size(), start + count);
                float power = initialPower * (secondary == TankType::TYPE_YELLOW ? 2 : 1.5f);
                EXPECT_FLOAT_EQ(parent->GetPower(), power);
                for (size_t i = 0; i < count; ++i) {
                    auto* child = world.GetBullets()[start + i].get();
                    EXPECT_EQ(child->GetOwnerIdentity(), TankIdentity::Player(p));
                    EXPECT_TRUE(child->IsAlive());
                    EXPECT_FALSE(child->GetIsSpecial());
                    EXPECT_FLOAT_EQ(child->GetDT(), 0);
                    EXPECT_FLOAT_EQ(child->GetPower(), secondary == TankType::TYPE_PURPLE ? power * 2 : power);
                    bool grey = secondary == TankType::TYPE_YELLOW || secondary == TankType::TYPE_GREY;
                    EXPECT_EQ(child->GetType1(), grey ? TankType::TYPE_GREY : secondary == TankType::TYPE_RED ? TankType::TYPE_RED : TankType::TYPE_YELLOW);
                    EXPECT_EQ(child->GetType2(), grey ? TankType::TYPE_GREY : secondary);
                    EXPECT_EQ(child->GetBounces(), secondary == TankType::TYPE_RED ? 0 : secondary == TankType::TYPE_PURPLE ? 16 : 4);
                    EXPECT_FLOAT_EQ(child->GetRY(), secondary == TankType::TYPE_RED ? i * 18 : grey ? -originalAngle : parent->GetRY());
                    EXPECT_FLOAT_EQ(child->GetRX(), parent->GetRX());
                    EXPECT_FLOAT_EQ(child->GetRZ(), parent->GetRZ());
                    EXPECT_FLOAT_EQ(child->GetY(), parent->GetY());
                    float offset = secondary == TankType::TYPE_BLUE ? 0.2f + i * 0.2f + bounce * 0.2f : 0;
                    float radians = (parent->GetRY() + 90) * 3.14159265358979323846f / 180;
                    EXPECT_NEAR(child->GetX(), parent->GetX() + offset * std::cos(radians), 0.0001f);
                    EXPECT_NEAR(child->GetZ(), parent->GetZ() + offset * std::sin(radians), 0.0001f);
                    ExpectColor(child->GetPrimaryColor(), secondary == TankType::TYPE_RED ? Color(1, 0, 0) : parent->GetPrimaryColor());
                    ExpectColor(child->GetSecondaryColor(), secondary == TankType::TYPE_RED ? Color(1, 0, 0) : parent->GetSecondaryColor());
                    float angle = child->GetRY();
                    child->Update();
                    EXPECT_NEAR(child->GetRY(), angle + (secondary == TankType::TYPE_PURPLE ? 0.1f : 0), 0.0001f);
                    // Every child starts with its full bounce allowance, even on later parent bounces.
                    for (int n = 0; n < child->GetBounces(); ++n) {
                        child->HandleLevelCollision(0, 0, child->GetRY());
                        EXPECT_TRUE(child->IsAlive());
                        EXPECT_EQ(world.GetBullets().size(), start + count);
                    }
                    // Exhaustion creates wall FX through the child's attached world.
                    child->HandleLevelCollision(1, 0, child->GetRY());
                    EXPECT_FALSE(child->IsAlive());
                    EXPECT_EQ(world.GetBullets().size(), start + count);
                }
            }
            size_t total = world.GetBullets().size();
            parent->HandleLevelCollision(1, 0, parent->GetRY());
            EXPECT_FALSE(parent->IsAlive());
            EXPECT_EQ(world.GetBullets().size(), total);
            Events::GetBus().Clear();
            world.Clear();
        }
    }
}

TEST_F(BulletSpecialTest, YellowOnlySplitsPlayerSpecialsWithBouncesRemaining) {
    for (auto* bullet : {Shot(TankType::TYPE_YELLOW, TankType::TYPE_RED, false),
                         Shot(TankType::TYPE_YELLOW, TankType::TYPE_RED, true, -1),
                         Shot(TankType::TYPE_YELLOW, TankType::TYPE_RED, true, 0, 0),
                         Shot(TankType::TYPE_RED, TankType::TYPE_YELLOW)}) {
        size_t count = world.GetBullets().size();
        bullet->HandleLevelCollision(1, 0, 30);
        EXPECT_EQ(world.GetBullets().size(), count);
    }
}

TEST_F(BulletSpecialTest, QueuedWallCollisionSpawnsBeforeUpdateAndCleansUpChildren) {
    for (int p = 0; p < 2; ++p) {
        for (auto secondary : {TankType::TYPE_RED, TankType::TYPE_BLUE, TankType::TYPE_YELLOW, TankType::TYPE_GREY, TankType::TYPE_PURPLE}) {
            auto& level = LevelHandler::GetSingleton();
            level.SetTerrainHeight(31, 30, 5);
            auto* parent = world.CreateBullet(TankIdentity::Player(p), 200, TankType::TYPE_YELLOW,
                secondary, 4, 0, Color(1, 1, 0), Color(0, 0, 1), 30.8f, 2, 30.5f, 0, 0, 0, true);
            world.Update();
            ASSERT_EQ(world.GetBullets().size(), 1u);
            EXPECT_GT(parent->GetX(), 31);
            Events::ProcessQueuedEvents(); // Same ordering as GameTask::HandlePlayingState.
            const size_t count = secondary == TankType::TYPE_RED ? 20 : secondary == TankType::TYPE_BLUE ? 2 : 1;
            ASSERT_EQ(world.GetBullets().size(), 1 + count);
            EXPECT_NEAR(parent->GetX(), 30.8f, 0.0001f);
            for (size_t i = 1; i <= count; ++i) EXPECT_FLOAT_EQ(world.GetBullets()[i]->GetDT(), 0);
            level.SetTerrainHeight(31, 30, 0);
            world.Update();
            for (size_t i = 1; i <= count; ++i) EXPECT_FLOAT_EQ(world.GetBullets()[i]->GetDT(), GlobalTimer::dT);
            // Queued expiration kills; the following world update unregisters and removes.
            for (const auto& bullet : world.GetBullets())
                Events::GetBus().Post(BulletTimeoutEvent(bullet.get(), 100));
            Events::ProcessQueuedEvents();
            world.Update();
            EXPECT_TRUE(world.GetBullets().empty());
            Events::GetBus().Clear();
        }
    }
}
