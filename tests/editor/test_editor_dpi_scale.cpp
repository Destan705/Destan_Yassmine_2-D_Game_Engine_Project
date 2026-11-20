#include <gtest/gtest.h>
#include "editor_dpi_scale.h"
#include <limits>

TEST(EditorDpiTest, NormalScaleIsUnchanged) {
    EXPECT_FLOAT_EQ(EditorDpi::normalizedScale(1.0f), 1.0f);
}

TEST(EditorDpiTest, TwoHundredPercentScaleIsSupported) {
    EXPECT_FLOAT_EQ(EditorDpi::normalizedScale(2.0f), 2.0f);
}

TEST(EditorDpiTest, OneHundredFiftyPercentScaleIsSupported) {
    EXPECT_FLOAT_EQ(EditorDpi::normalizedScale(1.5f), 1.5f);
}

TEST(EditorDpiTest, RejectsInvalidScaleWithoutJumpingToOne) {
    EXPECT_FLOAT_EQ(EditorDpi::normalizedScale(0.0f, 2.0f), 2.0f);
    EXPECT_FLOAT_EQ(EditorDpi::normalizedScale(std::numeric_limits<float>::quiet_NaN(), 1.5f), 1.5f);
    EXPECT_FLOAT_EQ(EditorDpi::normalizedScale(std::numeric_limits<float>::infinity(), 1.5f), 1.5f);
}

TEST(EditorDpiTest, RejectsInvalidPriorScaleToo) {
    EXPECT_FLOAT_EQ(EditorDpi::normalizedScale(-1.0f, 0.0f), 1.0f);
}

TEST(EditorDpiTest, CapsExtremeScales) {
    EXPECT_FLOAT_EQ(EditorDpi::normalizedScale(0.2f), 0.75f);
    EXPECT_FLOAT_EQ(EditorDpi::normalizedScale(8.0f), 4.0f);
}

TEST(EditorDpiTest, DetectsDisplayChangeWithoutNoise) {
    EXPECT_FALSE(EditorDpi::scaleChanged(2.0f, 2.005f));
    EXPECT_TRUE(EditorDpi::scaleChanged(2.0f, 1.0f));
    EXPECT_TRUE(EditorDpi::scaleChanged(0.0f, 2.0f));
}

TEST(EditorDpiTest, CompactToolbarTracksDpi) {
    EXPECT_FALSE(EditorDpi::compactToolbar(900.0f, 1.0f));
    EXPECT_TRUE(EditorDpi::compactToolbar(500.0f, 1.0f));
    EXPECT_FALSE(EditorDpi::compactToolbar(1574.0f, 2.0f));
    EXPECT_TRUE(EditorDpi::compactToolbar(1000.0f, 2.0f));
}
