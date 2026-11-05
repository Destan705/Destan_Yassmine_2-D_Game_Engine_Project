# Platform — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/platform/platform.h`](../../include/platform/platform.h) and [`src/platform/platform.cpp`](../../src/platform/platform.cpp). This `.cpp` also implements the functions declared in [`render_api.h`](render-api.md).

**In one sentence:** Starts and stops SDL, owns the window, the one engine [`Renderer`](renderer.md) and the one engine [`AssetManager`](asset-manager.md), and gives the engine simple functions to draw a scene and load textures.

**Why it exists:** It keeps window/SDL set-up and tear-down in one place, so the engine loop ([`engine`](engine.md#engine)) never touches SDL objects directly. It also guarantees clean-up on every failure path.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | SDL start/stop, window, the global `Renderer`, frame drawing entry points |
| Gets information from | The engine (scene to draw, texture paths) |
| Changes | Global `window`, `renderer`, `assetManager`, `currentTexture`; the screen |
| Used by | [engine](engine.md) (start/stop, drawing, asset root, AssetManager), [game](game.md) (asset root), tests |
| Depends on | SDL3, [Renderer](renderer.md), [logger](logger.md), [Scene](scene.md) |

## Contents — jump to an explanation

| Symbol | What it means | Jump |
| --- | --- | --- |
| `window` | The SDL window (global) | [State](#platform-window) |
| `renderer` | The engine's Renderer (global) | [State](#platform-renderer) |
| `assetManager` | The engine's AssetManager (global) | [State](#platform-assetmanager) |
| `currentTexture` | Texture loaded at start-up for the old render API | [State](#platform-currenttexture) |
| `initializePlatform()` | Start SDL, window, renderer | [Function](#initializeplatform) |
| `shutdownPlatform()` | Destroy everything, quit SDL | [Function](#shutdownplatform) |
| `renderPlatform(Scene&)` | Draw one frame of a scene | [Function](#renderplatform) |
| `loadPlatformTexture(const char*)` | Load a texture via the global renderer | [Function](#loadplatformtexture) |
| `getAssetRootPath()` | `<executable folder>/assets` | [Function](#getassetrootpath) |
| `getPlatformAssetManager()` | Access the engine's AssetManager | [Function](#getplatformassetmanager) |
| `beginFrame()` | Old API: clear to black | [Function](#beginframe) |
| `drawFilledRect(Color, Bounds)` | Old API: draw rectangle | [Function](#drawfilledrect) |
| `drawTexture()` | Old API: draw start-up texture | [Function](#drawtexture) |
| `presentFrame()` | Old API: present | [Function](#presentframe) |

## State and types

<a id="state-and-types"></a>

These are **file-level globals** (not `static`, so they have external linkage).

| Name | Kind / type | Value or initialization | What it means / where used |
| --- | --- | --- | --- |
| <a id="platform-window"></a>`window` | global `SDL_Window*` | `nullptr` | The game window. Created in [`initializePlatform`](#initializeplatform), destroyed in [`shutdownPlatform`](#shutdownplatform) or on a failure path. Owning raw pointer. |
| <a id="platform-renderer"></a>`renderer` | global `std::unique_ptr<Renderer>` | `nullptr` | The engine's only [`Renderer`](renderer.md#class-renderer). All drawing goes through it. `nullptr` means "not initialised", and most functions check this. |
| <a id="platform-assetmanager"></a>`assetManager` | global `std::unique_ptr<AssetManager>` | `nullptr` | The engine's path ↔ handle cache, rooted at [`getAssetRootPath()`](#getassetrootpath). Created at the end of `initializePlatform`, destroyed **first** in `shutdownPlatform` (it borrows the Renderer). |
| <a id="platform-currenttexture"></a>`currentTexture` | global `textureHandle` | `0` | Handle of `assets/test.bmp`, loaded during start-up. Only used by the Phase 1 [`drawTexture`](#drawtexture). Not reset by shutdown. |

## Functions

<a id="initializeplatform"></a>
### `bool initializePlatform()`

**What it does:** Starts SDL video (required) and audio (optional), opens an 800×600 window, creates the [`Renderer`](renderer.md#renderer-renderer), loads `assets/test.bmp`, and creates the [`AssetManager`](asset-manager.md#assetmanager-assetmanager).

**Why/when it's used:** First thing in [`engine`](engine.md#engine). Tests call it directly.

**Returns:** `bool`. `true` means everything is ready; `false` means start-up failed and **everything was cleaned up already**.

**What happens, in order**

1. `SDL_Init(SDL_INIT_VIDEO)`. If it fails, it logs an Error, calls `SDL_Quit()` and returns `false`.
2. `SDL_Init(SDL_INIT_AUDIO)`. If it fails, it only logs a **Warning** (`… (continuing without sound) …`) and carries on. The game still runs on a machine without a sound device; [`AudioService`](audio-service.md) then reports itself unavailable.
3. Logs `SDL3 initialized successfully!`.
4. `SDL_CreateWindow("My SDL3 Window", 800, 600, SDL_WINDOW_OPENGL)`. If it fails, it logs an Error, calls `SDL_Quit()` and returns `false`.
5. Logs `Window created successfully!`.
6. Inside `try`:
    1. Creates the renderer with `std::make_unique<Renderer>(window)`.
    2. Gets the executable folder with `SDL_GetBasePath()`. If null, it throws.
    3. Builds `<basePath>assets/test.bmp` and loads it into `currentTexture`.
    4. Creates `assetManager = std::make_unique<AssetManager>(*renderer, getAssetRootPath())`.
7. `catch (const std::exception&)`: logs the message, calls [`shutdownPlatform()`](#shutdownplatform) (AssetManager → renderer → window → `SDL_Quit`) and returns `false`.
8. Returns `true`.

**Calls other project functions**

| Calls | Why this call is needed here | Where to find it |
| --- | --- | --- |
| `logMessage` | Report each step's success or failure | [logMessage](logger.md#logmessage) |
| `Renderer::Renderer` (via `make_unique`) | Create the drawing object | [Renderer constructor](renderer.md#renderer-renderer) |
| `Renderer::LoadTexture` | Load the start-up texture for `drawTexture` | [LoadTexture](renderer.md#renderer-loadtexture) |
| `shutdownPlatform` | Undo everything on a failure inside the `try` | [shutdownPlatform](#shutdownplatform) |

**Side effects:** Starts SDL, opens a window, sets the three globals, and logs.

**Errors and edge cases:**

- Every failure path undoes what was already done, so `SDL_WasInit(0) == 0` after a `false` return. Tests check this.
- Any `std::exception` is caught (Phase 2 caught only `runtime_error`). Because the failure path now calls `shutdownPlatform`, it also logs `Engine is shutting down`.
- Audio failure is **not** a start-up failure.
- A missing `assets/test.bmp` next to the executable makes start-up fail.

**Used by:** [`engine`](engine.md#engine).

**Related tests:** [PlatformTest](../tests/test-platform.md) (all), [EngineTest](../tests/test-engine.md) failure tests.

<a id="shutdownplatform"></a>
### `void shutdownPlatform()`

**What it does:** Destroys the AssetManager, then the renderer, then the window, then quits SDL.

**Why/when it's used:** At the end of [`engine`](engine.md#engine). Tests also call it before and after each test.

**What happens, in order**

1. `assetManager.reset()`: the AssetManager borrows the Renderer, so it must go first.
2. `renderer.reset()` runs [`~Renderer`](renderer.md#renderer-destructor), which frees textures and the SDL renderer.
3. If `window` isn't null, it destroys it and sets it to `nullptr`.
4. Logs `Engine is shutting down`.
5. `SDL_Quit()`.

**Calls other project functions**

| Calls | Why | Where |
| --- | --- | --- |
| `Renderer::~Renderer` (via `reset`) | Free GPU resources **before** the window disappears | [Destructor](renderer.md#renderer-destructor) |
| `logMessage` | Announce shutdown | [logMessage](logger.md#logmessage) |

**Errors and edge cases:** Safe to call before initialisation and safe to call twice. It logs "Engine is shutting down" every time, even if nothing was running. It doesn't reset `currentTexture`.

**Related tests:** [ShutdownIsSafeBeforeInitialization](../tests/test-platform.md#platformtest-shutdownissafebeforeinitialization), [ShutdownIsSafeWhenCalledTwice](../tests/test-platform.md#platformtest-shutdownissafewhencalledtwice).

<a id="renderplatform"></a>
### `void renderPlatform(Scene& scene)`

**What it does:** Draws one full frame of `scene`: it clears to white, lets the scene draw itself, then presents.

**Why/when it's used:** Every frame, via [`renderEngineFrame`](engine.md#renderengineframe). This is how the platform layer **delegates drawing to the active scene**.

| Name | Type | What the caller supplies | Rules |
| --- | --- | --- | --- |
| `scene` | `Scene&` | The scene to draw | Borrowed, not owned. Must be alive for the call. |

**What happens, in order**

1. If `renderer` is null, it logs Warning `Renderer is not initialized!` and returns.
2. `renderer->ClearScreen(Color{255,255,255,255})` clears to white.
3. `scene.render(*renderer)` hands every entity the renderer.
4. `renderer->Present()`.

**Calls other project functions**

| Calls | Why | Where |
| --- | --- | --- |
| `Renderer::ClearScreen` | Remove last frame | [ClearScreen](renderer.md#renderer-clearscreen) |
| `Scene::render` | Let entities/components draw themselves | [Scene::render](scene.md#scene-render) |
| `Renderer::Present` | Show the frame | [Present](renderer.md#renderer-present) |
| `logMessage` | Warn if called too early | [logMessage](logger.md#logmessage) |

<a id="loadplatformtexture"></a>
### `textureHandle loadPlatformTexture(const char* filePath)`

**What it does:** Loads a BMP through the global renderer and returns its handle.

**Why/when it's used:** Nothing in the game uses it any more (the demo now loads from JSON through the AssetManager). Kept for tests and quick experiments.

| Name | Type | What the caller supplies | Rules |
| --- | --- | --- | --- |
| `filePath` | `const char*` | Path to a BMP | **Passed straight through.** A relative path is relative to the **current working directory**. |

**Returns:** the new handle, or `0` if the renderer isn't initialised (and logs a Warning).

**Calls other project functions:** [`Renderer::LoadTexture`](renderer.md#renderer-loadtexture) to do the actual load, and [`logMessage`](logger.md#logmessage) for the warning.

**Errors and edge cases:**

- **Exceptions are not caught here.** A missing file throws `std::runtime_error` to the caller.
- **Path behaviour differs from `initializePlatform`:** `initializePlatform` prefixes `SDL_GetBasePath()` (the executable's folder), but this function does **not**. The demo works when launched from the repository root (`./build/bin/game_engine`), because `assets/` exists there. Launched from another folder, it fails.

<a id="getassetrootpath"></a>
### `std::filesystem::path getAssetRootPath()`

**What it does:** Returns `<executable folder>/assets`, using `SDL_GetBasePath()`.

**Why/when it's used:** It's the root for the platform's [`AssetManager`](#platform-assetmanager), the [`AudioService`](audio-service.md#audioservice-audioservice), the demo scene file ([`createGameScene`](game.md#creategamescene)) and the F5 save file ([`engine`](engine.md#engine)). So everything is found next to the executable, wherever the program is launched from.

**Returns:** the path, or an **empty path** if SDL can't find the base path (it logs a Warning `Unable to fetch base path : …`). With an empty root, later loads look relative to the working directory.

**Calls:** [`logMessage`](logger.md#logmessage). **Needs:** SDL to be usable (`SDL_GetBasePath` works before `SDL_Init` in SDL3). **Tests:** none directly.

<a id="getplatformassetmanager"></a>
### `AssetManager& getPlatformAssetManager()`

**What it does:** Returns the engine's one [`AssetManager`](asset-manager.md).

**Why it exists:** The AssetManager needs the Renderer, which only the platform has. This function shares the cache without exposing the Renderer. Using **one** AssetManager everywhere means the scene loaded at start-up and a scene reloaded with F9 share the same texture handles, and `save` can turn every handle back into a path.

**Errors:** throws `std::runtime_error("Platform asset manager is not initialized.")` before `initializePlatform` succeeds or after `shutdownPlatform`.

**Returns:** a reference valid until `shutdownPlatform()`. Don't keep it past that.

**Used by:** [`engine`](engine.md#engine) (start-up scene, F5, F9). **Tests:** none directly.

<a id="beginframe"></a>
### `void beginFrame()` (render API, Phase 1)

**What it does:** Clears the screen to **black** through the global renderer. If the renderer is null, it logs a Warning and returns.

**Calls:** [`Renderer::ClearScreen`](renderer.md#renderer-clearscreen), [`logMessage`](logger.md#logmessage). **Used by:** tests only ([CanRenderFrameThroughPublicRenderingApi](../tests/test-platform.md#platformtest-canrenderframethroughpublicrenderingapi)). The current engine loop uses [`renderPlatform`](#renderplatform) instead.

<a id="drawfilledrect"></a>
### `void drawFilledRect(Color rectColor, Bounds rectBounds)` (render API, Phase 1)

**What it does:** Forwards to [`Renderer::DrawFilledRect`](renderer.md#renderer-drawfilledrect). The comment says "center of the screen", but it actually draws wherever `rectBounds` says.

**Errors:** **There is no null check.** Calling it before `initializePlatform` dereferences a null `unique_ptr` (a crash).

**Used by:** tests only (the Phase 1 `renderGame` that used it was removed in Phase 3).

<a id="drawtexture"></a>
### `void drawTexture()` (render API, Phase 1)

**What it does:** Draws the start-up texture ([`currentTexture`](#platform-currenttexture)) at x=100, y=100, 200×200, via [`Renderer::DrawTexture`](renderer.md#renderer-drawtexture).

**Errors:** No null check. **Used by:** tests only.

<a id="presentframe"></a>
### `void presentFrame()` (render API, Phase 1)

**What it does:** Forwards to [`Renderer::Present`](renderer.md#renderer-present). No null check. **Used by:** tests only.

## Dependencies and interactions

```mermaid
flowchart LR
    Engine -->|initializePlatform / shutdownPlatform| Platform
    Engine -->|renderPlatform scene| Platform
    Engine -->|getAssetRootPath, getPlatformAssetManager| Platform
    Platform -->|owns| AM[AssetManager]
    AM -.->|borrows| Renderer
    Platform -->|owns| Renderer
    Platform -->|scene.render| Scene
    Platform --> SDL[SDL window + init]
```

**Includes that matter:** `platform.h` includes `scene.h` (so callers can pass a `Scene&`), `rendertypes.h` and `<filesystem>`. It forward-declares `Renderer` and `AssetManager`, so `platform.h` doesn't drag SDL into its users.

## Things worth knowing

- **Assumptions:** [`initializePlatform`](#initializeplatform) must succeed before the drawing functions are used.
- **Surprising behavior:** the old render API (`beginFrame`, `drawFilledRect`, `drawTexture`, `presentFrame`) is still compiled and tested, but the engine loop doesn't use it.
- **Paths:** the game now uses the executable folder everywhere (`getAssetRootPath`), so it runs from any working directory. Only the unused `loadPlatformTexture` still takes working-directory paths.
- `platform.cpp` uses `std::filesystem` through `platform.h`.

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
