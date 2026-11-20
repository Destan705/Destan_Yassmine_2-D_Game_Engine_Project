#include "input.h"
#include <SDL3/SDL.h>
#include <gtest/gtest.h>

namespace {
class EditorInputIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(SDL_Init(SDL_INIT_EVENTS)) << SDL_GetError();
        initializeInput();
        SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
    }
    void TearDown() override {
        SDL_Quit();
    }
    void SendKey(Uint32 type, SDL_Scancode scancode, bool repeat = false) {
        SDL_Event event{};
        event.type = type;
        event.key.scancode = scancode;
        event.key.repeat = repeat;
        ASSERT_TRUE(SDL_PushEvent(&event)) << SDL_GetError();
    }
};
}

TEST_F(EditorInputIntegrationTest, EngineReadsPressHeldAndReleaseFromEventQueue) {
    SendKey(SDL_EVENT_KEY_DOWN, SDL_SCANCODE_A);
    EXPECT_TRUE(processPlatformEvents());
    EXPECT_TRUE(wasKeyPressed(Key::A));
    EXPECT_TRUE(isKeyHeld(Key::A));
    EXPECT_TRUE(processPlatformEvents());
    EXPECT_FALSE(wasKeyPressed(Key::A));
    EXPECT_TRUE(isKeyHeld(Key::A));
    SendKey(SDL_EVENT_KEY_UP, SDL_SCANCODE_A);
    EXPECT_TRUE(processPlatformEvents());
    EXPECT_TRUE(wasKeyReleased(Key::A));
    EXPECT_FALSE(isKeyHeld(Key::A));
}

TEST_F(EditorInputIntegrationTest, ShortcutIsTrackedSeparatelyFromGameKeyEdges) {
    SendKey(SDL_EVENT_KEY_DOWN, SDL_SCANCODE_F1);
    ASSERT_TRUE(processPlatformEvents());
    EXPECT_TRUE(wasShortcutPressed(Key::F1));
    ASSERT_TRUE(processPlatformEvents());
    EXPECT_FALSE(wasShortcutPressed(Key::F1));
}

TEST_F(EditorInputIntegrationTest, QuitEventIsNeverIgnored) {
    SDL_Event quit{};
    quit.type = SDL_EVENT_QUIT;
    ASSERT_TRUE(SDL_PushEvent(&quit)) << SDL_GetError();
    EXPECT_FALSE(processPlatformEvents());
}
