#include <gtest/gtest.h>
#include <algorithm>
#include "App.h"
#include "LevelHandler.h"
#include "TankTypeManager.h"

class TankJumpTest : public ::testing::TestWithParam<int> {
protected:
    App app;
    GameTask task;
    SoundTask sound;
    GameWorld world;
    float oldDT = GlobalTimer::dT;

    void SetUp() override {
        app.gameTask = &task;
        app.soundTask = &sound;
        sound.disable = true;
        LevelHandler::Create();
        LevelHandler::GetSingleton().Load(nullptr);
        LevelHandler::GetSingleton().Flatten(3);
        TankTypeManager::Initialize();
        GlobalTimer::dT = 1.0f / GetParam();
    }

    void TearDown() override {
        world.Clear();
        LevelHandler::Destroy();
        GlobalTimer::dT = oldDT;
    }

    void Prepare(Tank& tank, float energy) {
        tank.Init();
        tank.SetGameWorld(&world);
        tank.SetType(TankType::TYPE_RED, TankType::TYPE_BLUE);
        tank.SetPosition(30.5f, 3.0f, 30.5f);
        tank.isGrounded = true;
        tank.energy = energy;
    }
};

TEST_P(TankJumpTest, FullEnergyLandsOnTwoBlockLedge) {
    for (bool regenerate : {false, true}) {
        SCOPED_TRACE(regenerate);
        Tank tank;
        Prepare(tank, 100.0f);
        if (!regenerate) tank.energyRegen = 0;
        auto& level = LevelHandler::GetSingleton();
        for (int x = 32; x < 40; ++x)
            for (int z = 29; z < 33; ++z)
                level.SetTerrainHeight(x, z, 5);

        bool exhausted = false;
        float peak = tank.y;
        for (int frame = 0; frame < GetParam() * 3; ++frame) {
            if (!exhausted) {
                tank.Jump();
                exhausted = tank.energy <= 0;
            }
            if (tank.x < 33.5f) tank.Move(1.0f);
            tank.NextFrame();
            peak = std::max(peak, tank.y);
            if (exhausted && tank.isGrounded) break;
        }
        EXPECT_TRUE(exhausted);
        EXPECT_GT(peak, 5.2f);
        EXPECT_GE(tank.x, 32.5f);
        EXPECT_FLOAT_EQ(tank.y, 5.0f);
        EXPECT_TRUE(tank.isGrounded);
    }
}

TEST_P(TankJumpTest, TapStaysBelowTwoBlocks) {
    Tank tank;
    Prepare(tank, 100.0f);
    tank.Jump();
    float peak = tank.y;
    for (int frame = 0; frame < GetParam() * 2; ++frame) {
        tank.Fall();
        peak = std::max(peak, tank.y);
    }
    EXPECT_GT(peak, 3.0f);
    EXPECT_LT(peak, 5.0f);
    EXPECT_TRUE(tank.isGrounded);
}

TEST_P(TankJumpTest, LastEnergyProvidesProportionalThrustWithoutGoingNegative) {
    Tank tank;
    Prepare(tank, 0.5f);
    tank.jumpTime = 0.1f;
    tank.vy = 1.0f;
    tank.Jump();
    EXPECT_FLOAT_EQ(tank.energy, 0.0f);
    EXPECT_NEAR(tank.vy, 1.0f + tank.jumpRate * 0.5f / tank.jumpCost, 0.00001f);
    const float velocity = tank.vy;
    tank.Jump();
    EXPECT_FLOAT_EQ(tank.vy, velocity);
}

TEST_P(TankJumpTest, RegeneratedEnergyIsFullyConsumedByNextJump) {
    Tank tank;
    Prepare(tank, 0.0f);
    tank.NextFrame();
    ASSERT_GT(tank.energy, 0.0f);
    ASSERT_LT(tank.energy, tank.jumpCost * GlobalTimer::dT);

    tank.jumpTime = 0.1f;
    tank.vy = 1.0f;
    const float expectedVelocity = tank.vy + tank.jumpRate * (tank.energy / tank.jumpCost);
    tank.Jump();
    EXPECT_EQ(tank.energy, 0.0f);
    EXPECT_NEAR(tank.vy, expectedVelocity, 0.00001f);
    tank.Jump();
    EXPECT_EQ(tank.energy, 0.0f);
    EXPECT_NEAR(tank.vy, expectedVelocity, 0.00001f);
}

INSTANTIATE_TEST_SUITE_P(FrameRates, TankJumpTest, ::testing::Values(30, 60, 144, 240));
