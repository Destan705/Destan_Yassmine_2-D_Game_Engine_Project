#include "input.h"
#include <SDL3/SDL.h>
#include <array>

SDL_Event event;
const bool* keystate;
std::array<bool, SDL_SCANCODE_COUNT> pressedThisFrame{};
std::array<bool, SDL_SCANCODE_COUNT> releasedThisFrame{};

SDL_Scancode toSDLScancode(Key key) {
    switch (key) {
    case Key::A:
        return SDL_SCANCODE_A;
    case Key::B:
        return SDL_SCANCODE_B;
    case Key::C:
        return SDL_SCANCODE_C;
    case Key::D:
        return SDL_SCANCODE_D;
    case Key::E:
        return SDL_SCANCODE_E;
    case Key::F:
        return SDL_SCANCODE_F;
    case Key::G:
        return SDL_SCANCODE_G;
    case Key::H:
        return SDL_SCANCODE_H;
    case Key::I:
        return SDL_SCANCODE_I;
    case Key::J:
        return SDL_SCANCODE_J;
    case Key::K:
        return SDL_SCANCODE_K;
    case Key::L:
        return SDL_SCANCODE_L;
    case Key::M:
        return SDL_SCANCODE_M;
    case Key::N:
        return SDL_SCANCODE_N;
    case Key::O:
        return SDL_SCANCODE_O;
    case Key::P:
        return SDL_SCANCODE_P;
    case Key::Q:
        return SDL_SCANCODE_Q;
    case Key::R:
        return SDL_SCANCODE_R;
    case Key::S:
        return SDL_SCANCODE_S;
    case Key::T:
        return SDL_SCANCODE_T;
    case Key::U:
        return SDL_SCANCODE_U;
    case Key::V:
        return SDL_SCANCODE_V;
    case Key::W:
        return SDL_SCANCODE_W;
    case Key::X:
        return SDL_SCANCODE_X;
    case Key::Y:
        return SDL_SCANCODE_Y;
    case Key::Z:
        return SDL_SCANCODE_Z;

    case Key::Number1:
        return SDL_SCANCODE_1;
    case Key::Number2:
        return SDL_SCANCODE_2;
    case Key::Number3:
        return SDL_SCANCODE_3;
    case Key::Number4:
        return SDL_SCANCODE_4;
    case Key::Number5:
        return SDL_SCANCODE_5;
    case Key::Number6:
        return SDL_SCANCODE_6;
    case Key::Number7:
        return SDL_SCANCODE_7;
    case Key::Number8:
        return SDL_SCANCODE_8;
    case Key::Number9:
        return SDL_SCANCODE_9;
    case Key::Number0:
        return SDL_SCANCODE_0;

    case Key::Space:
        return SDL_SCANCODE_SPACE;
    case Key::Minus:
        return SDL_SCANCODE_MINUS;
    case Key::Equals:
        return SDL_SCANCODE_EQUALS;
    case Key::LeftBracket:
        return SDL_SCANCODE_LEFTBRACKET;
    case Key::RightBracket:
        return SDL_SCANCODE_RIGHTBRACKET;
    case Key::Backslash:
        return SDL_SCANCODE_BACKSLASH;
    case Key::Semicolon:
        return SDL_SCANCODE_SEMICOLON;
    case Key::Apostrophe:
        return SDL_SCANCODE_APOSTROPHE;
    case Key::Grave:
        return SDL_SCANCODE_GRAVE;
    case Key::Comma:
        return SDL_SCANCODE_COMMA;
    case Key::Period:
        return SDL_SCANCODE_PERIOD;
    case Key::Slash:
        return SDL_SCANCODE_SLASH;

    case Key::Escape:
        return SDL_SCANCODE_ESCAPE;
    case Key::Return:
        return SDL_SCANCODE_RETURN;
    case Key::Backspace:
        return SDL_SCANCODE_BACKSPACE;
    case Key::Tab:
        return SDL_SCANCODE_TAB;
    case Key::CapsLock:
        return SDL_SCANCODE_CAPSLOCK;

    case Key::F1:
        return SDL_SCANCODE_F1;
    case Key::F2:
        return SDL_SCANCODE_F2;
    case Key::F3:
        return SDL_SCANCODE_F3;
    case Key::F4:
        return SDL_SCANCODE_F4;
    case Key::F5:
        return SDL_SCANCODE_F5;
    case Key::F6:
        return SDL_SCANCODE_F6;
    case Key::F7:
        return SDL_SCANCODE_F7;
    case Key::F8:
        return SDL_SCANCODE_F8;
    case Key::F9:
        return SDL_SCANCODE_F9;
    case Key::F10:
        return SDL_SCANCODE_F10;
    case Key::F11:
        return SDL_SCANCODE_F11;
    case Key::F12:
        return SDL_SCANCODE_F12;

    case Key::Insert:
        return SDL_SCANCODE_INSERT;
    case Key::Delete:
        return SDL_SCANCODE_DELETE;
    case Key::Home:
        return SDL_SCANCODE_HOME;
    case Key::End:
        return SDL_SCANCODE_END;
    case Key::PageUp:
        return SDL_SCANCODE_PAGEUP;
    case Key::PageDown:
        return SDL_SCANCODE_PAGEDOWN;
    case Key::Up:
        return SDL_SCANCODE_UP;
    case Key::Down:
        return SDL_SCANCODE_DOWN;
    case Key::Left:
        return SDL_SCANCODE_LEFT;
    case Key::Right:
        return SDL_SCANCODE_RIGHT;

    case Key::LeftControl:
        return SDL_SCANCODE_LCTRL;
    case Key::RightControl:
        return SDL_SCANCODE_RCTRL;
    case Key::LeftShift:
        return SDL_SCANCODE_LSHIFT;
    case Key::RightShift:
        return SDL_SCANCODE_RSHIFT;
    case Key::LeftAlt:
        return SDL_SCANCODE_LALT;
    case Key::RightAlt:
        return SDL_SCANCODE_RALT;
    case Key::LeftGui:
        return SDL_SCANCODE_LGUI;
    case Key::RightGui:
        return SDL_SCANCODE_RGUI;
    }
    return SDL_SCANCODE_UNKNOWN;
}

void initializeInput() { // call after SDL been init in platform.cpp
    keystate = SDL_GetKeyboardState(nullptr);
}

bool processPlatformEvents() {
    pressedThisFrame.fill(false);
    releasedThisFrame.fill(false);

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT or event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            return false;
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            if (!event.key.repeat) {
                pressedThisFrame[event.key.scancode] = true;
            }
        } else if (event.type == SDL_EVENT_KEY_UP) {
            releasedThisFrame[event.key.scancode] = true;
        }
    }
    return true;
}

bool isKeyHeld(Key key) {
    return keystate[toSDLScancode(key)] == true;
}

bool wasKeyPressed(Key key) {
    return pressedThisFrame[toSDLScancode(key)];
}

bool wasKeyReleased(Key key) {
    return releasedThisFrame[toSDLScancode(key)];
}