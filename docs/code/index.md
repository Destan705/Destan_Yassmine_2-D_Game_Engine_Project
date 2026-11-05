# Code Files & Modules

**Find a source file, see its responsibility, and open its documentation.**

> **Coverage inventory** for `phase3` @ `ca2b728` (checked 2026-11-05). Every tracked file under `include/` and `src/` is listed.

| Module / file pair | What it owns or does | Documentation | Coverage |
| --- | --- | --- | --- |
| `src/main.cpp` | Program entry; calls `engine()` | [Engine loop](engine.md#main) | Verified |
| `include/core/engine.h` · `src/core/engine.cpp` | Start-up, engine systems, frame loop, error catching, shutdown | [Engine loop](engine.md) | Verified |
| `include/core/game.h` · `src/core/game.cpp` | Loads the demo scene; player movement; trigger entry | [Game](game.md) | Verified |
| `include/platform/platform.h` · `src/platform/platform.cpp` | SDL start/stop, window, global Renderer and AssetManager, scene drawing, asset root | [Platform](platform.md) | Verified |
| `include/rendering/render_api.h` (implemented in `platform.cpp`) | Phase 1 free drawing functions | [Render API](render-api.md) | Verified |
| `include/platform/input.h` · `src/platform/input.cpp` | Event pump, quit detection, keyboard state | [Input](input.md) | Verified |
| `include/platform/logger.h` · `src/platform/logger.cpp` | Labelled console logging | [Logger](logger.md) | Verified |
| `include/rendering/renderer.h` · `src/rendering/renderer.cpp` | SDL renderer wrapper; owns textures; texture sizes | [Renderer](renderer.md) | Verified |
| `include/rendering/rendertypes.h` | `textureHandle`, `Color`, `Bounds` | [Render types](render-types.md) | Verified |
| `include/ecs/component.h` · `src/ecs/component.cpp` | Component base class | [Component](component.md) | Verified |
| `include/ecs/entity.h` · `src/ecs/entity.cpp` | Entity: owns components | [Entity](entity.md) | Verified |
| `include/ecs/transform.h` | Position and size component | [Transform](transform.md) | Verified |
| `include/ecs/sprite.h` · `src/ecs/sprite.cpp` | Draws a texture at the Transform | [Sprite](sprite.md) | Verified |
| `include/ecs/animationComponent.h` · `src/ecs/animationComponent.cpp` | Sprite-sheet animation | [AnimationComponent](animation-component.md) | Verified |
| `include/ecs/colliderComponent.h` · `src/ecs/colliderComponent.cpp` | Local hitbox + on/off | [ColliderComponent](collider-component.md) | Verified |
| `include/colliderSystem.h` · `src/colliderSystem.cpp` | Finds overlapping entity pairs | [ColliderSystem](collider-system.md) | Verified |
| `include/asset_manager.h` · `src/asset_manager.cpp` | Path ↔ texture-handle cache | [AssetManager](asset-manager.md) | Verified |
| `include/audio_service.h` · `src/audio_service.cpp` | WAV cache and playback | [AudioService](audio-service.md) | Verified |
| `include/events/event_dispatcher.h` · `src/events/event_dispatcher.cpp` | Subscribe/dispatch for `SceneChangedEvent` | [EventDispatcher](event-dispatcher.md) | Verified |
| `include/scene/scene.h` · `src/scene/scene.cpp` | Named set of entities; read-only entity list | [Scene](scene.md) | Verified |
| `include/scene/scene_manager.h` · `src/scene/scene_manager.cpp` | Owns and swaps the active scene | [SceneManager](scene-manager.md) | Verified |
| `include/scene_serializer.h` · `src/scene_serializer.cpp` | JSON scene load/save | [SceneSerializer](scene-serializer.md) | Verified |

### Coverage statuses

- **Not started:** source file found, documentation missing.
- **Draft:** written, but links/behavior are not yet checked against source.
- **Verified:** all documented symbols and behavior checked against the selected revision.

### Not covered here

- Tests: see the [test index](../tests/index.md).
- CMake, CI, formatting and assets: see [other project files](../project-files.md).
- `build/`: generated, so it isn't documented file by file.

**Find a particular method:** [Symbol index](symbol-index.md). **How to write a new page:** [Documentation guide](../contributing.md).
