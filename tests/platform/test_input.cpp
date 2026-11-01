
#include "input.h"
#include <SDL3/SDL.h>
#include <gtest/gtest.h>

namespace {

class InputTest : public ::testing::Test {
  protected:
    void SetUp() override {
        ASSERT_TRUE(SDL_Init(SDL_INIT_EVENTS))
            << SDL_GetError(); // the flag for ONLY the event system
        initializeInput();
    }

    void TearDown() override {
        SDL_Quit();
    }

    // Simulates a real key press by putting a key-down event in SDL's queue.
    void KeyPress(SDL_Scancode scancode) { // SDL's type for a key code, not Key
        SDL_Event event{};                 // start every field at zero
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.scancode = scancode; // the parameter, no translation
        event.key.repeat = false;      // a fresh press, not a held-key repeat
        ASSERT_TRUE(SDL_PushEvent(&event)) << SDL_GetError(); // the ADDRESS of the event
    }
};

TEST_F(InputTest, PressedKeyIsReportedByEngineKey) { // name it after the behaviour
    KeyPress(SDL_SCANCODE_A);                        // press A, in SDL's language
    processPlatformEvents();                         // let the input system read the queue

    EXPECT_TRUE(wasKeyPressed(Key::A));  // ask about A, in the engine's language
    EXPECT_FALSE(wasKeyPressed(Key::B)); // a key you did NOT press
}

} // namespace