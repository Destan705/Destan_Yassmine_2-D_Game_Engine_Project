# SceneSerializer — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/scene_serializer.h`](../../include/scene_serializer.h) and [`src/scene_serializer.cpp`](../../src/scene_serializer.cpp). Library: nlohmann/json v3.11.3 (fetched by CMake).

**In one sentence:** Reads a version-1 JSON scene file into a brand-new `Scene` (`load`), and writes a `Scene` back to JSON (`save`).

**Why it exists:** It lets levels live in **data files** instead of C++ code, and it's strict on purpose: a bad file is rejected with a message that says **which file, which entity and which field**, instead of half-loading.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | Validating scene files; building scenes from them; saving scenes safely |
| Gets information from | A `.json` file; an [AssetManager](asset-manager.md) (path ↔ handle) |
| Changes | Creates a new `Scene` (load); writes a file (save). It **never** touches the active scene. |
| Used by | [createGameScene](game.md#creategamescene) (start-up), [engine](engine.md#engine) (F5 save, F9 reload), tests |
| Depends on | [Scene](scene.md), [Entity](entity.md), [Transform](transform.md), [Sprite](sprite.md), [AnimationComponent](animation-component.md), [ColliderComponent](collider-component.md), [AssetManager](asset-manager.md), nlohmann/json |

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| The file format | What a valid file looks like | [Format](#file-format) |
| `SceneSerializer` | Stateless load/save class | [Class](#class-sceneserializer) |
| `load(path, AssetManager&)` | File → new `Scene` | [Function](#sceneserializer-load) |
| `save(const Scene&, path, const AssetManager&)` | `Scene` → file | [Function](#sceneserializer-save) |
| `readNumber` / `readFiniteNumber` | Helpers: read one finite number | [Helpers](#helpers) |
| `checkKnownKeys` | Helper: reject unknown fields | [Helpers](#helpers) |

## The file format

<a id="file-format"></a>

```json
{
    "version": 1,
    "name": "Phase 3 Demo",
    "entities": [
        {
            "transform": { "x": 100, "y": 150, "width": 50, "height": 50 },
            "sprite":    { "texture": "test.bmp" },
            "collider":  { "offsetX": 0, "offsetY": 0, "width": 50, "height": 50, "enabled": true },
            "animation": {
                "frames": [ { "x": 0, "y": 0, "w": 32, "h": 32 } ],
                "frameDuration": 0.25,
                "loop": true
            }
        }
    ]
}
```

| Part | Required? | Rules |
| --- | --- | --- |
| `version` | yes | Integer, exactly `1` |
| `name` | yes | String |
| `entities` | yes | Array (may be empty); kept in order |
| `transform` | yes, per entity | Numbers `x`, `y`, `width`, `height`, all finite |
| `sprite` | yes, per entity | `texture`: a path **relative to the AssetManager root** |
| `collider` | optional | Finite `offsetX`, `offsetY`; `width`, `height` > 0; `enabled` true/false (required) |
| `animation` | optional | `frames`: non-empty array of `{x,y,w,h}` with `w`, `h` > 0; `frameDuration` > 0; `loop` true/false |

**Everywhere:** unknown fields and duplicate keys are errors. **Not stored:** entity IDs, texture handles, animation progress, paused state (those are runtime state). The real example is `assets/scenes/phase3-demo.json`.

## Class: `SceneSerializer`

<a id="class-sceneserializer"></a>

**How it's created:** `SceneSerializer serializer;`. It has **no fields**, so one object can be reused for any number of loads and saves.

## Functions

<a id="sceneserializer-load"></a>
### `std::unique_ptr<Scene> SceneSerializer::load(std::filesystem::path filePath, AssetManager& assetManager)`

**What it does:** Validates the whole file while building a **new** scene, and returns it. The caller decides whether to activate it.

| Name | Type | What the caller supplies | Rules |
| --- | --- | --- | --- |
| `filePath` | `std::filesystem::path` | The scene file | Any path (not limited to the asset root) |
| `assetManager` | `AssetManager&` | Turns texture paths into handles | Must outlive the returned scene's use of the handles |

**Returns:** `std::unique_ptr<Scene>`, never null.

**What happens, in order**

1. Opens the file; fails → `Could not open scene file: <path>`.
2. **Parses** with a callback (the `rejectDuplicates` lambda) that keeps one `std::set` of keys per open `{`. A key seen twice in the same object → `<path>: duplicate key '<key>'`. Syntax errors → `<path>: bad JSON syntax: …`.
3. Top level must be an object, with only `version`, `name`, `entities`.
4. Checks `version == 1` (integer), `name` (string), `entities` (array). Creates `Scene(name)`.
5. **For each entity** (`where` = `"<path>: entity <index>"`):
    1. Must be an object with only `transform`, `sprite`, `animation`, `collider`.
    2. `scene->createEntity()`.
    3. **Transform** (required): known keys only; four numbers via [`readNumber`](#helpers).
    4. **Sprite** (required): `texture` string → [`assetManager.loadTexture`](asset-manager.md#assetmanager-loadtexture). Any loader error is re-thrown as `<where> sprite.texture '<path>': <reason>`.
    5. **Collider** (optional): reads and checks the five fields, then adds a [`ColliderComponent`](collider-component.md#collidercomponent-collidercomponent).
    6. **Animation** (optional): reads each frame (`w`, `h` > 0), `frameDuration` > 0, `loop` boolean, then adds an [`AnimationComponent`](animation-component.md#animationcomponent-animationcomponent).
6. Returns the scene.

**Calls other project functions**

| Calls | Why this call is needed here | Where |
| --- | --- | --- |
| `Scene::Scene` / `createEntity` | Build the new scene | [Scene](scene.md#scene-scene), [createEntity](scene.md#scene-createentity) |
| `Entity::addComponent<T>` | Attach each component | [addComponent](entity.md#entity-addcomponent) |
| `AssetManager::loadTexture` | Path → handle (cached) | [loadTexture](asset-manager.md#assetmanager-loadtexture) |
| `ColliderComponent` constructor | Optional hitbox | [Constructor](collider-component.md#collidercomponent-collidercomponent) |
| `AnimationComponent` constructor | Optional animation | [Constructor](animation-component.md#animationcomponent-animationcomponent) |

**Errors:** every problem throws `std::runtime_error` with file and entity context. Nothing is caught and swallowed.

**Failure is all-or-nothing:** the scene being built is a local `unique_ptr`, so on any throw it is destroyed. The caller's active scene is untouched and no `SceneChangedEvent` is sent, because `setActiveScene` is never reached:

```cpp
sceneManager.setActiveScene(serializer.load(path, assets)); // load throws first → nothing changes
```

**Edge cases:**

- Textures loaded **before** the failing entity stay in the AssetManager cache (and the Renderer). That's harmless, and a retry reuses them.
- Frame rectangles are only checked for positive size here, not against the texture size; [`DrawTexture`](renderer.md#renderer-drawtexture) skips out-of-range frames at draw time.
- A number too large for `float` (e.g. `1e100`) becomes infinity and is rejected as "must be finite".

**Related tests:** all of [test_scene_serializer](../tests/test-scene-serializer.md) and [test_scene_serializer_contract](../tests/test-scene-serializer-contract.md).

<a id="sceneserializer-save"></a>
### `void SceneSerializer::save(const Scene& scene, std::filesystem::path filePath, const AssetManager& assetManager)`

**What it does:** Writes `scene` as version-1 JSON (4-space indent), **safely**: the old file is only replaced once the new one is fully written.

**What happens, in order**

1. Writes `version` and `name` (via [`getSceneName`](scene.md#scene-getscenename)).
2. For each entity in [`getEntities()`](scene.md#scene-getentities):
    1. **Transform** required, else `Entity is missing Transform component`.
    2. **Sprite** required, else `Entity is missing Sprite component`. Writes `texture` = [`assetManager.getPath(sprite->getTexture())`](asset-manager.md#assetmanager-getpath) (throws `out_of_range` if the handle didn't come from this AssetManager).
    3. **Collider** if present: offsets, size, `enabled`.
    4. **Animation** if present: `frameDuration`, `loop`, `frames` (from the [getters](animation-component.md#animationcomponent-getframes)).
3. **Safe write:** writes to `<filePath>.tmp`, closes and checks the stream. On failure it deletes the temp file and throws. Then `std::filesystem::rename(tmp, filePath)` replaces the real file in one step. If the rename fails (e.g. the destination is a folder), it deletes the temp file and throws `Could not replace …`.

**Calls:** `Scene::getSceneName`, `Scene::getEntities`, `Entity::getComponent<T>`, `Sprite::getTexture`, `AssetManager::getPath`, the `AnimationComponent` and `ColliderComponent` getters.

**Errors:** `std::runtime_error` (missing component, write/rename failure) or `std::out_of_range` (unknown handle). An entity error happens **before** any file is opened, so the old file is safe.

**Edge cases:** an empty animation `frames` can't happen (the constructor forbids it), so `animData["frames"]` always becomes an array.

## Helpers

<a id="helpers"></a>

All three live in an **anonymous namespace** (private to the `.cpp`).

| Helper | What it does |
| --- | --- |
| `readNumber(object, key, context)` | Throws `<context>: '<key>' is missing or not a number` or `… must be finite`; returns the value as `float` |
| `readFiniteNumber(object, key, context)` | Calls `readNumber` and checks finiteness again (redundant now, kept for the collider code) |
| `checkKnownKeys(object, allowed, context)` | Throws `<context>: unknown field '<key>'` for any key not in `allowed` (catches typos like `"widht"`) |

## Things worth knowing

- **In the game:** the demo is loaded at start-up by [`createGameScene`](game.md#creategamescene); F5 saves to `phase3-demo-saved.json` and F9 loads it, both in [`engine`](engine.md#engine).
- **Saves are scene layout, not save games:** runtime state (animation progress, IDs) is deliberately not stored.
- `load` takes a non-const `AssetManager&` (it may load textures); `save` takes `const` (it only looks up paths).
- The longer, user-facing format description is `docs/scene-format.md` (Phase 3 hand-off docs).

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
