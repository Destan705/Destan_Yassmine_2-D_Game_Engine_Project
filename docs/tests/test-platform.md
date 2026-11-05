# test_platform.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/platform/test_platform.cpp`](../../tests/platform/test_platform.cpp)

**In one sentence:** It checks that the [platform](../code/platform.md) starts, draws through the old render API, and **always cleans up**, including when start-up fails at each step.

**Why these tests matter:** Leaking a window or leaving SDL running after a failure would break the next start-up and hide bugs.

## Test coverage at a glance

| Test | What it checks | Jump | Code under test |
| --- | --- | --- | --- |
| `CanRenderFrameThroughPublicRenderingApi` | Old API runs without crashing | [Jump](#platformtest-canrenderframethroughpublicrenderingapi) | [beginFrame](../code/platform.md#beginframe), [drawFilledRect](../code/platform.md#drawfilledrect), [drawTexture](../code/platform.md#drawtexture), [presentFrame](../code/platform.md#presentframe) |
| `ShutdownIsSafeBeforeInitialization` | Shutdown without init | [Jump](#platformtest-shutdownissafebeforeinitialization) | [shutdownPlatform](../code/platform.md#shutdownplatform) |
| `ShutdownIsSafeWhenCalledTwice` | Double shutdown | [Jump](#platformtest-shutdownissafewhencalledtwice) | [shutdownPlatform](../code/platform.md#shutdownplatform) |
| `CanInitializeAgainAfterShutdown` | Restart works | [Jump](#platformtest-caninitializeagainaftershutdown) | [initializePlatform](../code/platform.md#initializeplatform) |
| `VideoInitializationFailureReturnsFalseAndAllowsShutdown` | `SDL_Init` failure | [Jump](#platformtest-videoinitializationfailurereturnsfalseandallowsshutdown) | [initializePlatform](../code/platform.md#initializeplatform) |
| `WindowCreationFailureCleansUpInitializedSubsystems` | Window failure clean-up | [Jump](#platformtest-windowcreationfailurecleansupinitializedsubsystems) | [initializePlatform](../code/platform.md#initializeplatform) |
| `RendererCreationFailureCleansUpWindowAndSubsystems` | Renderer failure clean-up and recovery | [Jump](#platformtest-renderercreationfailurecleansupwindowandsubsystems) | [initializePlatform](../code/platform.md#initializeplatform) |

## Setup and shared helpers

### `PlatformTest` fixture

| Member / method | What it does | Why |
| --- | --- | --- |
| `SetUp()` | `shutdownPlatform()`, then `ConfigureDrivers()` | Start from a clean state |
| `TearDown()` | `shutdownPlatform()` and resets the audio/render/video hints | Runs even if a test failed midway |
| `ConfigureDrivers()` | Audio = `dummy`, render = `software`, with `SDL_HINT_OVERRIDE` | CI has no speakers or GPU |

"Cleaned up" is always checked with `SDL_WasInit(0) == 0`, meaning no SDL subsystem is still running.

## Individual tests

<a id="platformtest-canrenderframethroughpublicrenderingapi"></a>
### `PlatformTest.CanRenderFrameThroughPublicRenderingApi`

Initialises, then calls `beginFrame`, `drawFilledRect`, `drawTexture` and `presentFrame`. **Then:** nothing crashes or throws. It's a smoke test with no pixel checks.

<a id="platformtest-shutdownissafebeforeinitialization"></a>
### `PlatformTest.ShutdownIsSafeBeforeInitialization`

Calls `shutdownPlatform()` with nothing started. **Then:** SDL isn't running and there's no crash.

<a id="platformtest-shutdownissafewhencalledtwice"></a>
### `PlatformTest.ShutdownIsSafeWhenCalledTwice`

Init, then shutdown twice. **Then:** no double-free crash, and SDL is stopped.

<a id="platformtest-caninitializeagainaftershutdown"></a>
### `PlatformTest.CanInitializeAgainAfterShutdown`

Init, shutdown, init, shutdown. Both inits succeed. This catches stale globals.

<a id="platformtest-videoinitializationfailurereturnsfalseandallowsshutdown"></a>
### `PlatformTest.VideoInitializationFailureReturnsFalseAndAllowsShutdown`

Video driver = `missing-test-driver`, so `initializePlatform()` returns `false`. Shutdown afterwards is still safe.

<a id="platformtest-windowcreationfailurecleansupinitializedsubsystems"></a>
### `PlatformTest.WindowCreationFailureCleansUpInitializedSubsystems`

Video driver = `dummy` (it can't make the OpenGL window), so the result is `false`. SDL must already be stopped **before** shutdown is called, which proves the failure path cleans up by itself.

<a id="platformtest-renderercreationfailurecleansupwindowandsubsystems"></a>
### `PlatformTest.RendererCreationFailureCleansUpWindowAndSubsystems`

Render driver = `missing-test-renderer`, so the result is `false` and SDL is stopped. Shutdown is still safe (catches a dangling `window` pointer). After resetting the drivers, init succeeds again.

## Running these tests

**Target:** `engine_tests`. **Command:** `ctest --test-dir build -R PlatformTest --output-on-failure`.

**Dependencies:** `assets/test.bmp` must sit next to the test executable, because `initializePlatform` loads it from `SDL_GetBasePath()`. The `tests/CMakeLists.txt` post-build step copies it there.

## Coverage gaps

- No test for [`renderPlatform`](../code/platform.md#renderplatform), [`loadPlatformTexture`](../code/platform.md#loadplatformtexture) or [`getPlatformAssetManager`](../code/platform.md#getplatformassetmanager) directly (the first is exercised only through `EngineTest`).
- No test that `renderPlatform` warns when the renderer is null.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
