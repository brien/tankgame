#include <gtest/gtest.h>
#include "rendering/TargetingLineRenderer.h"
#include <limits>
#include <cmath>

TEST(TargetingLineTest, PreservesOriginLengthAndRotation) {
    HUDRenderData data; data.playerPosition = Vector3(2, 3, 4);
    data.playerRotation.y = 90;
    const auto model = TargetingLineRenderer::BuildModel(data);
    EXPECT_FLOAT_EQ(model(0,3), 2); EXPECT_FLOAT_EQ(model(1,3), 3.1f);
    EXPECT_FLOAT_EQ(model(2,3), 4); EXPECT_NEAR(model(0,0), 0, .00001f);
    EXPECT_FLOAT_EQ(model(2,2), 32);
}

TEST(TargetingLineTest, StippleUsesViewportPixelsAndExactWrappedNibbles) {
    HUDRenderData data; data.targetingAnimationTime = 0xA53C;
    BasicMaterial material; const int viewport[] = {17, 301, 801, 300};
    const auto mvp = Matrix4::Translation(-.5f, 0, 0) * Matrix4::Scale(1,0,0);
    ASSERT_TRUE(TargetingLineRenderer::BuildMaterial(data, mvp, viewport, material));
    EXPECT_FLOAT_EQ(material.stippleFrame[0], 217.25f);
    EXPECT_FLOAT_EQ(material.stippleFrame[1], 451);
    EXPECT_FLOAT_EQ(material.stippleFrame[2], 1);
    EXPECT_FLOAT_EQ(material.stippleFrame[3], 0);
    for (int i=0; i<4; ++i) EXPECT_EQ(material.stippleBits[i], (0xA53C >> (4*i)) & 15);
    data.targetingAnimationTime = -1;
    ASSERT_TRUE(TargetingLineRenderer::BuildMaterial(data, mvp, viewport, material));
    for (float nibble : material.stippleBits) EXPECT_EQ(nibble, 15);
}

TEST(TargetingLineTest, ClipsBeforePerspectiveDivideAndRejectsInvisibleRays) {
    HUDRenderData data; data.targetingAnimationTime = 65535;
    BasicMaterial material; const int viewport[] = {0,0,800,600};
    const auto projection = Matrix4::Perspective(60, 4.f/3, .1f, 100);
    // Starts behind the eye and crosses the near plane into the visible volume.
    const auto crossing = projection * Matrix4::Translation(0,0,1) * Matrix4::Scale(1,0,-5);
    ASSERT_TRUE(TargetingLineRenderer::BuildMaterial(data, crossing, viewport, material));
    for (float v : material.stippleFrame) EXPECT_TRUE(std::isfinite(v));
    EXPECT_FALSE(TargetingLineRenderer::BuildMaterial(data, projection * Matrix4::Translation(0,0,2), viewport, material));
    EXPECT_FALSE(TargetingLineRenderer::BuildMaterial(data, Matrix4::Scale(0,0,0), viewport, material));
    data.showTargeting = false;
    EXPECT_FALSE(TargetingLineRenderer::BuildMaterial(data, Matrix4::Identity(), viewport, material));
    data.showTargeting = true; data.targetingAnimationTime = std::numeric_limits<float>::infinity();
    EXPECT_FALSE(TargetingLineRenderer::BuildMaterial(data, Matrix4::Identity(), viewport, material));
    data.targetingAnimationTime = 65536;
    EXPECT_FALSE(TargetingLineRenderer::BuildMaterial(data, Matrix4::Identity(), viewport, material));
}
