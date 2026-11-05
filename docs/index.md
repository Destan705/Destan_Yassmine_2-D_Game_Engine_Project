# The Game Engine Handbook

**A guided tour of the code: what it does, how its parts connect, and where to find everything.**

> **Status:** Verified against `phase3` @ `ca2b728` (2026-11-05). Covers all of Phase 3: source rectangles, animation, AssetManager, scene serializer, collision, audio, and the integrated demo (JSON loading, player movement, trigger sound, F5/F9). 200 tests, all in `ctest`.

## Choose where to start

| I want to… | Go to |
| --- | --- |
| Understand the big picture | [Architecture overview](architecture/index.md) |
| Follow one engine frame | [Frame flow](architecture/frame-flow.md) |
| Know who owns and deletes what | [Ownership and lifetime](architecture/ownership.md) |
| Find a class, function, field, or constant | [Symbol index](code/symbol-index.md) |
| Look up a `.h`/`.cpp` pair | [Code file index](code/index.md) |
| Understand a test | [Test index](tests/index.md) |
| Find CMake, CI, assets, or `build/` | [Other project files](project-files.md) |
| Learn project terms | [Glossary](glossary.md) |
| Add or change documentation | [Documentation guide](contributing.md) |

## Running the demo

```sh
cmake --build build && ./build/bin/game_engine
```

**W/A/S/D** move the animated player · walk into the second square (the **trigger**) to hear the sound · **F5** saves the scene · **F9** reloads the save. The scene comes from `assets/scenes/phase3-demo.json`.

## Quick how-tos (using real API names)

**Add an object:** add an entity to `assets/scenes/phase3-demo.json` (no C++ needed), then rebuild so the copy next to the executable updates:

```json
{
  "transform": { "x": 500, "y": 100, "width": 40, "height": 40 },
  "sprite":    { "texture": "test.bmp" },
  "collider":  { "offsetX": 0, "offsetY": 0, "width": 40, "height": 40, "enabled": true }
}
```

**The same in C++:**

```cpp
Entity& e = scene.createEntity();
Transform& t = e.addComponent<Transform>();
t.x = 500; t.y = 100; t.width = 40; t.height = 40;
e.addComponent<Sprite>(getPlatformAssetManager().loadTexture("test.bmp"));
e.addComponent<ColliderComponent>(0.0f, 0.0f, 40.0f, 40.0f);
```

**Animate it** (sprite sheet with 4 frames of 32×32, 0.25 s each, looping):

```cpp
std::vector<Bounds> frames = {{0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}};
e.addComponent<AnimationComponent>(frames, 0.25f, true);
```

**Load a scene safely** (the old scene stays if loading throws):

```cpp
sceneManager.setActiveScene(serializer.load(path, getPlatformAssetManager()));
```

See [the demo scene](code/game.md#the-demo-scene), [SceneSerializer](code/scene-serializer.md#file-format), [ColliderComponent](code/collider-component.md) and [AudioService](code/audio-service.md).

## How to read a code page

Each code page tells you:

1. **Purpose**: what that file or class is responsible for.
2. **Contents**: classes, fields, constants and functions, with clickable links.
3. **Behavior**: what each function receives, does, changes and returns.
4. **Connections**: what calls it, what it calls, *why*, and where to read those other functions.
5. **Things to watch**: ownership, errors, assumptions, edge cases, tests.

## Issues found while documenting

These are **observations**, not changes. The code wasn't modified.

| # | Issue | Where |
| --- | --- | --- |
| 1 | Player and trigger are found **by convention** (first animated entity; entity number 2), so reordering the JSON silently changes who is who | [game](code/game.md#the-demo-scene) |
| 2 | Diagonal movement is about 1.41× faster (W+D aren't normalised) | [game](code/game.md#updateplayermovement) |
| 3 | No tests for `handleCollisions`, `updatePlayerMovement`, F5/F9 or `getPlatformAssetManager` | [test index](tests/index.md#known-gaps-from-reading-the-code) |
| 4 | "Invalid texture source rectangle" warning logs every frame (no warn-once) | [Renderer](code/renderer.md#renderer-drawtexture) |
| 5 | `engine.h`, `input.h`, `render_api.h` lack `#pragma once`; `entity.h` uses `std::logic_error` without `<stdexcept>` | code pages |
| 6 | `typeid(*component)` warning on every clang build | [Entity](code/entity.md#entity-getcomponent) |
| 7 | CI doesn't check `clang-format`; some Phase 3 files aren't formatted | [project files](project-files.md) |
| 8 | `.DS_Store` is committed | [project files](project-files.md) |
| 9 | `readFiniteNumber` repeats a check `readNumber` already does | [SceneSerializer](code/scene-serializer.md#helpers) |
| 10 | The old render API and `loadPlatformTexture` are unused by the game | [Render API](code/render-api.md) |

**Fixed since the last check (`6749cec`):** `game_tests` no longer exists (so nothing is silently skipped); `engine()` now catches exceptions and always shuts down; `AnimationComponent` and source rectangles now have tests; the demo now loads from JSON through the AssetManager, so it runs from any folder; the trigger actually fires.

## The main idea

> Explain the code as though someone else will maintain it next month. Describe what the implementation actually does, not what its author hoped it would do.
