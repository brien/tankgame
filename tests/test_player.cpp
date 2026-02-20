#include <gtest/gtest.h>
#include "../src/Player.h"
#include "../src/GameWorld.h"
#include "../src/Tank.h"

// Mock or minimal GameWorld for testing purposes
class MockGameWorld : public GameWorld {
public:
    MockGameWorld() : GameWorld() {}
    
    // We'll override Create methods to return test tanks if needed
    Tank* CreateTestTank() {
        Tank* tank = CreateTank();
        if (tank) {
            tank->Init();
        }
        return tank;
    }
};

// Test fixture for Player tests
class PlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        gameWorld = new MockGameWorld();
        gameWorld->Initialize();
    }

    void TearDown() override {
        delete gameWorld;
    }

    MockGameWorld* gameWorld;
};

// === CONSTRUCTION AND BASIC PROPERTIES ===

TEST_F(PlayerTest, Constructor_InitializesPlayerIndex) {
    Player player(0, gameWorld);
    EXPECT_EQ(player.GetPlayerIndex(), 0);
}

TEST_F(PlayerTest, Constructor_InitializesWithNullTank) {
    Player player(0, gameWorld);
    EXPECT_FALSE(player.HasTank());
    EXPECT_EQ(player.GetControlledTank(), nullptr);
}

TEST_F(PlayerTest, Constructor_InitializesScoreToZero) {
    Player player(0, gameWorld);
    EXPECT_EQ(player.GetScore(), 0);
}

TEST_F(PlayerTest, Constructor_InitializesComboToZero) {
    Player player(0, gameWorld);
    EXPECT_FLOAT_EQ(player.GetCombo(), 0.0f);
    EXPECT_EQ(player.GetComboNumber(), 0);
    EXPECT_EQ(player.GetHitCombo(), 0);
}

TEST_F(PlayerTest, Constructor_InitializesSpecialChargeToZero) {
    Player player(0, gameWorld);
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 0.0f);
}

TEST_F(PlayerTest, Constructor_InitializesWinsToZero) {
    Player player(0, gameWorld);
    EXPECT_EQ(player.GetWins(), 0);
}

// === SCORE MANAGEMENT ===

TEST_F(PlayerTest, AddScore_IncreasesScore) {
    Player player(0, gameWorld);
    player.AddScore(100);
    EXPECT_EQ(player.GetScore(), 100);
}

TEST_F(PlayerTest, AddScore_AccumulatesMultipleTimes) {
    Player player(0, gameWorld);
    player.AddScore(100);
    player.AddScore(50);
    player.AddScore(25);
    EXPECT_EQ(player.GetScore(), 175);
}

TEST_F(PlayerTest, ResetScore_SetsScoreToZero) {
    Player player(0, gameWorld);
    player.AddScore(500);
    player.ResetScore();
    EXPECT_EQ(player.GetScore(), 0);
}

// === COMBO SYSTEM ===

TEST_F(PlayerTest, AddCombo_IncreasesComboValue) {
    Player player(0, gameWorld);
    player.AddCombo(10.0f, 1);
    EXPECT_FLOAT_EQ(player.GetCombo(), 10.0f);
    EXPECT_EQ(player.GetComboNumber(), 1);
}

TEST_F(PlayerTest, AddCombo_AccumulatesComboPoints) {
    Player player(0, gameWorld);
    player.AddCombo(10.0f, 1);
    player.AddCombo(20.0f, 2);
    EXPECT_FLOAT_EQ(player.GetCombo(), 30.0f);
    EXPECT_EQ(player.GetComboNumber(), 2);
}

TEST_F(PlayerTest, AddCombo_AddsSpecialCharge) {
    Player player(0, gameWorld);
    player.AddCombo(100.0f, 1);
    // Special charge should be 10% of combo = 10.0f
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 10.0f);
}

TEST_F(PlayerTest, ResetCombo_ClearsAllComboValues) {
    Player player(0, gameWorld);
    player.AddCombo(50.0f, 5);
    player.IncrementHitCombo();
    player.IncrementHitCombo();
    
    player.ResetCombo();
    
    EXPECT_FLOAT_EQ(player.GetCombo(), 0.0f);
    EXPECT_EQ(player.GetComboNumber(), 0);
    EXPECT_EQ(player.GetHitCombo(), 0);
}

TEST_F(PlayerTest, IncrementHitCombo_IncreasesHitCombo) {
    Player player(0, gameWorld);
    player.IncrementHitCombo();
    player.IncrementHitCombo();
    player.IncrementHitCombo();
    EXPECT_EQ(player.GetHitCombo(), 3);
}

TEST_F(PlayerTest, ResetHitCombo_SetsHitComboToZero) {
    Player player(0, gameWorld);
    player.IncrementHitCombo();
    player.IncrementHitCombo();
    player.ResetHitCombo();
    EXPECT_EQ(player.GetHitCombo(), 0);
}

// === SPECIAL CHARGE SYSTEM ===

