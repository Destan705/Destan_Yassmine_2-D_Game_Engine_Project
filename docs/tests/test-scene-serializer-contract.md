# test_scene_serializer_contract.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/scene/test_scene_serializer_contract.cpp`](../../tests/scene/test_scene_serializer_contract.cpp)

**In one sentence:** Extra [SceneSerializer](../code/scene-serializer.md) checks: the real `phase3-demo.json`, error messages with file/entity/field, colliders, finite numbers, and safe replacement.

**Why these tests matter:** These pin down the **contract** other code relies on: what a good error message contains, how colliders are stored, and that the shipped demo file actually loads.

## Setup and shared helpers

| Item | What it does |
| --- | --- |
| `WithCollider(collider)` | A one-entity scene with the given `collider` JSON |
| `BasicScene(transform, sprite)` | A one-entity scene with the given transform (and sprite) |
| `ReadAll(path)` | Reads a file |
| `SceneSerializerContractTest::SetUp()` | Software renderer, hidden window, temp folder with `red.bmp`, AssetManager rooted there |
| `WriteText(name, text)` | Writes a file in the temp folder |
| `ExpectDiagnostic(json, field, context = "entity 0")` | Expects `load` to fail **and** the message to contain the file path, the context and the field name |

## Individual tests

### Real demo file

<a id="sceneserializercontracttest-loadsrealphase3demothroughassetmanager"></a>
#### `SceneSerializerContractTest.LoadsRealPhase3DemoThroughAssetManager`

Loads `assets/scenes/phase3-demo.json` with an AssetManager rooted at the repo's `assets/`. Name `Phase 3 Demo`, 3 entities, entities 1 and 2 share the `test.bmp` handle, entity 3 has 4 looping frames. Relies on CTest's working directory being the project root.

### Error messages

<a id="sceneserializercontracttest-rejectsmissingspritewithentitycontext"></a>
#### `SceneSerializerContractTest.RejectsMissingSpriteWithEntityContext`

No sprite → message has path, `entity 0`, `sprite`.

<a id="sceneserializercontracttest-missingtextureerrornamesentityandtexturefield"></a>
#### `SceneSerializerContractTest.MissingTextureErrorNamesEntityAndTextureField`

Missing BMP → message has path, `entity 0`, `texture`.

<a id="sceneserializercontracttest-rejectsnonfinitetransformvalue"></a>
#### `SceneSerializerContractTest.RejectsNonFiniteTransformValue`

`x: 1e100` overflows `float` → message mentions `x`.

<a id="sceneserializercontracttest-rejectsanimationframewithzerowidth"></a>
#### `SceneSerializerContractTest.RejectsAnimationFrameWithZeroWidth`

Frame `w: 0` → message mentions `w`.

### Colliders

<a id="sceneserializercontracttest-loadcolliderwithoffsetsdimensionsanddisabledstate"></a>
#### `SceneSerializerContractTest.LoadColliderWithOffsetsDimensionsAndDisabledState`

Offsets 2, −3, size 10×12, `enabled: false` all arrive in the `ColliderComponent`.

<a id="sceneserializercontracttest-collidersaveloadsavepreservesmeaningfulvalues"></a>
#### `SceneSerializerContractTest.ColliderSaveLoadSavePreservesMeaningfulValues`

Collider survives save → load → save; no `id` field is written.

<a id="sceneserializercontracttest-rejectscolliderwithoutwidthwithcontext"></a>
#### `SceneSerializerContractTest.RejectsColliderWithoutWidthWithContext`

Missing `width` → message mentions `width`.

<a id="sceneserializercontracttest-rejectscolliderwithnonpositivewidth"></a>
#### `SceneSerializerContractTest.RejectsColliderWithNonPositiveWidth`

`width: 0` → message mentions `width`.

<a id="sceneserializercontracttest-rejectscolliderenabledwhennotboolean"></a>
#### `SceneSerializerContractTest.RejectsColliderEnabledWhenNotBoolean`

`enabled: "yes"` → message mentions `enabled`.

### Safe replacement

<a id="sceneserializercontracttest-savecanreplaceexistingvalidfile"></a>
#### `SceneSerializerContractTest.SaveCanReplaceExistingValidFile`

Saving over an existing file replaces it and leaves no `.tmp`.

<a id="sceneserializercontracttest-failedreplacementpreservesdestinationdirectory"></a>
#### `SceneSerializerContractTest.FailedReplacementPreservesDestinationDirectory`

If the destination is a **folder**, save throws, the folder survives and no `.tmp` is left.

### Activation

<a id="sceneserializercontracttest-failedcolliderloaddoesnotreplaceactivesceneoremitevent"></a>
#### `SceneSerializerContractTest.FailedColliderLoadDoesNotReplaceActiveSceneOrEmitEvent`

A collider with width −1 makes load fail: old scene stays active, no event.

## Running these tests

**Target:** `engine_tests`. **Filter:** `ctest --test-dir build -R SceneSerializerContractTest`. Run through `ctest` (or from the repo root) so the demo-file test finds `assets/`.

## Coverage gaps

Not tested here: animation frames that don't fit inside the texture (they're accepted at load and skipped at draw time).

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
