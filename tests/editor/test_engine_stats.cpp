// Tests for EngineStats (Statistics panel): FPS and average frame time
// over a rolling one-second window. Plain C++, no ImGui needed.
#include "engine_stats.h"

#include <gtest/gtest.h>

#include <limits>

TEST(EngineStatsTest, NoFramesMeansNoNumbersYet) {
    EngineStats stats;
    EXPECT_EQ(stats.getSampleCount(), 0u);
    EXPECT_FALSE(stats.getFPS().has_value());
    EXPECT_FALSE(stats.getAverageFrameTimeMs().has_value());
}

TEST(EngineStatsTest, IgnoresZeroNegativeAndNonFiniteFrameTimes) {
    EngineStats stats;
    stats.recordFrame(0.0);
    stats.recordFrame(-0.016);
    stats.recordFrame(std::numeric_limits<double>::quiet_NaN());
    stats.recordFrame(std::numeric_limits<double>::infinity());
    EXPECT_EQ(stats.getSampleCount(), 0u);
    EXPECT_FALSE(stats.getFPS().has_value());

    // A bad value after good ones must not disturb the numbers either.
    stats.recordFrame(0.02);
    stats.recordFrame(std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(stats.getSampleCount(), 1u);
    EXPECT_NEAR(*stats.getFPS(), 50.0, 1e-9);
}

TEST(EngineStatsTest, SteadySixtyFpsReportsSixtyAndSixteenMs) {
    EngineStats stats;
    for (int i = 0; i < 240; ++i) {
        stats.recordFrame(1.0 / 60.0);
    }
    ASSERT_TRUE(stats.getFPS().has_value());
    EXPECT_NEAR(*stats.getFPS(), 60.0, 0.01);
    EXPECT_NEAR(*stats.getAverageFrameTimeMs(), 1000.0 / 60.0, 0.01);

    // Only about one second of frames is kept, not all 240.
    EXPECT_GE(stats.getSampleCount(), 59u);
    EXPECT_LE(stats.getSampleCount(), 61u);
}

TEST(EngineStatsTest, FpsAndFrameTimeAgree) {
    EngineStats stats;
    for (int i = 0; i < 30; ++i) {
        stats.recordFrame(i % 2 == 0 ? 0.01 : 0.03);
    }
    // FPS * average frame time (in seconds) is always 1.
    EXPECT_NEAR(*stats.getFPS() * (*stats.getAverageFrameTimeMs() / 1000.0), 1.0, 1e-9);
}

TEST(EngineStatsTest, SingleFrameLongerThanTheWindowIsStillReported) {
    EngineStats stats;
    stats.recordFrame(2.0);
    EXPECT_EQ(stats.getSampleCount(), 1u);
    EXPECT_NEAR(*stats.getFPS(), 0.5, 1e-9);
    EXPECT_NEAR(*stats.getAverageFrameTimeMs(), 2000.0, 1e-9);
}

TEST(EngineStatsTest, OldSlowFrameDropsOutOfTheWindow) {
    EngineStats stats;
    stats.recordFrame(0.5); // one hitch
    for (int i = 0; i < 200; ++i) {
        stats.recordFrame(0.01); // then two seconds at 100 FPS
    }
    // The hitch is more than a second old, so it no longer drags the average.
    EXPECT_NEAR(*stats.getFPS(), 100.0, 0.5);
    EXPECT_NEAR(*stats.getAverageFrameTimeMs(), 10.0, 0.05);
}

TEST(EngineStatsTest, SampleCountIsCappedAt4096) {
    EngineStats stats;
    // 5000 tiny frames add up to far less than one second,
    // so only the sample cap can limit them.
    for (int i = 0; i < 5000; ++i) {
        stats.recordFrame(0.00001);
    }
    EXPECT_EQ(stats.getSampleCount(), 4096u);
    EXPECT_NEAR(*stats.getAverageFrameTimeMs(), 0.01, 1e-6);
}
