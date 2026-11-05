# test_engine.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/core/test_engine.cpp`](../../tests/core/test_engine.cpp)

**In one sentence:** It runs the whole [`engine()`](../code/engine.md#engine) and checks that it stops on quit or close, returns on start-up failure, and always shuts SDL down.

**Why these tests matter:** A broken loop could hang forever or leave SDL running. These tests catch both.

## Test coverage at a glance

| Test | What it checks | Jump |
| --- | --- | --- |
| `WhenStartupFailsEngineReturnsAndCleansUp` | Bad video driver | [Jump](#enginetest-whenstartupfailsenginereturnsandcleansup) |
| `WhenQuitEventOccursEngineStopsAndCleansUp` | `SDL_EVENT_QUIT` | [Jump](#enginetest-whenquiteventoccursenginestopsandcleansup) |
| `WhenWindowCreationFailsEngineReturnsAndCleansUp` | Window failure | [Jump](#enginetest-whenwindowcreationfailsenginereturnsandcleansup) |
| `WhenRendererCreationFailsEngineReturnsAndCleansUp` | Renderer failure | [Jump](#enginetest-whenrenderercreationfailsenginereturnsandcleansup) |
| `WhenWindowCloseIsRequestedEngineStopsAndCleansUp` | Window close event | [Jump](#enginetest-whenwindowcloseisrequestedenginestopsandcleansup) |
| `EngineCanRunAgainAfterQuitting` | Two runs in a row | [Jump](#enginetest-enginecanrunagainafterquitting) |

## Setup and shared helpers

### `EngineTest` fixture

| Member / method | What it does | Why |
| --- | --- | --- |
| `SetUp()` | `shutdownPlatform()`; audio = `dummy`, render = `software` | Clean start; CI has no hardware |
| `TearDown()` | `shutdownPlatform()`; resets the three hints | Isolation |
| `PushQuit()` | Puts an `SDL_EVENT_QUIT` in SDL's queue | Makes the first frame stop the loop |

**Trick used:** to push an event, SDL's event system must be running, so tests call `SDL_Init(SDL_INIT_EVENTS)` **before** `engine()`.

## Individual tests

<a id="enginetest-whenstartupfailsenginereturnsandcleansup"></a>
### `EngineTest.WhenStartupFailsEngineReturnsAndCleansUp`

Video driver `nonexistent-driver`. `engine()` returns (doesn't hang), and `SDL_WasInit(0) == 0`.

<a id="enginetest-whenquiteventoccursenginestopsandcleansup"></a>
### `EngineTest.WhenQuitEventOccursEngineStopsAndCleansUp`

Queues a quit event, then runs `engine()`. The first [`processPlatformEvents`](../code/input.md#processplatformevents) returns false, the loop ends and SDL is shut down. **This also runs the full start-up**: the platform AssetManager, the `AudioService` (dummy driver), loading `audio/test.wav`, and [`createGameScene`](../code/game.md#creategamescene), which loads `assets/scenes/phase3-demo.json` from the test executable's `assets/` copy.

<a id="enginetest-whenwindowcreationfailsenginereturnsandcleansup"></a>
### `EngineTest.WhenWindowCreationFailsEngineReturnsAndCleansUp`

Video driver `dummy`, so window creation fails. It returns, and SDL is stopped.

<a id="enginetest-whenrenderercreationfailsenginereturnsandcleansup"></a>
### `EngineTest.WhenRendererCreationFailsEngineReturnsAndCleansUp`

Render driver `missing-test-renderer`. It returns, and SDL is stopped.

<a id="enginetest-whenwindowcloseisrequestedenginestopsandcleansup"></a>
### `EngineTest.WhenWindowCloseIsRequestedEngineStopsAndCleansUp`

Queues `SDL_EVENT_WINDOW_CLOSE_REQUESTED` instead of quit, to cover the second branch of the quit check.

<a id="enginetest-enginecanrunagainafterquitting"></a>
### `EngineTest.EngineCanRunAgainAfterQuitting`

Runs init-events, quit, `engine()` twice. Both runs end cleanly, which catches leftover global state.

## Running these tests

**Target:** `engine_tests`. **Command:** `ctest --test-dir build -R EngineTest --output-on-failure`.

**Dependencies:** `gtest_discover_tests` sets `WORKING_DIRECTORY` to the **project root**, so the relative paths `assets/test.bmp` and `assets/anim_sheet.bmp` used by [`engine()`](../code/engine.md#engine) resolve. Running the test executable from another folder would make the quit tests fail.

## Coverage gaps

- Nothing checks what is **drawn** or that the demo entities exist.
- No test for an asset that is missing during scene set-up (now caught and logged by `engine()`, but not checked).
- No test for player movement, the trigger sound, or F5/F9 save/reload.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
