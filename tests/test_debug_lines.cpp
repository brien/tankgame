#include <gtest/gtest.h>
#include "rendering/DebugLineRenderer.h"

TEST(DebugLinesTest, SegmentTransformPreservesEndpointsIncludingNegativeAndZeroComponents) {
    for (const auto& end : {Vector3(-3, 2, 9), Vector3(4, 2, -1), Vector3(4, 2, 3)}) {
        const DebugLineData line{Vector3(4, 2, 3), end, Vector3(1, 1, 1)};
        const Matrix4 model = DebugLineRenderer::BuildModel(line);
        EXPECT_FLOAT_EQ(model(0, 3), 4);
        EXPECT_FLOAT_EQ(model(1, 3), 2);
        EXPECT_FLOAT_EQ(model(2, 3), 3);
        EXPECT_FLOAT_EQ(model(0, 0) + model(0, 3), end.x);
        EXPECT_FLOAT_EQ(model(1, 1) + model(1, 3), end.y);
        EXPECT_FLOAT_EQ(model(2, 2) + model(2, 3), end.z);
    }
}

TEST(DebugLinesTest, MarksOnlyLivePlayerAndEnemyOrigins) {
    TankRenderData player, enemy, dead;
    player.alive = enemy.alive = true;
    player.isPlayer = true;
    enemy.isPlayer = false;
    dead.alive = false;
    enemy.position = Vector3(-4, 2, 7);
    auto lines = DebugLineRenderer::BuildTankMarkers({player, dead, enemy});
    ASSERT_EQ(lines.size(), 6u);
    EXPECT_FLOAT_EQ(lines[3].start.x, -4);
    EXPECT_FLOAT_EQ(lines[3].end.x, -3.25f);
    EXPECT_FLOAT_EQ(lines[4].end.y, 2.75f);
    EXPECT_FLOAT_EQ(lines[5].end.z, 7.75f);
    EXPECT_FLOAT_EQ(lines[3].color.x, 1);
    EXPECT_FLOAT_EQ(lines[4].color.y, 1);
    EXPECT_FLOAT_EQ(lines[5].color.z, 1);
}

TEST(DebugLinesTest, ReusedSceneDropsDebugLines) {
    SceneData scene;
    scene.debugLines.push_back({Vector3(), Vector3(1, 1, 1), Vector3(1, 0, 0)});
    scene.Clear();
    EXPECT_TRUE(scene.debugLines.empty());
}
