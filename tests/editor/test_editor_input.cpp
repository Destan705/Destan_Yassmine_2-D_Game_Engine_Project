#include "input_capture_state.h"
#include <gtest/gtest.h>

TEST(EditorInputTest, GameplayKeyStartsHeldAndHasOnePressEdge) {
    InputCaptureState state;
    state.beginFrame(false);
    state.keyDown(SDL_SCANCODE_W, false);
    EXPECT_TRUE(state.wasPressed(SDL_SCANCODE_W));
    EXPECT_TRUE(state.isHeld(SDL_SCANCODE_W));
    state.beginFrame(false);
    state.keyDown(SDL_SCANCODE_W, true);
    EXPECT_FALSE(state.wasPressed(SDL_SCANCODE_W));
    EXPECT_TRUE(state.isHeld(SDL_SCANCODE_W));
}

TEST(EditorInputTest, CapturingKeyboardBlocksGameplayButNotEditorShortcuts) {
    InputCaptureState state;
    state.beginFrame(true);
    state.keyDown(SDL_SCANCODE_W, false);
    state.keyDown(SDL_SCANCODE_F1, false);
    EXPECT_FALSE(state.wasPressed(SDL_SCANCODE_W));
    EXPECT_FALSE(state.isHeld(SDL_SCANCODE_W));
    EXPECT_TRUE(state.shortcutPressed(SDL_SCANCODE_F1));
}

TEST(EditorInputTest, CaptureWhileMovementKeyHeldStopsMovementWithoutReleaseEdge) {
    InputCaptureState state;
    state.beginFrame(false);
    state.keyDown(SDL_SCANCODE_W, false);
    state.beginFrame(true);
    EXPECT_FALSE(state.isHeld(SDL_SCANCODE_W));
    EXPECT_FALSE(state.wasPressed(SDL_SCANCODE_W));
    EXPECT_FALSE(state.wasReleased(SDL_SCANCODE_W));
}

TEST(EditorInputTest, LeavingCaptureNeverSynthesizesPressOrHeldMovement) {
    InputCaptureState state;
    state.beginFrame(true);
    state.keyDown(SDL_SCANCODE_A, false);
    state.beginFrame(false);
    EXPECT_FALSE(state.wasPressed(SDL_SCANCODE_A));
    EXPECT_FALSE(state.isHeld(SDL_SCANCODE_A));
    state.keyDown(SDL_SCANCODE_A, true); // SDL repeat from key held in UI.
    EXPECT_FALSE(state.isHeld(SDL_SCANCODE_A));
    state.keyUp(SDL_SCANCODE_A);
    EXPECT_FALSE(state.wasReleased(SDL_SCANCODE_A));
    state.keyDown(SDL_SCANCODE_A, false); // Fresh real press.
    EXPECT_TRUE(state.wasPressed(SDL_SCANCODE_A));
    EXPECT_TRUE(state.isHeld(SDL_SCANCODE_A));
}

TEST(EditorInputTest, ReleaseDuringCaptureDoesNotLeakToGameplay) {
    InputCaptureState state;
    state.beginFrame(false);
    state.keyDown(SDL_SCANCODE_D, false);
    state.setCaptured(true);
    state.keyUp(SDL_SCANCODE_D);
    state.setCaptured(false);
    EXPECT_FALSE(state.isHeld(SDL_SCANCODE_D));
    EXPECT_FALSE(state.wasReleased(SDL_SCANCODE_D));
}

TEST(EditorInputTest, GameplayReleaseIsEmittedOnceOnlyForAcceptedPress) {
    InputCaptureState state;
    state.beginFrame(false);
    state.keyDown(SDL_SCANCODE_D, false);
    state.beginFrame(false);
    state.keyUp(SDL_SCANCODE_D);
    EXPECT_TRUE(state.wasReleased(SDL_SCANCODE_D));
    EXPECT_FALSE(state.isHeld(SDL_SCANCODE_D));
    state.beginFrame(false);
    EXPECT_FALSE(state.wasReleased(SDL_SCANCODE_D));
}

TEST(EditorInputTest, FocusLossClearsHeldKeysAndEdges) {
    InputCaptureState state;
    state.beginFrame(false);
    state.keyDown(SDL_SCANCODE_A, false);
    state.resetGameplay();
    EXPECT_FALSE(state.isHeld(SDL_SCANCODE_A));
    EXPECT_FALSE(state.wasPressed(SDL_SCANCODE_A));
    EXPECT_FALSE(state.wasReleased(SDL_SCANCODE_A));
}

TEST(EditorInputTest, UnknownScancodesAreSafelyIgnored) {
    InputCaptureState state;
    state.beginFrame(false);
    state.keyDown(SDL_SCANCODE_UNKNOWN, false);
    state.keyUp(SDL_SCANCODE_UNKNOWN);
    EXPECT_FALSE(state.wasPressed(SDL_SCANCODE_UNKNOWN));
    EXPECT_FALSE(state.isHeld(SDL_SCANCODE_UNKNOWN));
}
