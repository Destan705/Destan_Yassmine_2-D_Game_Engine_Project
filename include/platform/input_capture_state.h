#pragma once

#include <SDL3/SDL_scancode.h>
#include <array>

// Gameplay-only input state. UI keyboard capture never changes editor shortcuts.
// A key held when capture begins must be pressed again after capture ends.
class InputCaptureState {
public:
    void beginFrame(bool captured) {
        pressed.fill(false);
        released.fill(false);
        shortcuts.fill(false);
        setCaptured(captured);
    }

    void setCaptured(bool captured) {
        if (captured && !capture) {
            held.fill(false);
            pressed.fill(false);
            released.fill(false);
        }
        capture = captured;
    }

    void keyDown(SDL_Scancode scancode, bool repeat) {
        if (!valid(scancode)) return;
        auto i = static_cast<std::size_t>(scancode);
        if (!repeat) shortcuts[i] = true;
        if (capture || repeat) return;
        if (!held[i]) pressed[i] = true;
        held[i] = true;
    }

    void keyUp(SDL_Scancode scancode) {
        if (!valid(scancode) || capture) return;
        auto i = static_cast<std::size_t>(scancode);
        if (held[i]) released[i] = true;
        held[i] = false;
    }

    void resetGameplay() {
        held.fill(false);
        pressed.fill(false);
        released.fill(false);
    }

    void reset() {
        resetGameplay();
        shortcuts.fill(false);
        capture = false;
    }

    bool isHeld(SDL_Scancode scancode) const {
        return valid(scancode) && !capture && held[static_cast<std::size_t>(scancode)];
    }
    bool wasPressed(SDL_Scancode scancode) const {
        return valid(scancode) && !capture && pressed[static_cast<std::size_t>(scancode)];
    }
    bool wasReleased(SDL_Scancode scancode) const {
        return valid(scancode) && !capture && released[static_cast<std::size_t>(scancode)];
    }
    bool shortcutPressed(SDL_Scancode scancode) const {
        return valid(scancode) && shortcuts[static_cast<std::size_t>(scancode)];
    }

private:
    static bool valid(SDL_Scancode scancode) {
        const int code = static_cast<int>(scancode);
        return code > 0 && code < SDL_SCANCODE_COUNT;
    }
    std::array<bool, SDL_SCANCODE_COUNT> held{};
    std::array<bool, SDL_SCANCODE_COUNT> pressed{};
    std::array<bool, SDL_SCANCODE_COUNT> released{};
    std::array<bool, SDL_SCANCODE_COUNT> shortcuts{};
    bool capture = false;
};
