# Destan & Yassmine's 2D Game Engine

A small 2D game engine built by **Destan and Yassmine** using **C++20 and SDL3**. We're building this project to learn how game engines work, practise software architecture, and collaborate on a C++ codebase.

The project is currently in **Phase 3**: data-driven scenes, cached resources and sprite animation, built on the Phase 2 scene and component system.

## Current Features

- Engine loop with input, update and rendering stages, using delta time in seconds.
- Scenes made of entities and components (`Transform`, `Sprite`, `AnimationComponent`), managed by `SceneManager` with a `SceneChangedEvent`.
- BMP textures owned by the `Renderer`; `AssetManager` caches them by asset-relative path, so a shared image is loaded once.
- Sprite-sheet animation: each animated sprite draws one source rectangle of its texture, with looping or hold-last-frame.
- Scene files in JSON (version 1): `SceneSerializer` loads with full validation and saves safely through a temporary file. See [docs/scene-format.md](docs/scene-format.md).
- Keyboard input abstraction for held, newly pressed and released keys.
- Console logging with info, warning and error levels.
- GoogleTest tests and GitHub Actions for builds, static analysis and tests.

Not built yet: collision, audio, and loading the demo from its JSON file at startup. See [docs/phase3-decisions.md](docs/phase3-decisions.md).

The current demo opens an 800 × 600 window with three sprites: two share one texture, and one plays a four-frame animation. Close the window to exit. Run it from the repository root, because the demo's texture paths are relative to the working directory.

## Requirements

- A C++20-compatible compiler
- CMake 3.20 or newer
- Git
- A build tool supported by CMake, such as Make, Ninja, or Visual Studio

CMake downloads SDL3 and GoogleTest automatically during configuration, so an internet connection is required for the first build. Linux also requires SDL's system development dependencies; see [the CI workflow](.github/workflows/ci.yml) for the Ubuntu package list.

## Build and Run

Clone the repository (use the branch you want to try, for example `phase2`):

```sh
git clone --branch phase2 https://github.com/Destan705/Destan_Yassmine_Cool_Amazing_Project.git
cd Destan_Yassmine_Cool_Amazing_Project
```

Configure and build:

```sh
cmake -S . -B build
cmake --build build --config Debug
```

Run on Linux/macOS:

```sh
./build/bin/game_engine
```

Run on Windows with a single-configuration generator, such as MinGW Makefiles or Ninja:

```powershell
.\build\bin\game_engine.exe
```

With a multi-configuration generator, such as Visual Studio, the executable is typically at `build/bin/Debug/game_engine.exe`.

Assets are copied beside the executable during the build. Keep the `assets` folder alongside it when moving the executable.

## Using the Engine

Load a scene from a file and make it active:

```cpp
SceneSerializer serializer;
AssetManager assets(renderer, "assets");   // texture paths are relative to this folder

try {
    sceneManager.setActiveScene(serializer.load("assets/scenes/phase3-demo.json", assets));
} catch (const std::exception& error) {
    logMessage(logger_level::Error, error.what()); // the previous scene keeps running
}
```

Add a textured, animated entity in code:

```cpp
Entity& entity = sceneManager.getActiveScene().createEntity();

Transform& transform = entity.addComponent<Transform>();
transform.x = 300; transform.y = 150; transform.width = 32; transform.height = 32;

entity.addComponent<Sprite>(assets.loadTexture("anim_sheet.bmp"));

std::vector<Bounds> frames = {{0, 0, 32, 32}, {32, 0, 32, 32}, {64, 0, 32, 32}, {96, 0, 32, 32}};
entity.addComponent<AnimationComponent>(frames, 0.25f, true); // 0.25 s per frame, looping
```

Save the active scene:

```cpp
serializer.save(sceneManager.getActiveScene(), "assets/scenes/my-level.json", assets);
```

The full file format, validation rules and path rules are in [docs/scene-format.md](docs/scene-format.md).

## Run Tests

```sh
ctest --test-dir build -C Debug --output-on-failure
```

Some tests initialize SDL's window and rendering systems. Headless Linux environments need a virtual display; the CI workflow includes an Xvfb setup.

## Project Structure

| Folder | Contents |
| --- | --- |
| `src/` | Engine, platform, input, rendering, and logging implementations |
| `include/` | Headers and public interfaces |
| `assets/` | Demo textures and `scenes/` (JSON scene files) |
| `docs/` | Scene format, Phase 3 decisions and verification |
| `tests/` | GoogleTest tests |
| `.github/workflows/` | Continuous integration |

## Collaboration

Development happens on feature branches. Changes are reviewed through pull requests into the current phase branch (for example `phase2`) before reaching `main`.
