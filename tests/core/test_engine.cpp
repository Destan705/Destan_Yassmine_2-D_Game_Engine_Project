#include "engine.h"
#include "platform.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

namespace {

class EngineTest : public ::testing::Test {
  protected:
    void SetUp() override {
        shutdownPlatform(); // start clean, like your friend's PlatformTest
        // CI has no speakers or GPU: same two hints as her ConfigureDrivers()
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_AUDIO_DRIVER, "dummy", SDL_HINT_OVERRIDE));
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE));
    }

    void TearDown() override {
        shutdownPlatform();
        // undo EVERY hint that SetUp() or a test might have changed (there are three)
        SDL_ResetHint(SDL_HINT_AUDIO_DRIVER);
        SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
        SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    }

    // Puts a "quit" letter in SDL's mailbox, like KeyPress did for keys.
    void PushQuit() {
        SDL_Event event{};
        event.type = SDL_EVENT_QUIT; // the type that makes processPlatformEvents() return false
        ASSERT_TRUE(SDL_PushEvent(&event)) << SDL_GetError();
    }
};

// Behaviour 1: if startup fails, engine() returns instead of looping, and cleans up.
TEST_F(EngineTest, WhenStartupFailsEngineReturnsAndCleansUp) {
    ASSERT_TRUE(SDL_SetHintWithPriority(
        SDL_HINT_VIDEO_DRIVER, "nonexistent-driver",
        SDL_HINT_OVERRIDE));
    engine();
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

// Behaviour 2: a quit event stops the loop, and engine() cleans up.
TEST_F(EngineTest, WhenQuitEventOccursEngineStopsAndCleansUp) {
    ASSERT_TRUE(SDL_Init(SDL_INIT_EVENTS)) << SDL_GetError();
    PushQuit();
    engine();
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

// The old "WhenWindowCreationFailsEngineReturnsAndCleansUp" test was removed.
// It relied on SDL's dummy video driver being unable to create an OpenGL window.
// With the SDL_Renderer-compatible window in Phase 4, the dummy video driver
// can create a window, so calling engine() without a quit event would hang.
// To test the window-creation failure path again, introduce a deterministic
// test seam for SDL_CreateWindow rather than relying on a specific driver.

// Startup fails at the renderer step: SDL and the window work, the renderer doesn't.
TEST_F(EngineTest, WhenRendererCreationFailsEngineReturnsAndCleansUp) {
    ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "missing-test-renderer",
                                        SDL_HINT_OVERRIDE));
    engine();
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

// The second close event, "close this window", also stops the engine.
TEST_F(EngineTest, WhenWindowCloseIsRequestedEngineStopsAndCleansUp) {
    ASSERT_TRUE(SDL_Init(SDL_INIT_EVENTS)) << SDL_GetError();
    SDL_Event event{};
    event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
    ASSERT_TRUE(SDL_PushEvent(&event)) << SDL_GetError();
    engine();
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

// The engine can start again after quitting, so no leftover state breaks a restart.
TEST_F(EngineTest, EngineCanRunAgainAfterQuitting) {
    for (int run = 0; run < 2; ++run) {
        SCOPED_TRACE(::testing::Message() << "Run " << run + 1);
        ASSERT_TRUE(SDL_Init(SDL_INIT_EVENTS)) << SDL_GetError();
        PushQuit();
        engine();
        EXPECT_EQ(SDL_WasInit(0), 0u);
    }
}

} // namespace
