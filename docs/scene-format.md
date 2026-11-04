# Scene File Format (version 1)

> **Status:** matches the `phase3` branch. Supported components: Transform, Sprite, Collider, Animation. Sound is **not** stored in scene files.

A scene file is a JSON file that describes **what is in a scene**: its name, and an ordered list of entities with their components. `SceneSerializer::load` turns it into a `Scene`; `SceneSerializer::save` writes a `Scene` back.

Example file: [`assets/scenes/phase3-demo.json`](../assets/scenes/phase3-demo.json).

> **Not wired into startup yet:** the running demo still builds its scene in code (`createGameScene()` in `src/core/game.cpp`). Loading this file at startup is part of the integration work.

## Complete example

```json
{
    "version": 1,
    "name": "Phase 3 Demo",
    "entities": [
        {
            "transform": { "x": 100, "y": 150, "width": 50, "height": 50 },
            "sprite": { "texture": "test.bmp" },
            "collider": { "offsetX": 0, "offsetY": 0, "width": 50, "height": 50, "enabled": true }
        },
        {
            "transform": { "x": 200, "y": 300, "width": 50, "height": 50 },
            "sprite": { "texture": "test.bmp" }
        },
        {
            "transform": { "x": 300, "y": 150, "width": 32, "height": 32 },
            "sprite": { "texture": "anim_sheet.bmp" },
            "animation": {
                "frames": [
                    { "x": 0,  "y": 0, "w": 32, "h": 32 },
                    { "x": 32, "y": 0, "w": 32, "h": 32 },
                    { "x": 64, "y": 0, "w": 32, "h": 32 },
                    { "x": 96, "y": 0, "w": 32, "h": 32 }
                ],
                "frameDuration": 0.25,
                "loop": true
            }
        }
    ]
}
```

Entities 1 and 2 use the same texture path, so the `AssetManager` loads `test.bmp` **once** and both sprites share one texture handle. (The demo file in the repo is the same, minus the collider.)

## Field reference

### Top level

| Field | Type | Required | Meaning |
| --- | --- | --- | --- |
| `version` | integer | yes | Must be exactly `1` |
| `name` | string | yes | Scene name, returned by `Scene::getSceneName()` |
| `entities` | array of objects | yes | Entities in creation order (may be empty) |

### Entity

| Field | Type | Required | Becomes |
| --- | --- | --- | --- |
| `transform` | object | yes | `Transform` |
| `sprite` | object | yes | `Sprite` |
| `collider` | object | no | `ColliderComponent` |
| `animation` | object | no | `AnimationComponent` |

Entity order in the file is the order in the scene. That is also the **draw order** (later entities are drawn on top) and the order collision pairs are reported in. Entity IDs are **not** stored; `Scene::createEntity()` assigns fresh IDs (1, 2, 3, …) on load.

### `transform`

| Field | Type | Required | Units |
| --- | --- | --- | --- |
| `x` | finite number | yes | screen pixels, left edge |
| `y` | finite number | yes | screen pixels, top edge (y grows downward) |
| `width` | finite number | yes | screen pixels |
| `height` | finite number | yes | screen pixels |

### `sprite`

| Field | Type | Required | Meaning |
| --- | --- | --- | --- |
| `texture` | string | yes | BMP path **relative to the assets folder**, e.g. `"test.bmp"` or `"sprites/cat.bmp"` |

### `collider` (optional)

An axis-aligned box **relative to the entity's Transform**. World position is computed every frame as `transform.x + offsetX`, `transform.y + offsetY`, so moving the Transform moves the collider.

| Field | Type | Required | Meaning |
| --- | --- | --- | --- |
| `offsetX`, `offsetY` | finite number | yes | Offset from the Transform's top-left, in pixels |
| `width`, `height` | finite number > 0 | yes | Box size in pixels |
| `enabled` | `true` / `false` | yes | Disabled colliders are ignored by collision detection |

### `animation` (optional)

