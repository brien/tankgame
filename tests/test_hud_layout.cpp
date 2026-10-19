#include <gtest/gtest.h>
#include "rendering/HUDLayout.h"
#include <limits>

TEST(HUDLayoutTest, SparseAndReorderedPlayersRetainTheirViewportIdentity) {
    HUDRenderData second, first;
    second.playerId = 1; second.health = 25;
    first.playerId = 0; first.health = 75;
    std::vector<HUDRenderData> players{second};
    EXPECT_EQ(HUDLayout::FindPlayer(players, 0), nullptr);
    ASSERT_NE(HUDLayout::FindPlayer(players, 1), nullptr);
    EXPECT_FLOAT_EQ(HUDLayout::FindPlayer(players, 1)->health, 25);
    players.push_back(first);
    EXPECT_FLOAT_EQ(HUDLayout::FindPlayer(players, 0)->health, 75);
    EXPECT_FLOAT_EQ(HUDLayout::FindPlayer(players, 1)->health, 25);
    EXPECT_EQ(HUDLayout::FindPlayer(players, -1), nullptr);
}

TEST(HUDLayoutTest, HiddenMetersDoNotLeaveIconsOrCostMarkers) {
    HUDRenderData data;
    data.showHealthBar = data.showEnergyBar = data.showComboMeter = data.showSpecialMeter = false;
    // Text and targeting flags do not create geometry in the meter slice.
    EXPECT_TRUE(HUDLayout::Build(data).empty());
    data.showDebugInfo = true;
    EXPECT_EQ(HUDLayout::Build(data).size(), 1u);
}

TEST(HUDLayoutTest, HealthAndEnergyFillStayWithinBarsForInvalidOrOverchargedValues) {
    HUDRenderData data;
    for (const auto& pair : {std::make_pair(-20.f, 100.f), std::make_pair(150.f, 100.f),
                            std::make_pair(100.f, 0.f), std::make_pair(100.f, -10.f)}) {
        data.health = data.energy = pair.first;
        data.maxHealth = data.maxEnergy = pair.second;
        const auto draws = HUDLayout::Build(data);
        ASSERT_GE(draws.size(), 8u);
        EXPECT_GE(draws[1].width, 0); EXPECT_LE(draws[1].width, .40f);
        EXPECT_FLOAT_EQ(draws[1].width, draws[5].width);
        EXPECT_EQ(draws[3].icon, HUDLayout::Icon::Health);
        EXPECT_EQ(draws[7].icon, HUDLayout::Icon::Energy);
    }
}

TEST(HUDLayoutTest, ActionMarkersReflectAffordabilityAndDistinctCosts) {
    HUDRenderData data;
    data.showHealthBar = data.showComboMeter = data.showSpecialMeter = false;
    data.jumpCost = data.fireCost;
    const auto affordable = HUDLayout::Build(data);
    ASSERT_EQ(affordable.size(), 5u);
    data.canFire = false; data.jumpCost += 10;
    const auto depleted = HUDLayout::Build(data);
    ASSERT_EQ(depleted.size(), 6u);
    EXPECT_FLOAT_EQ(affordable[4].x, depleted[4].x);
    EXPECT_NE(affordable[4].color.y, depleted[4].color.y);
    EXPECT_GT(depleted[5].x, depleted[4].x);
}

TEST(HUDLayoutTest, SpecialTicksHandleZeroNegativeAndNonfiniteCostsWithoutUnboundedWork) {
    HUDRenderData data;
    data.special = 100; data.fireCost = 50;
    EXPECT_EQ(HUDLayout::SpecialIndicatorCount(data), 10);
    for (float cost : {0.f, -10.f, std::numeric_limits<float>::infinity(),
                       std::numeric_limits<float>::quiet_NaN()}) {
        data.fireCost = cost;
        EXPECT_EQ(HUDLayout::SpecialIndicatorCount(data), 0);
    }
    data.fireCost = .00001f;
    EXPECT_EQ(HUDLayout::SpecialIndicatorCount(data), 1024);
}
