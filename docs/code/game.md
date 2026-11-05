# Game (demo scene, player, trigger) — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/core/game.h`](../../include/core/game.h) and [`src/core/game.cpp`](../../src/core/game.cpp).

**In one sentence:** The **gameplay side** of the engine: it loads the demo scene from JSON, moves the player with W/A/S/D, and detects when the player **enters** the trigger.

**Why it exists:** It keeps "what this game is" out of the engine loop. [`engine()`](engine.md#engine) only asks for a scene, calls the movement function, and asks whether the trigger was just entered. (In Phase 1 and 2 this file held the cat/chef demo. That code and its `game_tests` were removed in Phase 3.)

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | Loading the demo scene; player movement; trigger-entry detection |
| Gets information from | `assets/scenes/phase3-demo.json`; the keyboard; collision pairs from [ColliderSystem](collider-system.md) |
| Changes | The player's Transform; the caller's `wasOverlapping` flag |
| Used by | [engine](engine.md#engine) |
| Depends on | [SceneSerializer](scene-serializer.md), [AssetManager](asset-manager.md), [platform](platform.md#getassetrootpath), [input](input.md#iskeyheld), [Scene](scene.md), [Entity](entity.md), [Transform](transform.md), [AnimationComponent](animation-component.md) |

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| The demo scene | Who is who in `phase3-demo.json` | [Demo](#the-demo-scene) |
| `createGameScene(AssetManager&)` | Load the demo scene from JSON | [Function](#creategamescene) |
| `updatePlayerMovement(Scene&, float)` | Move the player with W/A/S/D | [Function](#updateplayermovement) |
| `handleCollisions(pairs, Scene&, bool&)` | Did the player **just** enter the trigger? | [Function](#handlecollisions) |

## The demo scene

<a id="the-demo-scene"></a>

`assets/scenes/phase3-demo.json` (loaded through the [SceneSerializer](scene-serializer.md)):

| Entity | Transform (x, y, w×h) | Texture | Extra | Role |
| --- | --- | --- | --- | --- |
| 1 | 100, 150, 50×50 | `test.bmp` | — | Decoration (shares the texture handle with entity 2) |
| 2 | 200, 300, 50×50 | `test.bmp` | Collider 50×50 | **Trigger** (found by position: the 2nd entity) |
| 3 | 300, 150, 32×32 | `anim_sheet.bmp` | 4-frame looping animation; collider 32×32 | **Player** (found as the first entity with an `AnimationComponent`) |

**Roles are decided by convention, not by data:** "the player is the first animated entity" and "the trigger is entity number 2 in the list". Reordering the JSON changes who is who.

## Functions

<a id="creategamescene"></a>
### `std::unique_ptr<Scene> createGameScene(AssetManager& assetManager)`

**What it does:** Loads `getAssetRootPath() / "scenes/phase3-demo.json"` with a local `SceneSerializer` and returns the new scene. The caller ([`SceneManager::setActiveScene`](scene-manager.md#scenemanager-setactivescene)) takes ownership.

| Name | Type | What the caller supplies | Rules |
| --- | --- | --- | --- |
| `assetManager` | `AssetManager&` | Turns the file's texture paths into handles | In the game: [`getPlatformAssetManager()`](platform.md#getplatformassetmanager) |

**Calls other project functions**

| Calls | Why | Where |
| --- | --- | --- |
| `getAssetRootPath` | Find the file next to the executable | [getAssetRootPath](platform.md#getassetrootpath) |
| `SceneSerializer::load` | Read and validate the JSON, build the scene | [load](scene-serializer.md#sceneserializer-load) |

**Errors:** anything `load` throws (missing file, bad JSON, missing texture) passes straight out. `engine()` catches it, logs `Engine error: …` and shuts down.

**Edge case:** it reads the copy in `build/bin/assets/`, which CMake copies at **build** time. After editing `assets/scenes/phase3-demo.json`, rebuild so the copy updates.

<a id="updateplayermovement"></a>
### `void updatePlayerMovement(Scene& scene, float deltaTime)`

**What it does:** Finds the first entity with an `AnimationComponent` (the player) and moves its Transform by `200 px/s × deltaTime` for each held key: **A** left, **D** right, **W** up, **S** down.

**Details:**

- Uses [`isKeyHeld`](input.md#iskeyheld), so it moves every frame the key is down.
- Keys add up, so W + D moves diagonally (and diagonal speed is about 1.41× faster, because the two directions aren't normalised).
- If that entity has no Transform, it returns.
- `playerSpeed` is a local `constexpr float` = 200.

**Called by:** [`engine`](engine.md#engine), **before** `sceneManager.update`. **Calls:** `Scene::getEntities`, `Entity::getComponent<T>`, `isKeyHeld`.

<a id="handlecollisions"></a>
### `bool handleCollisions(const std::vector<ColliderSystem::CollisionPair>& collisionPairs, Scene& scene, bool& wasOverlapping)`

**What it does:** Returns `true` only on the **first frame** the player overlaps the trigger (an "enter" event). Staying inside doesn't fire again; leaving and coming back does.

| Name | Type | What the caller supplies | Rules |
| --- | --- | --- | --- |
| `collisionPairs` | `const std::vector<CollisionPair>&` | This frame's pairs | From [`detectCollisions`](collider-system.md#collidersystem-detectcollisions) |
| `scene` | `Scene&` | The active scene | Used to find player and trigger |
| `wasOverlapping` | `bool&` | Last frame's answer | **Read and updated.** The caller keeps it between frames. |

**What happens, in order**

1. Fewer than 2 entities → sets `wasOverlapping = false`, returns `false`.
2. Finds the player (first entity with an `AnimationComponent`). None → `false` (and resets the flag).
3. The trigger is `entities[1]`. If that's the player → `false`.
4. Looks for a pair containing both IDs, in either order → `currentlyOverlapping`.
5. `justEntered = currentlyOverlapping && !wasOverlapping`.
6. `wasOverlapping = currentlyOverlapping`; returns `justEntered`.

**Why the state lives in the caller:** this keeps the function free of hidden globals. [`engine`](engine.md#engine) owns `wasOverlapping` and resets it to `false` after loading a new scene (F9), because the old scene's overlap means nothing in the new one.

**Calls:** `Scene::getEntities`, `Entity::getComponent<AnimationComponent>`, `Entity::getId`. **No logging here**; `engine()` logs `Player entered trigger!` and plays the sound.

## Things worth knowing

- `game.h` has `#pragma once` and includes `colliderSystem.h` (for `CollisionPair`); it forward-declares `AssetManager`.
- `game.cpp` uses `std::filesystem` without including `<filesystem>` directly (it comes through `platform.h`).
- **No tests** cover these three functions. `handleCollisions` and `updatePlayerMovement` need no SDL, so they would be easy to unit-test with a hand-built `Scene`.

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
