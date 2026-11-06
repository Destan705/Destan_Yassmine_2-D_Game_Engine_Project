#include "platform.h"
#include "render_api.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

namespace {

class PlatformTest : public ::testing::Test {
  protected:
    void SetUp() override {
        shutdownPlatform();
        ConfigureDrivers();
    }

    void TearDown() override {
        // Runs even when a test assertion fails partway through initialization.
        shutdownPlatform();
        SDL_ResetHint(SDL_HINT_AUDIO_DRIVER);
        SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
        SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    }

    void ConfigureDrivers() {
        // CI has no speakers; drawing tests do not need a physical GPU either.
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_AUDIO_DRIVER, "dummy", SDL_HINT_OVERRIDE));
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE));
    }
};

TEST_F(PlatformTest, CanRenderFrameThroughPublicRenderingApi) {
    ASSERT_TRUE(initializePlatform()) << SDL_GetError();

    beginFrame();
    drawFilledRect(Color{255, 0, 0, 255}, Bounds{10.0f, 20.0f, 50.0f, 50.0f});
    drawTexture();
    presentFrame();
}

TEST_F(PlatformTest, ShutdownIsSafeBeforeInitialization) {
    shutdownPlatform();
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

TEST_F(PlatformTest, ShutdownIsSafeWhenCalledTwice) {
    ASSERT_TRUE(initializePlatform()) << SDL_GetError();
    shutdownPlatform();
    shutdownPlatform();
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

TEST_F(PlatformTest, CanInitializeAgainAfterShutdown) {
    ASSERT_TRUE(initializePlatform()) << SDL_GetError();
    shutdownPlatform();
    EXPECT_EQ(SDL_WasInit(0), 0u);

    ConfigureDrivers();
    ASSERT_TRUE(initializePlatform()) << SDL_GetError();
    shutdownPlatform();
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

TEST_F(PlatformTest, VideoInitializationFailureReturnsFalseAndAllowsShutdown) {
    ASSERT_TRUE(
        SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "missing-test-driver", SDL_HINT_OVERRIDE));
    EXPECT_FALSE(initializePlatform());
    shutdownPlatform();
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

TEST_F(PlatformTest, WindowCreationFailureCleansUpInitializedSubsystems) {
    // SDL's dummy video backend cannot create the OpenGL window used by the platform.
    ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "dummy", SDL_HINT_OVERRIDE));
    EXPECT_FALSE(initializePlatform());
    // Check cleanup BEFORE calling shutdown, so missing failure-path cleanup is caught.
    EXPECT_EQ(SDL_WasInit(0), 0u);
    shutdownPlatform();
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

TEST_F(PlatformTest, RendererCreationFailureCleansUpWindowAndSubsystems) {
    ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "missing-test-renderer",
                                        SDL_HINT_OVERRIDE));
    EXPECT_FALSE(initializePlatform());
    EXPECT_EQ(SDL_WasInit(0), 0u);
    // Catches a dangling window pointer left behind by the failure path.
    shutdownPlatform();
    EXPECT_EQ(SDL_WasInit(0), 0u);

    // Startup can recover once the failing renderer setting is removed.
    ConfigureDrivers();
    ASSERT_TRUE(initializePlatform()) << SDL_GetError();
}

} // namespace
