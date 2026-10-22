#include <gtest/gtest.h>
#include "rendering/HUDData.h"

TEST(SceneUIOwnershipTest, SnapshotSurvivesCopyAndIsReleasedByLastOwner) {
    std::weak_ptr<const UIRenderData> lifetime;
    SceneData retained;
    {
        SceneData frame;
        auto ui = std::make_unique<UIRenderData>();
        ui->menu.selectedOption = 2;
        frame.uiData = std::move(ui);
        lifetime = frame.uiData;
        retained = frame;
        frame.Clear();
        ASSERT_FALSE(lifetime.expired());
        EXPECT_EQ(retained.uiData->menu.selectedOption, 2);
        EXPECT_FALSE(frame.uiData);
    }
    EXPECT_FALSE(lifetime.expired());
    retained.Clear();
    EXPECT_TRUE(lifetime.expired());
}

TEST(SceneUIOwnershipTest, FrameReplacementReleasesPreviousSnapshot) {
    SceneData scene;
    for (int frame = 0; frame < 1000; ++frame) {
        scene.uiData = std::make_shared<UIRenderData>();
        std::weak_ptr<const UIRenderData> previous = scene.uiData;
        scene = SceneData();
        EXPECT_TRUE(previous.expired());
    }
}
