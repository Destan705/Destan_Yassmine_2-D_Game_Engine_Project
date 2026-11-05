# SceneManager — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/scene/scene_manager.h`](../../include/scene/scene_manager.h) and [`src/scene/scene_manager.cpp`](../../src/scene/scene_manager.cpp).

**In one sentence:** Owns the one active [`Scene`](scene.md), swaps it for a new one when asked, announces the change, and updates it each frame.

**Why it exists:** The engine loop doesn't need to know which scene is running. It just asks the manager.

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `SceneManager` | Owner of the active scene | [Class](#class-scenemanager) |
| `SceneManager(EventDispatcher&)` | Create, borrowing a dispatcher | [Function](#scenemanager-scenemanager) |
| `setActiveScene(unique_ptr<Scene>)` | Replace the scene and announce it | [Function](#scenemanager-setactivescene) |
| `getActiveScene()` | Access the scene | [Function](#scenemanager-getactivescene) |
| `update(float)` | Update the active scene | [Function](#scenemanager-update) |

## Class: `SceneManager`

<a id="class-scenemanager"></a>

**How it's created and destroyed:** On the stack in [`engine`](engine.md#engine). Its destructor destroys the active scene.

**Important rules:** the referenced `EventDispatcher` must outlive the manager. At most one scene is active.

### Fields

| Field | Type / initial value | Meaning | Who changes it |
| --- | --- | --- | --- |
| <a id="scenemanager-currentscene"></a>`currentScene` | `std::unique_ptr<Scene>`, null | The active scene. **Owning.** | `setActiveScene` |
| <a id="scenemanager-eventdispatcher"></a>`eventDispatcher` | `EventDispatcher&` | Where change events go. **Borrowed** (a reference member, so it's set in the initializer list). | Constructor |

## Functions

<a id="scenemanager-scenemanager"></a>
### `explicit SceneManager::SceneManager(EventDispatcher& dispatcher)`

Binds the reference. A reference member **must** be initialised in the member initializer list. **No direct project-function calls.**

<a id="scenemanager-setactivescene"></a>
### `void SceneManager::setActiveScene(std::unique_ptr<Scene> scene)`

**What it does:** Makes `scene` the active one, destroys the previous one, then dispatches a `SceneChangedEvent`.

| Name | Type | Meaning | Rules |
| --- | --- | --- | --- |
| `scene` | `std::unique_ptr<Scene>` | The new scene; ownership moves in | Must not be null |

**What happens, in order**

1. If `scene` is null, it throws `std::invalid_argument("Scene cannot be null")`. Nothing changes and no event is sent.
2. Copies the old name (or `""` if none).
3. Copies the new name.
4. `currentScene = std::move(scene)`. **The old scene is destroyed here.**
5. Dispatches `SceneChangedEvent{oldName, newName}`.

**Calls other project functions**

| Calls | Why | Where |
| --- | --- | --- |
| `Scene::getSceneName` | Copy names for the event, before the old scene dies | [getSceneName](scene.md#scene-getscenename) |
| `EventDispatcher::dispatch` | Announce the change | [dispatch](event-dispatcher.md#eventdispatcher-dispatch) |

**Guarantees (tested):** the old scene is gone **before** listeners run; the new scene is already active **during** the event; the event names stay valid.

**Errors:** an exception from a listener escapes **after** the switch has happened.

**Related tests:** [SceneManagerTest](../tests/test-scene.md#scenemanagertest-throwswhennoactivesceneexists) (most of them).

<a id="scenemanager-getactivescene"></a>
### `Scene& SceneManager::getActiveScene()`

Returns the active scene, or throws `std::runtime_error("no active scene")` if there isn't one. **Used by:** [`engine`](engine.md#engine), [`renderEngineFrame`](engine.md#renderengineframe).

<a id="scenemanager-update"></a>
### `void SceneManager::update(float dt)`

If a scene is active, it calls [`Scene::update(dt)`](scene.md#scene-update). Otherwise it does nothing (no throw). **Called by:** [`engine`](engine.md#engine) every frame.

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
