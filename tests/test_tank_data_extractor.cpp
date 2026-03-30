#include <gtest/gtest.h>

#include "../src/rendering/TankDataExtractor.h"
#include "../src/TankIdentity.h"

// Regression charter:
// Protect Tank->TankRenderData field mapping used by rendering after refactors.

TEST(TankDataExtractorRegression, ExtractRenderData_MapsCoreFields) {
    Tank tank;

    // Spatial state
    tank.x = 10.5f;
    tank.y = 2.25f;
    tank.z = -7.75f;
    tank.rx = 11.0f;
    tank.ry = 22.0f;
    tank.rz = 33.0f;
    tank.rtx = 44.0f;
    tank.rty = 55.0f;
    tank.rtz = 66.0f;
    tank.rrl = 77.0f;

    // Survival/action resources
    tank.health = 87.0f;
    tank.maxHealth = 120.0f;
    tank.energy = 42.0f;
    tank.maxEnergy = 80.0f;
    tank.fireCost = 9.5f;

    // Identity and status
    tank.alive = true;
    tank.isGrounded = false;
    tank.isJumping = true;
    tank.identity = TankIdentity::Player(1); // legacy id should be -2

    // Movement/effects
    tank.vx = 1.1f;
    tank.vy = 2.2f;
    tank.vz = 3.3f;
    tank.bonus = 4;
    tank.bonusTime = 5.5f;
    tank.hitAlpha = 0.6f;
    tank.hitNum = 7;

    TankRenderData data = TankDataExtractor::ExtractRenderData(tank);

    EXPECT_FLOAT_EQ(data.position.x, tank.x);
    EXPECT_FLOAT_EQ(data.position.y, tank.y);
    EXPECT_FLOAT_EQ(data.position.z, tank.z);
    EXPECT_FLOAT_EQ(data.bodyRotation.x, tank.rx);
    EXPECT_FLOAT_EQ(data.bodyRotation.y, tank.ry);
    EXPECT_FLOAT_EQ(data.bodyRotation.z, tank.rz);
    EXPECT_FLOAT_EQ(data.turretRotation.x, tank.rtx);
    EXPECT_FLOAT_EQ(data.turretRotation.y, tank.rty);
    EXPECT_FLOAT_EQ(data.turretRotation.z, tank.rtz);
    EXPECT_FLOAT_EQ(data.targetRotation, tank.rrl);

    EXPECT_FLOAT_EQ(data.health, tank.health);
    EXPECT_FLOAT_EQ(data.maxHealth, tank.maxHealth);
    EXPECT_FLOAT_EQ(data.charge, tank.energy);
    EXPECT_FLOAT_EQ(data.maxCharge, tank.maxEnergy);
    EXPECT_FLOAT_EQ(data.fireCost, tank.fireCost);

    EXPECT_TRUE(data.alive);
    EXPECT_TRUE(data.isPlayer);
    EXPECT_EQ(data.playerId, -2);

    EXPECT_FALSE(data.grounded);
    EXPECT_TRUE(data.isJumping);
    EXPECT_FLOAT_EQ(data.velocity.x, tank.vx);
    EXPECT_FLOAT_EQ(data.velocity.y, tank.vy);
    EXPECT_FLOAT_EQ(data.velocity.z, tank.vz);

    EXPECT_EQ(data.bonus, tank.bonus);
    EXPECT_FLOAT_EQ(data.bonusTime, tank.bonusTime);
    EXPECT_FLOAT_EQ(data.hitAlpha, tank.hitAlpha);
    EXPECT_EQ(data.hitNum, tank.hitNum);

    // Color mapping should come from tank type-derived color functions.
    Color expectedPrimary = tank.GetPrimaryColor();
    Color expectedSecondary = tank.GetSecondaryColor();
    EXPECT_FLOAT_EQ(data.primaryColor.r, expectedPrimary.r);
    EXPECT_FLOAT_EQ(data.primaryColor.g, expectedPrimary.g);
    EXPECT_FLOAT_EQ(data.primaryColor.b, expectedPrimary.b);
    EXPECT_FLOAT_EQ(data.primaryColor.a, expectedPrimary.a);
    EXPECT_FLOAT_EQ(data.secondaryColor.r, expectedSecondary.r);
    EXPECT_FLOAT_EQ(data.secondaryColor.g, expectedSecondary.g);
    EXPECT_FLOAT_EQ(data.secondaryColor.b, expectedSecondary.b);
    EXPECT_FLOAT_EQ(data.secondaryColor.a, expectedSecondary.a);
}
