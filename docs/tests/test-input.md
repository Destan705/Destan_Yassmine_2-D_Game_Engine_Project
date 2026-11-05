# test_input.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/platform/test_input.cpp`](../../tests/platform/test_input.cpp)

**In one sentence:** It checks that a key-down event in SDL's queue is reported under the engine's own `Key` name.

## Setup and shared helpers

| Member / method | What it does | Why |
| --- | --- | --- |
| `InputTest::SetUp()` | `SDL_Init(SDL_INIT_EVENTS)`, then [`initializeInput()`](../code/input.md#initializeinput) | Only the event system is needed |
| `InputTest::TearDown()` | `SDL_Quit()` | Isolation |
| `KeyPress(SDL_Scancode)` | Pushes a non-repeat `SDL_EVENT_KEY_DOWN` | Simulates a real key press |

## Individual tests

<a id="inputtest-pressedkeyisreportedbyenginekey"></a>
### `InputTest.PressedKeyIsReportedByEngineKey`

| Stage | What happens |
| --- | --- |
| Given | Key A pressed (SDL scancode) |
| When | [`processPlatformEvents()`](../code/input.md#processplatformevents) |
| Then | [`wasKeyPressed(Key::A)`](../code/input.md#waskeypressed) is true and `wasKeyPressed(Key::B)` is false |

**Why:** it proves the SDL-to-engine key translation and the per-frame "pressed" tracking.

## Coverage gaps

No tests for `isKeyHeld`, `wasKeyReleased`, repeat events being ignored, the per-frame reset, or the quit return value (the quit value is covered by [EngineTest](test-engine.md)).

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