| Field | Type | Required | Meaning |
| --- | --- | --- | --- |
| `frames` | non-empty array | yes | Source rectangles on the sprite sheet, in play order |
| `frames[i].x`, `.y` | finite number | yes | Top-left of the frame **inside the texture**, in texture pixels |
| `frames[i].w`, `.h` | finite number > 0 | yes | Frame size in texture pixels |
| `frameDuration` | number > 0 | yes | **Seconds per frame** (0.25 = 4 frames per second) |
| `loop` | `true` / `false` | yes | `true`: go back to frame 0; `false`: stop on the last frame |

Note the naming: `transform` and `collider` use `width`/`height`, but frames use `w`/`h` (matching `Bounds`). A frame is cut out of the texture and stretched to the transform's size.

## Path policy

Texture paths go through `AssetManager`, which keeps them portable:

- Paths are **relative to the asset root** given to `AssetManager`. Don't write `assets/` in front.
- `\` becomes `/`, and `./` segments are removed, so `sprites\cat.bmp`, `./sprites/cat.bmp` and `sprites/cat.bmp` are the same texture.
- **Rejected:** empty paths, absolute paths (`/x.bmp`), Windows drive paths (`C:/x.bmp`), rooted paths (`//x`), and any `..` segment.
- Saved files always contain the normalized path, as returned by `AssetManager::getPath`.

## Validation

`load` throws `std::runtime_error` and **returns no scene** when any check fails. Messages name the file, and where possible the entity and field, for example:

```
assets/scenes/level.json: entity 2 collider: 'width' must be greater than zero
```

| Problem | Example | Message contains |
| --- | --- | --- |
| File can't be opened | wrong path | `Could not open scene file` |
| Bad JSON syntax | missing `}` | `bad JSON syntax` |
| Duplicate key in one object | `{"x": 1, "x": 2}` | `duplicate key 'x'` |
| Top level isn't an object | `[ ... ]` | `top level must be an object` |
| Unknown field (typo) | `"widht"`, `"sound"` | `unknown field 'widht'` |
| Missing or wrong version | `"version": 2`, `"version": "1"` | `Unsupported or missing version` |
| Missing or non-text name | `"name": 5` | `Unsupported or missing scene name` |
| `entities` missing or not an array | | `Entities is missing or not an array` |
| Entity isn't an object | `5` | `entity 0: must be an object` |
| Missing component | no `"transform"` / `"sprite"` | `'transform' is missing or not an object` |
| Missing or non-number value | `"x": "100"` | `'x' is missing or not a number` |
| Infinite / NaN value | | `must be finite` |
| Texture not a string | `"texture": 3` | `'texture' is missing or not a string` |
| Texture can't be loaded or path invalid | `"missing.bmp"`, `"../x.bmp"` | `entity N sprite.texture 'missing.bmp': …` |
| Collider size ≤ 0 | `"width": 0` | `'width' must be greater than zero` |
| Collider `enabled` not a boolean | `"enabled": 1` | `'enabled' is missing or not true/false` |
| Empty frame list | `"frames": []` | `'frames' must be a non-empty array` |
| Frame size ≤ 0 | `"w": 0` | `'w' must be greater than zero` |
| Non-positive frame duration | `"frameDuration": 0` | `'frameDuration' must be > 0` |
| `loop` not a boolean | `"loop": "yes"` | `'loop' is missing or not true/false` |

Not checked at load time: whether a frame rectangle fits **inside** its texture. An out-of-bounds frame is skipped at draw time by `Renderer::DrawTexture`, which logs `Invalid source rectangle`.

## Loading a scene

```cpp
#include "scene_serializer.h"

SceneSerializer serializer;
AssetManager assets(renderer, getAssetRootPath()); // <executable folder>/assets

std::unique_ptr<Scene> scene = serializer.load(getAssetRootPath() / "scenes/phase3-demo.json", assets);
sceneManager.setActiveScene(std::move(scene)); // activate only after load succeeded
```

The loader builds a **new, temporary** `Scene`. The serializer never activates scenes and never dispatches events.

- **Success:** `setActiveScene` replaces the old scene and dispatches **one** `SceneChangedEvent`.
- **Failure:** `load` throws before `setActiveScene` runs, so the **old scene stays active** and **no event** is sent. Textures loaded before the failure stay in the `AssetManager` cache.

```cpp
try {
    sceneManager.setActiveScene(serializer.load(path, assets));
} catch (const std::exception& error) {
    logMessage(logger_level::Error, error.what()); // previous scene keeps running
}
```

