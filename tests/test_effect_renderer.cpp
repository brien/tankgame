#include <gtest/gtest.h>
#include "rendering/EffectRenderer.h"
#include "rendering/RenderData.h"

TEST(EffectRendererTest, AllTypesRetainScaleAndTextureSelection) {
    const float scales[][3] = {{2,1,2},{2,1,2},{2,.25f,2},{1,1,1},
                               {2,.5f,2},{2,.3f,2},{1,1,1},{.02f,1,.2f}};
    for (int i = 0; i < 8; ++i) {
        EffectRenderData effect;
        effect.type = static_cast<FxType>(i);
        effect.scale = 2;
        effect.position = Vector3(3,4,5);
        const auto model = EffectRenderer::BuildModel(effect);
        for (int axis = 0; axis < 3; ++axis) EXPECT_FLOAT_EQ(model(axis,axis), scales[i][axis]);
        EXPECT_FLOAT_EQ(model(0,3), 3);
        EXPECT_FLOAT_EQ(model(1,3), 4.2f);
        EXPECT_FLOAT_EQ(model(2,3), 5);
        EXPECT_EQ(EffectRenderer::TextureFor(effect.type),
                  i == 3 ? TEXTURE_HEART : i == 5 ? TEXTURE_STAR : TEXTURE_NAMES_COUNT);
        EXPECT_EQ(EffectRenderer::HasOutline(effect.type), i == 0 || i == 6);
    }
}

TEST(EffectRendererTest, RotationOrderAndNegativeYawPreserveLegacyPose) {
    EffectRenderData effect;
    effect.type = FxType::TYPE_SMOKE;
    effect.scale = 2;
    effect.rotation = Vector3(90,90,90);
    const auto model = EffectRenderer::BuildModel(effect);
    // Rx * Ry(-yaw) * Rz maps (x,y,z) to (-z,y,x).
    EXPECT_NEAR(model(0,2), -2, .00001f);
    EXPECT_NEAR(model(1,1), .25f, .00001f);
    EXPECT_NEAR(model(2,0), 2, .00001f);
    EXPECT_FLOAT_EQ(model(1,3), .2f);
}

TEST(EffectRendererTest, ZeroLifetimeScaleProducesFiniteCollapsedGeometry) {
    EffectRenderData effect;
    effect.scale = 0;
    const auto model = EffectRenderer::BuildModel(effect);
    EXPECT_FLOAT_EQ(model(0,0), 0);
    EXPECT_FLOAT_EQ(model(2,2), 0);
    EXPECT_FLOAT_EQ(model(1,1), 1);
}
