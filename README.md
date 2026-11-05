
# Destan & Yassmine's 2D Game Engine

A small 2D game engine built by **Destan and Yassmine** using **C++20 and SDL3**. We're building it to learn how game engines work, practise software architecture, and collaborate on a C++ codebase.

We're currently working on **Phase 3**, which brings together data-driven scenes, texture caching, animation, collision detection, and audio into a playable demo.

## Current Features

- **Core engine:** Initialization, a game loop, delta-time-based updates, rendering, and clean shutdown.
- **Input:** Keyboard support for held, newly pressed, and released keys.
- **Entities and components:** Scenes contain Entities with components such as `Transform`, `Sprite`, `AnimationComponent`, and `ColliderComponent`.
- **Scene management:** `SceneManager` owns the active Scene and dispatches scene-change events when it is replaced.
- **Rendering and textures:** SDL3 rendering, BMP textures, sprite-sheet source rectangles, and an `AssetManager` that caches textures by asset-relative path.
- **Animation:** Timed frames, looping and non-looping playback, pause, resume, and restart.
- **Collision detection:** Rectangle (AABB) overlap detection with local collider offsets. This detects overlaps; it does not perform physics or collision resolution.
- **Audio:** Short WAV sound effects through `AudioService`.
- **JSON scenes:** Load and save version-1 scene files containing supported components, with validation and error reporting.
- **Testing:** GoogleTest tests and GitHub Actions for builds, static analysis, and automated testing.

## Phase 3 Demo

The demo opens an **800 × 600** window and loads `assets/scenes/phase3-demo.json`. It shows three Entities, including an animated player. Two Entities share the same texture through the asset cache.

Use the keyboard to move the player into the stationary trigger. A sound plays **once when the overlap begins**, not every frame. Leave and enter again to trigger it another time.

| Control | Action |
| --- | --- |
| **W / A / S / D** | Move the animated player |
| **F5** | Save the current Scene, including the player's position |
| **F9** | Reload the saved Scene |
| **Close window** | Exit the engine |

**Saving and reloading:** F5 writes `scenes/phase3-demo-saved.json` inside the **executable's copied `assets` folder**. F9 loads that saved file. If you haven't saved yet, F9 displays a warning. Reloading creates fresh component runtime state (including animation playback) and clears the previous Scene's overlap state.

The original `phase3-demo.json` is left unchanged. The demo also keeps running if an optional sound cannot be played; an invalid required scene prevents startup, or prevents replacement during reload, with an error message.

## Requirements

- A **C++20-compatible compiler** (for example, MinGW-w64 `g++`, Clang, or MSVC)
- **CMake 3.20** or newer
- **Git**
- A compatible build tool (for example, `mingw32-make`, Make, Ninja, or Visual Studio)

CMake downloads **SDL3**, **GoogleTest**, and **nlohmann/json** during the initial configuration, so you need an internet connection the first time. Linux may also require SDL3 system development packages; see the [CI workflow](.github/workflows/ci.yml) for the Ubuntu dependencies used by this project.

## Build and Run

### 1. Clone the repository

Until the integration work is merged into `phase3`, use the integration branch:

```sh
git clone --branch feature/phase3-integration https://github.com/Destan705/Destan_Yassmine_Cool_Amazing_Project.git
cd Destan_Yassmine_Cool_Amazing_Project
```

After the integration branch is merged, you can use `--branch phase3` instead.

### 2. Windows — MinGW

**First-time setup (run once for a new build folder):**

```cmd
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++
```

This selects MinGW explicitly. Without `-G "MinGW Makefiles"`, CMake may try to use **NMake**, which will fail if Visual Studio's NMake tools are not installed.

Make sure `g++` and `mingw32-make` are available on your `PATH`.

**Build the project (first time and after code changes):**

```cmd
cmake --build build
```

**Run the game:**

```cmd
.\build\bin\game_engine.exe
```

**For later builds, you normally only need** `cmake --build build`. You do **not** need to repeat the full configuration command every time.

If `build/` was previously configured with a different generator (such as NMake), delete the existing **generated `build/` directory** first, then rerun the first-time MinGW configuration command. Only delete it if you do not need anything stored inside; this does not delete your source code. CMake cannot switch generators in an existing build cache.

### 3. Linux / macOS

**First-time configuration:**

```sh
cmake -S . -B build
```

**Build:**

```sh
cmake --build build
```

**Run:**

```sh
./build/bin/game_engine
```

For subsequent builds, you normally only need `cmake --build build`.

### Asset location

CMake copies the `assets` directory beside the game executable after building. The engine locates assets relative to the **executable**, not the terminal's working directory, so you can also launch it from another folder using the executable's full path.

Keep the copied `assets` directory alongside the executable when moving or sharing the built game.

## Run Tests

On **Windows (MinGW)**:

```cmd
ctest --test-dir build --output-on-failure
```

On **Linux / macOS**:

```sh
ctest --test-dir build --output-on-failure
```

For a multi-configuration generator such as Visual Studio, add `-C Debug` (or the configuration you built).

Some tests use SDL's window and rendering systems. Headless Linux environments need a virtual display; the CI workflow uses Xvfb.

## Project Structure

| Path | Contents |
| --- | --- |
| `src/core/` | Engine loop and demo gameplay |
| `src/platform/` | SDL setup, input, platform integration, and logging |
| `src/rendering/` | Renderer and texture drawing |
| `src/ecs/` | Entities and components |
| `src/scene/` | Scenes and active Scene management |
| `src/` | AssetManager, audio, collision system, and scene serialization |
| `include/` | Public headers and interfaces |
| `assets/` | BMP textures, WAV audio, and JSON scene files |
| `tests/` | GoogleTest suites for the engine and subsystems |
| `.github/workflows/` | Continuous integration |

## Collaboration

We develop features on separate branches, review each other's changes, and integrate them into the current phase branch before merging into `main`.
