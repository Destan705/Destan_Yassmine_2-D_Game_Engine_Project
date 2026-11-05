# Test Index

**Every test should explain what could break and how the test would catch it.**

> **Coverage inventory** for `phase3` @ `ca2b728`. **200 tests in 15 files**, all in one executable (`engine_tests`) and all registered with `ctest`.

| Test file | Code under test | Documented behavior | Tests | Documentation | Coverage |
| --- | --- | --- | --- | --- | --- |
| `tests/rendering/test_renderer.cpp` | Renderer | Create/destroy, pixel-exact clears, rectangles, textures, bad handles | 16 | [test-renderer](test-renderer.md) | Verified |
| `tests/platform/test_platform.cpp` | platform | Init/shutdown safety, each start-up failure path | 7 | [test-platform](test-platform.md) | Verified |
| `tests/platform/test_input.cpp` | input | Key press translated to engine `Key` | 1 | [test-input](test-input.md) | Verified |
| `tests/platform/test_logger.cpp` | logger | Exact text and stream per level | 6 | [test-logger](test-logger.md) | Verified |
| `tests/core/test_engine.cpp` | engine | Quit/close stop the loop; failures return; restartable | 6 | [test-engine](test-engine.md) | Verified |
| `tests/ecs/test_component_entity.cpp` | Entity, Component | ID, add/find/duplicate/owner/update | 6 | [test-component-entity](test-component-entity.md) | Verified |
| `tests/ecs/test_transform_sprite.cpp` | Transform, Sprite | Values, attachment, missing-Transform safety | 12 | [test-transform-sprite](test-transform-sprite.md) | Verified |
| `tests/ecs/test_animation_component.cpp` | AnimationComponent (+ Sprite, Renderer source rectangles) | Validation; real pixels for frame, pause, loop, hold, restart | 10 | [test-animation-component](test-animation-component.md) | Verified |
| `tests/ecs/test_collision.cpp` | ColliderComponent, ColliderSystem | Validation; strict AABB rules; skips; pair order | 28 | [test-collision](test-collision.md) | Verified |
| `tests/assets/test_asset_manager.cpp` | AssetManager, `Renderer::getTextureDimensions` | Cache, path rules, retry, reverse lookup, sizes | 18 | [test-asset-manager](test-asset-manager.md) | Verified |
| `tests/audio/test_audio_service.cpp` | AudioService | Load/cache, limits, playback, no-device fallback | 18 | [test-audio-service](test-audio-service.md) | Verified |
| `tests/events/test_event_dispatcher.cpp` | EventDispatcher | IDs, order, unsubscribe | 3 | [test-event-dispatcher](test-event-dispatcher.md) | Verified |
| `tests/scene/test_scene.cpp` | Scene, SceneManager | IDs, order, ownership, event timing | 25 | [test-scene](test-scene.md) | Verified |
| `tests/scene/test_scene_serializer.cpp` | SceneSerializer | Valid loads, every rejection, safe save, round trip, activation | 31 | [test-scene-serializer](test-scene-serializer.md) | Verified |
| `tests/scene/test_scene_serializer_contract.cpp` | SceneSerializer | Real demo file, error context, colliders, safe replacement | 13 | [test-scene-serializer-contract](test-scene-serializer-contract.md) | Verified |

## How to run

```sh
cmake --build build
ctest --test-dir build --output-on-failure   # all 200
ctest --test-dir build -N                    # list them
```

Run one group with `-R`, for example `ctest --test-dir build -R SceneSerializer`.

**Headless Linux (no display, e.g. a container):** run with `SDL_VIDEO_DRIVER=offscreen`. Even then, 4 `PlatformTest` tests fail there because they need a real OpenGL window; they pass on macOS and Windows.

## Test techniques used

| Technique | Where | Why |
| --- | --- | --- |
| Hidden window + `software` render driver | Renderer, AssetManager, animation, serializer tests | Real drawing without a visible window or GPU |
| Reading pixels back | Renderer, animation tests | Proves what was **drawn**, not just what was stored |
| SDL `dummy` audio driver | Audio tests | Real SDL audio code, no speakers |
| Fresh random temp folder per test | AssetManager, audio, serializer tests | Tests can't affect each other or the repo |
| Handle/ID arithmetic (`first + 1`) | AssetManager, audio tests | Proves the cache skipped a reload without spying |

## Known gaps (from reading the code)

| Area | Missing test |
| --- | --- |
| [game](../code/game.md): `handleCollisions`, `updatePlayerMovement` | No tests (they need no SDL, so they're easy to add) |
| [engine](../code/engine.md#engine) | F5 save / F9 reload paths |
| [getAssetRootPath](../code/platform.md#getassetrootpath), [getPlatformAssetManager](../code/platform.md#getplatformassetmanager) | No direct test |
| [Renderer::DrawTexture](../code/renderer.md#renderer-drawtexture) | Out-of-range or NaN source rectangles |
| [AnimationComponent](../code/animation-component.md) | Leftover time over several small updates |
| [AudioService](../code/audio-service.md) | 2 MB / 8 MB limits |

## Reading a test page

A test page answers: *What is prepared? What action runs? What result is expected? Why does it matter?* For each fixture, also explain setup and cleanup. Include how tests are built or run only when the commands have been verified from CMake or scripts.

**Related:** [Code index](../code/index.md) · [Other project files](../project-files.md)