## Saving a scene

```cpp
serializer.save(sceneManager.getActiveScene(), "my-level.json", assets);
```

What `save` does, in order:

1. Builds JSON from `scene.getEntities()`: each entity's `Transform`, `Sprite` (handle → path with `AssetManager::getPath`), and, if present, `ColliderComponent` and `AnimationComponent`.
2. Writes it (indented) to a temporary file, `<destination>.tmp`.
3. Checks the write succeeded; otherwise deletes the temp file and throws.
4. Replaces the destination with `std::filesystem::rename`.

If any step fails, `save` throws `std::runtime_error` and the **previous destination file is unchanged**.

Requirements: every entity must have a `Transform` and a `Sprite` whose texture was loaded through the same `AssetManager`; otherwise `save` throws.

### What is never saved

| Not saved | Why |
| --- | --- |
| Entity IDs | Recreated by `Scene::createEntity()` on load |
| Texture handles | Runtime numbers; the path is saved instead |
| SDL pointers, listener IDs | Only meaningful while the program runs |
| Collider **world** position | Recomputed from Transform + offset each frame |
| Animation progress (`currentFrame`, `elapsed`, `paused`) | Animations restart from frame 0 on load |
| Audio state | Not part of a scene file |

**Round trip:** `save` → `load` → `save` produces identical JSON, including colliders. Covered by `SceneSerializerTest.SaveLoadSavePreservesMeaningfulValues` and `SceneSerializerContractTest.ColliderSaveLoadSavePreservesMeaningfulValues`.

## Adding things

**A textured object:** an entity with a `transform` and a `sprite`. Put the BMP in `assets/` (or a subfolder) and use its relative path.

**A trigger area:** add a `collider`. It can be a different size from the sprite by using offsets, e.g. a 20×20 box in the middle of a 50×50 sprite: `{"offsetX": 15, "offsetY": 15, "width": 20, "height": 20, "enabled": true}`.

**An animated object:**

1. Make a sprite sheet: all frames in one BMP, e.g. 4 frames of 32×32 side by side → 128×32.
2. Add an `animation` with one `{x, y, w, h}` per frame. For frames side by side, `x` goes up by the frame width: 0, 32, 64, 96.
3. Choose `frameDuration` (seconds per frame) and `loop`.

The same entities in C++, without a file:

```cpp
Entity& e = scene.createEntity();
Transform& t = e.addComponent<Transform>();
t.x = 300; t.y = 150; t.width = 32; t.height = 32;
e.addComponent<Sprite>(assets.loadTexture("anim_sheet.bmp"));

std::vector<Bounds> frames = {{0, 0, 32, 32}, {32, 0, 32, 32}, {64, 0, 32, 32}, {96, 0, 32, 32}};
e.addComponent<AnimationComponent>(frames, 0.25f, true);

e.addComponent<ColliderComponent>(0.0f, 0.0f, 32.0f, 32.0f); // enabled defaults to true
```

## Animation behaviour

`AnimationComponent` chooses **which part of the texture** to draw; `Sprite` and `Renderer` do the drawing.

- **Source vs destination:** the frame `{x, y, w, h}` is the **source** (a rectangle inside the image). The `Transform` is the **destination** (where on screen, how big).
- **Timing:** each `update(dt)` adds `dt` (seconds) to `elapsed`. While `elapsed >= frameDuration`, it moves to the next frame and subtracts `frameDuration`, so leftover time carries over and one long frame can skip several animation frames.
- **End of clip:** `loop: true` wraps to frame 0; `loop: false` holds the last frame.
- **Controls:** `pause()`, `resume()`, `restart()`. Paused time is not counted.
- **Independent:** two entities can share one sprite sheet and show different frames, because each has its own `AnimationComponent`.
- Each `update` passes the current frame to the sibling `Sprite` with `Sprite::sourceSet`. Updates run before rendering, so the first drawn frame is already frame 0.

## Not supported in version 1

- **Sound:** audio is played by `AudioService`, not stored in scene files.
- **Entities without a sprite:** `sprite` is required on every entity.
- **Multiple colliders or animations per entity:** at most one of each component type.
