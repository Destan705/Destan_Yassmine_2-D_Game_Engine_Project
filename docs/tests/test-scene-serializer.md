# test_scene_serializer.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/scene/test_scene_serializer.cpp`](../../tests/scene/test_scene_serializer.cpp)

**In one sentence:** It checks [SceneSerializer](../code/scene-serializer.md) loading, every validation rule, safe saving, round trips, and activation through `SceneManager`.

**Why these tests matter:** A scene file is user input. These tests prove that good files load exactly, bad files are rejected with a useful message, and a failed save or load never damages what was there before.

## Setup and shared helpers

| Item | What it does |
| --- | --- |
| `kValidScene` | A valid file: entities 1 and 2 share `red.bmp`; entity 3 uses `sheet.bmp` with a 2-frame, non-looping animation (0.25 s) |
| `SceneSerializerTest::SetUp()` | Software renderer, hidden window, `Renderer`, fresh temp folder with `red.bmp` (4×4) and `sheet.bmp` (64×32), `AssetManager` rooted there |
| `SceneSerializerTest::TearDown()` | AssetManager → Renderer → window → `SDL_Quit`; deletes the folder |
| `WriteFile(name, text)` | Writes a file into the temp folder and returns its path |
| `ReadFile(path)` | Reads a whole file as text |
| `ExpectLoadError(text, expected)` | Writes `text`, expects `load` to throw, and checks the message contains `expected` |
| `ValidSceneWith(from, to)` | `kValidScene` with the first `from` replaced by `to`: a quick way to break exactly one thing |

## Individual tests

### Loading valid files

<a id="sceneserializertest-loadsscenenameandentityorder"></a>
#### `SceneSerializerTest.LoadsSceneNameAndEntityOrder`

Name is `Test Scene`; 3 entities with IDs 1, 2, 3 in file order.

<a id="sceneserializertest-loadstransformvalues"></a>
#### `SceneSerializerTest.LoadsTransformValues`

Entity 2's Transform is exactly 200, 300, 40×60.

<a id="sceneserializertest-spritepathsresolvethroughassetmanager"></a>
#### `SceneSerializerTest.SpritePathsResolveThroughAssetManager`

Sprite handles map back to `red.bmp` and `sheet.bmp` via `getPath`.

<a id="sceneserializertest-sharedtexturepathreusesonehandle"></a>
#### `SceneSerializerTest.SharedTexturePathReusesOneHandle`

Entities 1 and 2 (same path) get the **same** handle.

<a id="sceneserializertest-loadsanimationonlywhereconfigured"></a>
#### `SceneSerializerTest.LoadsAnimationOnlyWhereConfigured`

Entity 1 has no animation; entity 3's duration, loop and frames match the file.

<a id="sceneserializertest-loadsscenewithnoentities"></a>
#### `SceneSerializerTest.LoadsSceneWithNoEntities`

`"entities": []` loads as an empty scene.

### Rejecting invalid files

<a id="sceneserializertest-missingfilethrows"></a>
#### `SceneSerializerTest.MissingFileThrows`

A non-existent file throws `std::runtime_error`.

<a id="sceneserializertest-badsyntaxthrows"></a>
#### `SceneSerializerTest.BadSyntaxThrows`

Cut-off JSON → message contains `syntax`.

<a id="sceneserializertest-duplicatetoplevelkeythrows"></a>
#### `SceneSerializerTest.DuplicateTopLevelKeyThrows`

`name` twice → `duplicate key 'name'`.

<a id="sceneserializertest-duplicatenestedkeythrows"></a>
#### `SceneSerializerTest.DuplicateNestedKeyThrows`

`x` twice inside a transform → `duplicate key 'x'`.

<a id="sceneserializertest-samekeyindifferentobjectsisallowed"></a>
#### `SceneSerializerTest.SameKeyInDifferentObjectsIsAllowed`

Every entity has its own `x`; that's fine.

<a id="sceneserializertest-unsupportedversionthrows"></a>
#### `SceneSerializerTest.UnsupportedVersionThrows`

`version: 2` → mentions `version`.

<a id="sceneserializertest-missingversionthrows"></a>
#### `SceneSerializerTest.MissingVersionThrows`

No `version` → mentions `version`.

<a id="sceneserializertest-versionastextthrows"></a>
#### `SceneSerializerTest.VersionAsTextThrows`