TEST_F(PlayerTest, AddSpecialCharge_IncreasesCharge) {
    Player player(0, gameWorld);
    player.AddSpecialCharge(25.0f);
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 25.0f);
}

TEST_F(PlayerTest, AddSpecialCharge_CapsAtMaximum) {
    Player player(0, gameWorld);
    player.AddSpecialCharge(150.0f); // Exceeds SPECIAL_CHARGE_MAX (100.0f)
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 100.0f);
}

TEST_F(PlayerTest, AddSpecialCharge_AccumulatesUpToMax) {
    Player player(0, gameWorld);
    player.AddSpecialCharge(60.0f);
    player.AddSpecialCharge(50.0f);
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 100.0f);
}

TEST_F(PlayerTest, UseSpecialCharge_DecreasesCharge) {
    Player player(0, gameWorld);
    player.AddSpecialCharge(50.0f);
    player.UseSpecialCharge(20.0f);
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 30.0f);
}

TEST_F(PlayerTest, UseSpecialCharge_FlooredAtZero) {
    Player player(0, gameWorld);
    player.AddSpecialCharge(10.0f);
    player.UseSpecialCharge(20.0f);
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 0.0f);
}

TEST_F(PlayerTest, SetSpecialCharge_SetsExactValue) {
    Player player(0, gameWorld);
    player.SetSpecialCharge(75.0f);
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 75.0f);
}

TEST_F(PlayerTest, CanUseSpecial_ReturnsTrueWhenSufficientCharge) {
    Player player(0, gameWorld);
    player.AddSpecialCharge(50.0f);
    EXPECT_TRUE(player.CanUseSpecial(30.0f));
    EXPECT_TRUE(player.CanUseSpecial(50.0f));
}

TEST_F(PlayerTest, CanUseSpecial_ReturnsFalseWhenInsufficientCharge) {
    Player player(0, gameWorld);
    player.AddSpecialCharge(30.0f);
    EXPECT_FALSE(player.CanUseSpecial(50.0f));
}

// === WINS TRACKING ===

TEST_F(PlayerTest, AddWin_IncreasesWinCount) {
    Player player(0, gameWorld);
    player.AddWin();
    EXPECT_EQ(player.GetWins(), 1);
}

TEST_F(PlayerTest, AddWin_AccumulatesMultipleWins) {
    Player player(0, gameWorld);
    player.AddWin();
    player.AddWin();
    player.AddWin();
    EXPECT_EQ(player.GetWins(), 3);
}

TEST_F(PlayerTest, ResetWins_SetsWinsToZero) {
    Player player(0, gameWorld);
    player.AddWin();
    player.AddWin();
    player.ResetWins();
    EXPECT_EQ(player.GetWins(), 0);
}

// === TANK CONTROL ===

// NOTE: Tank creation tests are skipped because they require LevelHandler singleton
// which is not available in the test environment. These would need proper mocking.

TEST_F(PlayerTest, TakeControlOf_NullTank_DoesNotCrash) {
    Player player(0, gameWorld);
    
    // Should not crash when taking control of nullptr
    EXPECT_NO_THROW(player.TakeControlOf(nullptr));
    EXPECT_FALSE(player.HasTank());
}

TEST_F(PlayerTest, ReleaseTank_HandlesNullTank) {
    Player player(0, gameWorld);
    
    // Should not crash when releasing with no tank
    EXPECT_NO_THROW(player.ReleaseTank());
}

// === MULTIPLE PLAYERS ===

TEST_F(PlayerTest, MultiplePlayersHaveUniqueIndices) {
    Player player1(0, gameWorld);
    Player player2(1, gameWorld);
    
    EXPECT_EQ(player1.GetPlayerIndex(), 0);
    EXPECT_EQ(player2.GetPlayerIndex(), 1);
}

// === INTEGRATION TESTS ===

TEST_F(PlayerTest, FullGameScenario_ScoreComboAndSpecial) {
    Player player(0, gameWorld);
    
    // Start with no score or combo
    EXPECT_EQ(player.GetScore(), 0);
    EXPECT_FLOAT_EQ(player.GetCombo(), 0.0f);
    
    // Hit enemy, gain score and combo
    player.AddScore(100);
    player.AddCombo(10.0f, 1);
    
    EXPECT_EQ(player.GetScore(), 100);
    EXPECT_FLOAT_EQ(player.GetCombo(), 10.0f);
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 1.0f); // 10% of 10
    
    // Continue combo
    player.AddScore(200);
    player.AddCombo(20.0f, 2);
    
    EXPECT_EQ(player.GetScore(), 300);
    EXPECT_FLOAT_EQ(player.GetCombo(), 30.0f);
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 3.0f); // 10% of 30 total
    
    // Use special attack
    player.UseSpecialCharge(2.0f);
    EXPECT_FLOAT_EQ(player.GetSpecialCharge(), 1.0f);
    
    // Break combo
    player.ResetCombo();
    EXPECT_FLOAT_EQ(player.GetCombo(), 0.0f);
    EXPECT_EQ(player.GetScore(), 300); // Score persists
}
