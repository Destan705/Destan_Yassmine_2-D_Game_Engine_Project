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
TEST_F(EngineTest, WhenStartupFailsEngineReturnsAndCleansUp) { // name it after the behaviour
    ASSERT_TRUE(SDL_SetHintWithPriority(
        SDL_HINT_VIDEO_DRIVER, "nonexistent-driver",
        SDL_HINT_OVERRIDE));       // force startup to fail: a video driver that doesn't exist
    engine();                      // must RETURN, not hang
    EXPECT_EQ(SDL_WasInit(0), 0u); // proves nothing is still running
}

// Behaviour 2: a quit event stops the loop, and engine() cleans up.
TEST_F(EngineTest, WhenQuitEventOccursEngineStopsAndCleansUp) { // name it after the behaviour
    ASSERT_TRUE(SDL_Init(SDL_INIT_EVENTS))
        << SDL_GetError();         // open the mailbox BEFORE engine() runs
    PushQuit();                    // the letter waits in the mailbox
    engine();                      // first frame reads "quit" and stops
    EXPECT_EQ(SDL_WasInit(0), 0u); // everything shut down again
}

// Startup fails at the window step: SDL starts, but the window can't be made.
// SDL's "dummy" video driver can't create the platform's window
// (the same trick as PlatformTest.WindowCreationFailureCleansUpInitializedSubsystems).
TEST_F(EngineTest, WhenWindowCreationFailsEngineReturnsAndCleansUp) {
    ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "dummy", SDL_HINT_OVERRIDE));
    engine();                      // must return, not loop
    EXPECT_EQ(SDL_WasInit(0), 0u); // SDL was shut down after the window failed
}

// Startup fails at the renderer step: SDL and the window work, the renderer doesn't.
TEST_F(EngineTest, WhenRendererCreationFailsEngineReturnsAndCleansUp) {
    ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "missing-test-renderer",
                                        SDL_HINT_OVERRIDE));
    engine();                      // must return, not loop
    EXPECT_EQ(SDL_WasInit(0), 0u); // window and SDL were both cleaned up
}

// The second close event, "close this window", also stops the engine.
// processPlatformEvents() checks two event types; PushQuit() only covers SDL_EVENT_QUIT.
TEST_F(EngineTest, WhenWindowCloseIsRequestedEngineStopsAndCleansUp) {
    ASSERT_TRUE(SDL_Init(SDL_INIT_EVENTS)) << SDL_GetError(); // open the mailbox first
    SDL_Event event{};
    event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
    ASSERT_TRUE(SDL_PushEvent(&event)) << SDL_GetError();
    engine(); // first frame reads the close request and stops
    EXPECT_EQ(SDL_WasInit(0), 0u);
}

// The engine can start again after quitting, so no leftover state breaks a restart.
TEST_F(EngineTest, EngineCanRunAgainAfterQuitting) {
    for (int run = 0; run < 2; ++run) {
        SCOPED_TRACE(::testing::Message() << "Run " << run + 1); // labels failures by run
        // engine() calls SDL_Quit() at the end, so reopen the mailbox for each run
        ASSERT_TRUE(SDL_Init(SDL_INIT_EVENTS)) << SDL_GetError();
        PushQuit();
        engine();
        EXPECT_EQ(SDL_WasInit(0), 0u);
    }
}

} // namespace