`"1"` (a string) → mentions `version`.

<a id="sceneserializertest-unknowntoplevelfieldthrows"></a>
#### `SceneSerializerTest.UnknownTopLevelFieldThrows`

`author` → `unknown field 'author'`.

<a id="sceneserializertest-unknowncomponentthrows"></a>
#### `SceneSerializerTest.UnknownComponentThrows`

`sound: {}` on an entity → `unknown field 'sound'`.

<a id="sceneserializertest-unknowntransformfieldreportsentity"></a>
#### `SceneSerializerTest.UnknownTransformFieldReportsEntity`

Typo `widht` → message names `entity 1`.

<a id="sceneserializertest-missingtransformthrows"></a>
#### `SceneSerializerTest.MissingTransformThrows`

Entity with only a sprite → `'transform' is missing`.

<a id="sceneserializertest-missingrequiredtransformfieldthrows"></a>
#### `SceneSerializerTest.MissingRequiredTransformFieldThrows`

No `width` → `'width' is missing`.

<a id="sceneserializertest-wrongvaluetypethrows"></a>
#### `SceneSerializerTest.WrongValueTypeThrows`

`"200"` instead of `200` → `'x' is missing or not a number`.

<a id="sceneserializertest-nonpositiveframedurationthrows"></a>
#### `SceneSerializerTest.NonPositiveFrameDurationThrows`

`frameDuration: 0` → mentions `frameDuration`.

<a id="sceneserializertest-emptyanimationframesthrow"></a>
#### `SceneSerializerTest.EmptyAnimationFramesThrow`

`frames: []` → mentions `frames`.

<a id="sceneserializertest-animationloopmustbetrueorfalse"></a>
#### `SceneSerializerTest.AnimationLoopMustBeTrueOrFalse`

`loop: "no"` → mentions `loop`.

<a id="sceneserializertest-missingtexturefilethrows"></a>
#### `SceneSerializerTest.MissingTextureFileThrows`

`missing.bmp` → `std::runtime_error`.

### Saving

<a id="sceneserializertest-saveloadsavepreservesmeaningfulvalues"></a>
#### `SceneSerializerTest.SaveLoadSavePreservesMeaningfulValues`

Load → save → load → save: both saves are equal **as parsed JSON**, and equal to the original file.

<a id="sceneserializertest-savedfilecontainsnoruntimestate"></a>
#### `SceneSerializerTest.SavedFileContainsNoRuntimeState`

The saved text never contains `"id"`, `handle`, `elapsed`, `currentFrame` or `paused`.

<a id="sceneserializertest-saveleavesnotemporaryfile"></a>
#### `SceneSerializerTest.SaveLeavesNoTemporaryFile`

After a save, `saved.json` exists and `saved.json.tmp` doesn't.

<a id="sceneserializertest-failedsavekeepspreviousfile"></a>
#### `SceneSerializerTest.FailedSaveKeepsPreviousFile`

A **folder** named `keep.json.tmp` makes the temp write fail; `keep.json` still has its old content.

<a id="sceneserializertest-savingunsupportedentityfailsandkeepspreviousfile"></a>
#### `SceneSerializerTest.SavingUnsupportedEntityFailsAndKeepsPreviousFile`

An entity with no Sprite → throws, and the old file is untouched.

### Activation through SceneManager

<a id="sceneserializertest-successfulloadthenactivateemitsoneevent"></a>
#### `SceneSerializerTest.SuccessfulLoadThenActivateEmitsOneEvent`

`load` itself sends **no** event; `setActiveScene` sends exactly one, with the new name.

<a id="sceneserializertest-failedloadkeepsoldsceneandemitsnoevent"></a>
#### `SceneSerializerTest.FailedLoadKeepsOldSceneAndEmitsNoEvent`

A bad file (`version: 9`) throws before `setActiveScene` runs: no event, and the old scene is still active.

## Running these tests

**Target:** `engine_tests`. **Filter:** `ctest --test-dir build -R SceneSerializerTest`. Headless Linux: set `SDL_VIDEO_DRIVER=offscreen`.

## Coverage gaps

Collider fields are covered in [test_scene_serializer_contract](test-scene-serializer-contract.md). Not tested: a failed `rename` on Windows when the destination is open in another program.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
