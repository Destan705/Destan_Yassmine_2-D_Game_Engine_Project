#include "scene_viewport_geometry.h"
#include <gtest/gtest.h>
#include <limits>

TEST(SceneViewportGeometryTest, MapsTopLeftToWorldOrigin) {
    const SceneViewportGeometry area{120, 80, 600, 400, 600, 400};
    const auto position = area.screenToScene(120, 80);
    ASSERT_TRUE(position.has_value());
    EXPECT_FLOAT_EQ(position->first, 0.0f);
    EXPECT_FLOAT_EQ(position->second, 0.0f);
}

TEST(SceneViewportGeometryTest, SubtractsScenePanelScreenOffset) {
    const SceneViewportGeometry area{120, 80, 600, 400, 600, 400};
    const auto position = area.screenToScene(270, 180);
    ASSERT_TRUE(position.has_value());
    EXPECT_FLOAT_EQ(position->first, 150.0f);
    EXPECT_FLOAT_EQ(position->second, 100.0f);
}

TEST(SceneViewportGeometryTest, ScalesForHighDpiRenderTarget) {
    const SceneViewportGeometry area{10, 20, 400, 200, 800, 400};
    const auto position = area.screenToScene(210, 120);
    ASSERT_TRUE(position.has_value());
    EXPECT_FLOAT_EQ(position->first, 400.0f);
    EXPECT_FLOAT_EQ(position->second, 200.0f);
}

TEST(SceneViewportGeometryTest, RejectsPointsOutsideSceneImage) {
    const SceneViewportGeometry area{120, 80, 600, 400, 600, 400};
    EXPECT_FALSE(area.screenToScene(119, 100));
    EXPECT_FALSE(area.screenToScene(120, 79));
    EXPECT_FALSE(area.screenToScene(720, 100));
    EXPECT_FALSE(area.screenToScene(125, 480));
}

TEST(SceneViewportGeometryTest, RejectsMissingOrInvalidDimensions) {
    const SceneViewportGeometry missing{};
    EXPECT_FALSE(missing.screenToScene(0, 0));
    const SceneViewportGeometry noTexture{0, 0, 100, 100, 0, 100};
    EXPECT_FALSE(noTexture.screenToScene(10, 10));
    const SceneViewportGeometry nonfinite{0, 0, 100, 100, 100, 100};
    EXPECT_FALSE(nonfinite.screenToScene(std::numeric_limits<float>::quiet_NaN(), 1));
}
