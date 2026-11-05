# Glossary

Small explanations for terms used in this handbook.

| Term | Easy explanation |
| --- | --- |
| **AABB** | Axis-aligned bounding box: a rectangle that isn't rotated. [`Bounds`](code/render-types.md#struct-bounds) is one, and the [ColliderSystem](code/collider-system.md) compares AABBs. Touching edges don't count as overlapping. |
| **Asset root** | The folder that asset-relative paths start from, e.g. `<executable folder>/assets`. See [AssetManager](code/asset-manager.md) and [getAssetRootPath](code/platform.md#getassetrootpath). |
| **Asset-relative path** | A path like `sprites/cat.bmp`, relative to the asset root. Scene files store these, never handles. |
| **Cache** | Remembering a loaded thing so asking again returns the same one without reloading. [AssetManager](code/asset-manager.md) caches textures, [AudioService](code/audio-service.md) caches sounds. |
| **Collider** | An entity's hitbox: a rectangle relative to its Transform. See [ColliderComponent](code/collider-component.md). |
| **Back buffer** | The hidden image you draw into. `Present()` swaps it onto the screen. |
| **Component** | One piece of an [entity](code/entity.md), like its position ([Transform](code/transform.md)) or picture ([Sprite](code/sprite.md)). Subclass of [`Component`](code/component.md). |
| **Delta time (`dt`)** | **Seconds** since the previous frame. Multiplying speeds by it makes motion frame-rate independent. |
| **Destination rectangle** | **Where on screen** to draw, and how big. Comes from the Transform. |
| **Entity** | A game object: an ID plus a list of components. Owned by a [Scene](code/scene.md). |
| **Event dispatcher** | Code that calls every registered function when something happens. See [EventDispatcher](code/event-dispatcher.md). |
| **Fake** | A test-only replacement for a real thing, so a test doesn't need it. For example, the tests use SDL's `dummy` audio driver and a hidden software-rendered window. See [test_audio_service](tests/test-audio-service.md). |
| **Fixture** | A GoogleTest class whose `SetUp`/`TearDown` run around every test (`TEST_F`). |
| **Frame (animation)** | One picture in an animation, i.e. one source rectangle on the sprite sheet. |
| **Handle** | A number that identifies a resource without exposing it. [`textureHandle`](code/render-types.md#rendertypes-texturehandle) 0 = none. |
| **Listener** | A function registered with the dispatcher. |
| **Owner / owning pointer** | The object responsible for deleting another (`std::unique_ptr`). A **borrowed** pointer or reference must never delete. |
| **RAII** | "Resource acquisition is initialization": the constructor grabs a resource and the destructor frees it, so clean-up happens automatically. |
| **Renderer** | The object that issues drawing commands. Here, [`Renderer`](code/renderer.md) wraps `SDL_Renderer`. |
| **Pimpl** | "Pointer to implementation": the header holds only a pointer to a hidden `Impl` struct, so private details (like SDL types) stay in the `.cpp`. Used by [AudioService](code/audio-service.md#class-audioservice). |
| **Frame boundary** | The end of one loop iteration, after drawing. The safe moment to swap scenes (F9). |
| **Round trip** | Save → load → save again gives the same file. The serializer tests check this. |
| **Scancode** | SDL's number for a physical keyboard key. |
| **Scene** | A named set of entities, such as one level. |
| **Scene file** | A version-1 JSON description of a scene. See [SceneSerializer](code/scene-serializer.md#file-format). |
| **Serializer** | Code that turns objects into a file (save) and back (load). |
| **Source rectangle** | **Which part of the texture image** to draw, in texture pixels. Used for sprite sheets. |
| **Sprite sheet** | One image holding several animation frames side by side. |
| **Trigger** | A collider that makes something happen when the player **enters** it, without blocking movement. The demo's trigger plays a sound. See [handleCollisions](code/game.md#handlecollisions). |
| **Vsync** | Waiting for the screen refresh before showing a frame, which caps the frame rate and stops tearing. |
