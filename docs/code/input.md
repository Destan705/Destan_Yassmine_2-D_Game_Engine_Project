# Input — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/platform/input.h`](../../include/platform/input.h) and [`src/platform/input.cpp`](../../src/platform/input.cpp).

**In one sentence:** Reads SDL events each frame, detects quit/close, and answers "is this key held / was it just pressed / just released?" using the engine's own `Key` names.

**Why it exists:** Game code asks about `Key::A`, not `SDL_SCANCODE_A`, so it doesn't depend on SDL. This file is the only place that translates between the two.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | The SDL event pump, quit detection, keyboard state |
| Gets information from | SDL's event queue and keyboard-state array |
| Changes | Its own per-frame pressed/released arrays |
| Used by | [engine](engine.md#engine) (`processPlatformEvents`, `wasKeyPressed` for F5/F9), [game](game.md#updateplayermovement) (`isKeyHeld` for W/A/S/D), tests |
| Depends on | SDL3 |

## Contents — jump to an explanation

| Symbol | What it means | Jump |
| --- | --- | --- |
| `Key` | Engine key names | [Enum](#input-key) |
| `event`, `keystate`, `pressedThisFrame`, `releasedThisFrame` | Module state | [State](#state-and-types) |
| `toSDLScancode(Key)` | Translate `Key` to SDL | [Function](#tosdlscancode) |
| `initializeInput()` | Grab SDL's keyboard array | [Function](#initializeinput) |
| `processPlatformEvents()` | Pump events; return false on quit | [Function](#processplatformevents) |
| `isKeyHeld(Key)` | Is the key down right now? | [Function](#iskeyheld) |
| `wasKeyPressed(Key)` | Went down this frame? | [Function](#waskeypressed) |
| `wasKeyReleased(Key)` | Came up this frame? | [Function](#waskeyreleased) |

## State and types

<a id="state-and-types"></a>

| Name | Kind / type | Value or initialization | What it means / where used |
| --- | --- | --- | --- |
| <a id="input-key"></a>`Key` | `enum class` | `A`–`Z`, `Number1`–`Number9`, `Number0`, `Space`, `Minus`, `Equals`, `LeftBracket`, `RightBracket`, `Backslash`, `Semicolon`, `Apostrophe`, `Grave`, `Comma`, `Period`, `Slash`, `Escape`, `Return`, `Backspace`, `Tab`, `CapsLock`, `F1`–`F12`, `Insert`, `Delete`, `Home`, `End`, `PageUp`, `PageDown`, `Up`, `Down`, `Left`, `Right`, `LeftControl`, `RightControl`, `LeftShift`, `RightShift`, `LeftAlt`, `RightAlt`, `LeftGui`, `RightGui` | Engine-side key names. Each maps 1:1 to an SDL scancode in [`toSDLScancode`](#tosdlscancode). "Gui" is the Windows/Command key. |
| <a id="input-event"></a>`event` | global `SDL_Event` | zeroed | Scratch space that `SDL_PollEvent` fills |
| <a id="input-keystate"></a>`keystate` | global `const bool*` | uninitialised (null as a global) | SDL's live "is key down" array, indexed by scancode. Set by [`initializeInput`](#initializeinput). Owned by SDL. |
| <a id="input-pressedthisframe"></a>`pressedThisFrame` | `std::array<bool, SDL_SCANCODE_COUNT>` | all `false` | Keys that went down during the latest `processPlatformEvents` |
| <a id="input-releasedthisframe"></a>`releasedThisFrame` | `std::array<bool, SDL_SCANCODE_COUNT>` | all `false` | Keys that came up during the latest `processPlatformEvents` |

All of these are file-level globals with external linkage (not `static` or in an anonymous namespace).

## Functions

<a id="tosdlscancode"></a>
### `SDL_Scancode toSDLScancode(Key key)`

**What it does:** Translates an engine `Key` to the matching SDL scancode with one big `switch`.

**Returns:** the scancode, or `SDL_SCANCODE_UNKNOWN` if the value isn't handled (this can't happen with valid `Key` values).

**Calls other project functions:** **No direct project-function calls.** **Used by:** [`isKeyHeld`](#iskeyheld), [`wasKeyPressed`](#waskeypressed), [`wasKeyReleased`](#waskeyreleased). Not declared in the header, so it's an internal helper.

<a id="initializeinput"></a>
### `void initializeInput()`

**What it does:** Stores SDL's keyboard-state pointer in `keystate`.

**Rules:** Must be called **after** SDL is initialised (the comment says so). [`engine`](engine.md#engine) calls it right after [`initializePlatform`](platform.md#initializeplatform) succeeds.

**Calls other project functions:** **No direct project-function calls.**

<a id="processplatformevents"></a>
### `bool processPlatformEvents()`

**What it does:** Clears the per-frame arrays, then reads every waiting SDL event.

**Returns:** `false` if a quit or window-close event was found (the engine should stop); otherwise `true`.

**What happens, in order**

1. Fills `pressedThisFrame` and `releasedThisFrame` with `false`.
2. Loops `while (SDL_PollEvent(&event))`:
    - `SDL_EVENT_QUIT` or `SDL_EVENT_WINDOW_CLOSE_REQUESTED`: **returns `false` immediately**.
    - `SDL_EVENT_KEY_DOWN`: if it isn't an auto-repeat, marks `pressedThisFrame[scancode] = true`.
    - `SDL_EVENT_KEY_UP`: marks `releasedThisFrame[scancode] = true`.
3. Returns `true`.

**Calls other project functions:** **No direct project-function calls.**

**Side effects:** Empties SDL's event queue (except events after a quit, which stay queued). Updates the pressed/released arrays. SDL also updates `keystate` while pumping events.

**Edge cases:** Holding a key generates repeat key-down events. These are ignored for "pressed", so `wasKeyPressed` is true only on the first frame.

**Used by:** [`engine`](engine.md#engine). **Related tests:** [InputTest](../tests/test-input.md), [EngineTest quit tests](../tests/test-engine.md).

<a id="iskeyheld"></a>
### `bool isKeyHeld(Key key)`

**What it does:** Returns whether `key` is currently down, from SDL's live array.

**Calls:** [`toSDLScancode`](#tosdlscancode).

**Errors:** If [`initializeInput`](#initializeinput) wasn't called, `keystate` is null and this crashes. **Used by:** [`updatePlayerMovement`](game.md#updateplayermovement) (W/A/S/D).

<a id="waskeypressed"></a>
### `bool wasKeyPressed(Key key)`

**What it does:** Returns `pressedThisFrame` for the key. This is true only during the frame the key went down. **Calls:** [`toSDLScancode`](#tosdlscancode). **Related tests:** [PressedKeyIsReportedByEngineKey](../tests/test-input.md#inputtest-pressedkeyisreportedbyenginekey).

<a id="waskeyreleased"></a>
### `bool wasKeyReleased(Key key)`

**What it does:** Returns `releasedThisFrame` for the key. **Calls:** [`toSDLScancode`](#tosdlscancode). **Related tests:** none yet.

## Things worth knowing

- `input.h` has **no `#pragma once`**. Including it twice in one file would redefine `enum class Key` and fail to compile.
- Nothing in the current Phase 3 demo reads keys. Input is pumped only to detect quit.

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
