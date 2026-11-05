# test_scene.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/scene/test_scene.cpp`](../../tests/scene/test_scene.cpp)

**In one sentence:** It checks [`Scene`](../code/scene.md) (IDs, order, ownership) and [`SceneManager`](../code/scene-manager.md) (replacement, null rejection, and the exact timing of `SceneChangedEvent`).

**Why these tests matter:** Scene switching mixes ownership and events. A wrong order (e.g. notifying before the old scene is gone) causes subtle bugs.

## Setup and shared helpers

| Helper | What it does | Why |
| --- | --- | --- |
| `TrackingComponent(calls, id)` | Its `update` and `render` push `id` into `calls`; `update` also stores `lastDeltaTime` | Prove call order and `dt` |
| `DestructionTracker(destroyed)` | Its destructor sets `destroyed = true` | Prove when things are destroyed |

## Individual tests — Scene

<a id="scenetest-storesscenename"></a>
### `SceneTest.StoresSceneName`

`getSceneName()` returns `"MainMenu"`.

<a id="scenetest-createsentitieswithuniqueincreasingids"></a>
### `SceneTest.CreatesEntitiesWithUniqueIncreasingIds`

The IDs are 1, 2, 3.

<a id="scenetest-entityidsareindependentbetweenscenes"></a>
### `SceneTest.EntityIdsAreIndependentBetweenScenes`

The second scene starts again at 1.

<a id="scenetest-createsdistinctentities"></a>
### `SceneTest.CreatesDistinctEntities`

Two entities have different addresses.

<a id="scenetest-updatesentitiesininsertionorder"></a>
### `SceneTest.UpdatesEntitiesInInsertionOrder`

`update` calls entities in order 1, 2, 3.

<a id="scenetest-passesdeltatimetocomponents"></a>
### `SceneTest.PassesDeltaTimeToComponents`

The component receives exactly `0.25f`.

<a id="scenetest-emptyscenecanupdate"></a>
### `SceneTest.EmptySceneCanUpdate`

No throw when there are no entities.

<a id="scenetest-destroysownedentities"></a>
### `SceneTest.DestroysOwnedEntities`

Leaving the scene's scope destroys its components.

<a id="scenetest-rendersentitiesininsertionorder"></a>
### `SceneTest.RendersEntitiesInInsertionOrder`

With a real software `Renderer`, `render` calls entities in order 1, 2, 3.

## Individual tests — SceneManager

<a id="scenemanagertest-throwswhennoactivesceneexists"></a>
### `SceneManagerTest.ThrowsWhenNoActiveSceneExists`

`getActiveScene()` with nothing set throws `std::runtime_error`.

<a id="scenemanagertest-storesactivescene"></a>
### `SceneManagerTest.StoresActiveScene`

After setting "Level1", `getActiveScene()` returns it.

<a id="scenemanagertest-replacesactivescene"></a>
### `SceneManagerTest.ReplacesActiveScene`

Setting "Level2" replaces "Level1".

<a id="scenemanagertest-destroysprevioussceneonreplacement"></a>
### `SceneManagerTest.DestroysPreviousSceneOnReplacement`

The old scene's components are destroyed on replacement.

<a id="scenemanagertest-updatesonlyactivescene"></a>
### `SceneManagerTest.UpdatesOnlyActiveScene`

After switching, only the new scene updates. The calls are `{1, 2}`, not `{1, 1, 2}`.

<a id="scenemanagertest-updatewithoutactivescenedoesnothing"></a>
### `SceneManagerTest.UpdateWithoutActiveSceneDoesNothing`

`update` with no scene doesn't throw.

<a id="scenemanagertest-rejectsnullscene"></a>
### `SceneManagerTest.RejectsNullScene`

`setActiveScene(nullptr)` throws `std::invalid_argument`.

<a id="scenemanagertest-nullscenedoesnotreplaceexistingscene"></a>
### `SceneManagerTest.NullSceneDoesNotReplaceExistingScene`

After the null is rejected, "Level1" is still active.

<a id="scenemanagertest-firstscenedispatchescorrectevent"></a>
### `SceneManagerTest.FirstSceneDispatchesCorrectEvent`

The first scene sends one event: old `""`, new `"Level1"`.

<a id="scenemanagertest-replacementdispatchescorrectevent"></a>
### `SceneManagerTest.ReplacementDispatchesCorrectEvent`

The second event has old `"Level1"` and new `"Level2"`.

<a id="scenemanagertest-scenechangeeventissynchronous"></a>
### `SceneManagerTest.SceneChangeEventIsSynchronous`

The listener has already run when `setActiveScene` returns.

<a id="scenemanagertest-oldscenedestroyedbeforeevent"></a>
### `SceneManagerTest.OldSceneDestroyedBeforeEvent`

Inside the listener, the old scene is **already** destroyed.

<a id="scenemanagertest-newsceneactiveduringevent"></a>
### `SceneManagerTest.NewSceneActiveDuringEvent`

Inside the listener, `getActiveScene()` already returns the new scene.

<a id="scenemanagertest-eventnamesremainvalidafterreplacement"></a>
### `SceneManagerTest.EventNamesRemainValidAfterReplacement`

Stored events still hold the right names after the scenes are gone. (Its assertions are the same as `ReplacementDispatchesCorrectEvent`.)

<a id="scenemanagertest-nullscenedoesnotdispatchevent"></a>
### `SceneManagerTest.NullSceneDoesNotDispatchEvent`

A rejected null sends no event (the count stays 1).

<a id="scenemanagertest-destroysactivesceneonshutdown"></a>
### `SceneManagerTest.DestroysActiveSceneOnShutdown`

When the manager goes out of scope, the active scene is destroyed.

## Running these tests

**Target:** `engine_tests`. **Command:** `ctest --test-dir build -R "SceneTest|SceneManagerTest" --output-on-failure`.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
