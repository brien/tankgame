#include <gtest/gtest.h>

#include "../src/GameWorld.h"
#include "../src/PlayerManager.h"
#include "../src/Player.h"
#include "../src/Tank.h"
#include "../src/TankIdentity.h"
#include "../src/LevelHandler.h"

// Regression charter:
// Protect PlayerManager ownership/tank-id invariants after player/tank refactors.

class PlayerManagerRegressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        LevelHandler::Create();
        gameWorld.Initialize();
        playerManager.Initialize(&gameWorld);
    }

    void TearDown() override {
        playerManager.Shutdown();
        gameWorld.Clear();
        gameWorld.Shutdown();
        LevelHandler::Destroy();
    }

    GameWorld gameWorld;
    PlayerManager playerManager;
};

TEST_F(PlayerManagerRegressionTest, GetPlayerByTankId_MapsLegacyPlayerIds) {
    playerManager.SetNumPlayers(2);

    Player* player0 = playerManager.GetPlayer(0);
    Player* player1 = playerManager.GetPlayer(1);

    ASSERT_NE(player0, nullptr);
    ASSERT_NE(player1, nullptr);

    EXPECT_EQ(playerManager.GetPlayerByTankId(-1), player0);
    EXPECT_EQ(playerManager.GetPlayerByTankId(-2), player1);
}

TEST_F(PlayerManagerRegressionTest, GetPlayerByTankId_RejectsInvalidIds) {
    playerManager.SetNumPlayers(2);

    EXPECT_EQ(playerManager.GetPlayerByTankId(0), nullptr);
    EXPECT_EQ(playerManager.GetPlayerByTankId(1), nullptr);
    EXPECT_EQ(playerManager.GetPlayerByTankId(-3), nullptr);
    EXPECT_EQ(playerManager.GetPlayerByTankId(-99), nullptr);
}

TEST_F(PlayerManagerRegressionTest, ResetHitComboByTankId_OnlyResetsTargetPlayer) {
    playerManager.SetNumPlayers(2);

    Player* player0 = playerManager.GetPlayer(0);
    Player* player1 = playerManager.GetPlayer(1);

    ASSERT_NE(player0, nullptr);
    ASSERT_NE(player1, nullptr);

    player0->IncrementHitCombo();
    player0->IncrementHitCombo();
    player1->IncrementHitCombo();
    player1->IncrementHitCombo();
    player1->IncrementHitCombo();

    ASSERT_EQ(player0->GetHitCombo(), 2);
    ASSERT_EQ(player1->GetHitCombo(), 3);

    playerManager.ResetHitComboByTankId(-1);
    EXPECT_EQ(player0->GetHitCombo(), 0);
    EXPECT_EQ(player1->GetHitCombo(), 3);

    playerManager.ResetHitComboByTankId(1);   // non-player tank id
    playerManager.ResetHitComboByTankId(-99); // out-of-range player id
    EXPECT_EQ(player0->GetHitCombo(), 0);
    EXPECT_EQ(player1->GetHitCombo(), 3);
}

TEST_F(PlayerManagerRegressionTest, EnsurePlayerTankSetup_RevalidatesPlayerIdentityFields) {
    Player* player0 = playerManager.GetPlayer(0);
    ASSERT_NE(player0, nullptr);

    Tank* tank = gameWorld.CreateTank();
    ASSERT_NE(tank, nullptr);
    tank->alive = true;

    player0->TakeControlOf(tank);
    ASSERT_EQ(player0->GetControlledTank(), tank);

    // Corrupt identity state to simulate stale/invalid setup after transitions.
    tank->identity = TankIdentity::Enemy(7);
    tank->isPlayer = false;
    tank->jid = 99;

    playerManager.EnsurePlayerTankSetup();

    EXPECT_TRUE(tank->identity.IsPlayer());
    EXPECT_EQ(tank->identity.GetPlayerIndex(), 0);
    EXPECT_EQ(tank->identity.GetLegacyId(), -1);
    EXPECT_TRUE(tank->isPlayer);
    EXPECT_EQ(tank->jid, 0u);
}

TEST_F(PlayerManagerRegressionTest, TargetingTracksEnemyDirectionAcrossAllQuadrants) {
    Tank* player = gameWorld.CreateTank();
    playerManager.GetPlayer(0)->TakeControlOf(player);
    player->alive = true;
    player->x = 10.0f;
    player->z = 20.0f;
    Tank* enemy = gameWorld.CreateTank();
    enemy->alive = true;
    enemy->isPlayer = false;

    const float offsets[][3] = {
        {5, 0, 0}, {5, 5, 45}, {0, 5, 90}, {-5, 5, 135},
        {-5, 0, 180}, {-5, -5, -135}, {0, -5, -90}, {5, -5, -45}
    };
    for (const auto& offset : offsets) {
        enemy->x = player->x + offset[0];
        enemy->z = player->z + offset[1];
        playerManager.UpdatePlayerTargeting();
        EXPECT_NEAR(player->rrl, offset[2], 0.001f);
    }
}

TEST_F(PlayerManagerRegressionTest, TargetingSelectsNearestLivingEnemyAndRetargets) {
    Tank* player = gameWorld.CreateTank();
    playerManager.GetPlayer(0)->TakeControlOf(player);
    player->alive = true;
    player->x = player->z = 0.0f;
    Tank* nearEnemy = gameWorld.CreateTank();
    nearEnemy->alive = true;
    nearEnemy->isPlayer = false;
    nearEnemy->x = 0.0f;
    nearEnemy->z = 2.0f;
    Tank* farEnemy = gameWorld.CreateTank();
    farEnemy->alive = true;
    farEnemy->isPlayer = false;
    farEnemy->x = -5.0f;
    farEnemy->z = 0.0f;

    playerManager.UpdatePlayerTargeting();
    EXPECT_NEAR(player->rrl, 90.0f, 0.001f);
    nearEnemy->alive = false;
    playerManager.UpdatePlayerTargeting();
    EXPECT_NEAR(player->rrl, 180.0f, 0.001f);
    farEnemy->x = farEnemy->z = 0.0f;
    playerManager.UpdatePlayerTargeting();
    EXPECT_NEAR(player->rrl, 180.0f, 0.001f);
    farEnemy->alive = false;
    playerManager.UpdatePlayerTargeting();
    EXPECT_NEAR(player->rrl, 180.0f, 0.001f);
}

TEST_F(PlayerManagerRegressionTest, TargetingIncludesOtherPlayerOnlyInVersusMode) {
    playerManager.SetNumPlayers(2);
    Tank* first = gameWorld.CreateTank();
    Tank* second = gameWorld.CreateTank();
    playerManager.GetPlayer(0)->TakeControlOf(first);
    playerManager.GetPlayer(1)->TakeControlOf(second);
    first->alive = second->alive = true;
    first->x = first->z = second->x = 0.0f;
    second->z = 5.0f;
    first->rrl = 45.0f;
    second->rrl = 45.0f;

    playerManager.UpdatePlayerTargeting();
    EXPECT_FLOAT_EQ(first->rrl, 45.0f);
    EXPECT_FLOAT_EQ(second->rrl, 45.0f);
    playerManager.SetVersusMode(true);
    playerManager.UpdatePlayerTargeting();
    EXPECT_NEAR(first->rrl, 90.0f, 0.001f);
    EXPECT_NEAR(second->rrl, -90.0f, 0.001f);
}
