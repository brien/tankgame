#include <gtest/gtest.h>
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
        Events::GetBus().ClearAll();
        app.gameTask = &task;
        app.soundTask = &sound;
        sound.disable = true;
        GlobalTimer::dT = 0.01f;
        LevelHandler::Create();
        LevelHandler::GetSingleton().Flatten(0);
        world.Initialize();
        task.GetPlayerManager()->Initialize(&world);
        task.GetPlayerManager()->SetNumPlayers(2);
    }
    void TearDown() override {
        Events::GetBus().ClearAll();
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
