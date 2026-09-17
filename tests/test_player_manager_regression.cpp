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
