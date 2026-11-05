# test_asset_manager.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/assets/test_asset_manager.cpp`](../../tests/assets/test_asset_manager.cpp)

**In one sentence:** It checks the [AssetManager](../code/asset-manager.md) cache, path rules, reverse lookup and texture sizes against a **real** software Renderer.

**Why these tests matter:** If the cache broke, every sprite would load its own copy of a texture; if the path rules broke, a scene file could load files from outside the asset folder.

## Setup and shared helpers

| Item | What it does |
| --- | --- |
| `AssetManagerTest::SetUp()` | Forces SDL's `software` render driver, starts SDL video, opens a hidden 64×64 window, creates a `Renderer`, makes a fresh random temp folder with `red.bmp` (4×4) and `sprites/cat.bmp` (16×8), and an `AssetManager` rooted there |
| `AssetManagerTest::TearDown()` | Destroys the AssetManager, then the Renderer, then the window; `SDL_Quit`; resets the hint; deletes the temp folder |
| `CreateBmp(path, w, h)` | Writes a blank BMP of that size with `SDL_SaveBMP` |

**Trick used:** the Renderer hands out handles 1, 2, 3… in order. So if loading the same path three times and then a *new* file gives `first + 1`, the Renderer must have loaded only once.

## Individual tests

### Caching

<a id="assetmanagertest-loadstextureandreturnsvalidhandle"></a>
#### `AssetManagerTest.LoadsTextureAndReturnsValidHandle`

A real file gives a non-zero handle from [`loadTexture`](../code/asset-manager.md#assetmanager-loadtexture).

<a id="assetmanagertest-samepathreturnssamehandle"></a>
#### `AssetManagerTest.SamePathReturnsSameHandle`

Loading `red.bmp` twice returns the same handle.

<a id="assetmanagertest-repeatedpathloadsonlyonce"></a>
#### `AssetManagerTest.RepeatedPathLoadsOnlyOnce`

Three loads of `red.bmp`, then `sprites/cat.bmp` gets `first + 1`: the repeats didn't reach the Renderer.

<a id="assetmanagertest-equivalentpathspellingsshareonehandle"></a>
#### `AssetManagerTest.EquivalentPathSpellingsShareOneHandle`

`./sprites/cat.bmp`, `sprites/./cat.bmp`, `sprites\cat.bmp` and `sprites//cat.bmp` all return the same handle as `sprites/cat.bmp`.

<a id="assetmanagertest-differentfilesgetdifferenthandles"></a>
#### `AssetManagerTest.DifferentFilesGetDifferentHandles`

Two files, two handles.

### Reverse lookup

<a id="assetmanagertest-getpathreturnsnormalizedpath"></a>
#### `AssetManagerTest.GetPathReturnsNormalizedPath`

Loading `./sprites\cat.bmp` then [`getPath`](../code/asset-manager.md#assetmanager-getpath) gives the clean `sprites/cat.bmp`.

<a id="assetmanagertest-getpathofunknownhandlethrows"></a>
#### `AssetManagerTest.GetPathOfUnknownHandleThrows`

`getPath(0)` and `getPath(999)` throw `std::out_of_range`.

### Failures and retry

<a id="assetmanagertest-missingfilethrows"></a>
#### `AssetManagerTest.MissingFileThrows`

A missing file throws `std::runtime_error` (from the Renderer).

<a id="assetmanagertest-failedloadisnotcachedandcanberetried"></a>
#### `AssetManagerTest.FailedLoadIsNotCachedAndCanBeRetried`

`later.bmp` fails, the test then creates it, and the second load succeeds with the right path. Proves failures aren't cached.

<a id="assetmanagertest-failedloaddoesnotaffectexistingentries"></a>
#### `AssetManagerTest.FailedLoadDoesNotAffectExistingEntries`

A failed load in between doesn't change the handle or path of an earlier `red.bmp`.

### Invalid paths

<a id="assetmanagertest-rejectsemptypath"></a>
#### `AssetManagerTest.RejectsEmptyPath`

`""` and `"."` throw `std::invalid_argument`.

<a id="assetmanagertest-rejectsabsoluteandrootedpaths"></a>
#### `AssetManagerTest.RejectsAbsoluteAndRootedPaths`

`/red.bmp`, `//server/red.bmp` and the full temp-folder path throw `invalid_argument`.

<a id="assetmanagertest-rejectswindowsdrivepaths"></a>
#### `AssetManagerTest.RejectsWindowsDrivePaths`

`C:/red.bmp` and `c:red.bmp` throw, even on macOS/Linux.

<a id="assetmanagertest-rejectsparenttraversal"></a>
#### `AssetManagerTest.RejectsParentTraversal`

`../red.bmp` and `sprites/../red.bmp` throw, even though the second would point inside the root.

### Texture dimensions

<a id="assetmanagertest-reportsoriginaltexturedimensions"></a>
#### `AssetManagerTest.ReportsOriginalTextureDimensions`

[`getOriginalDimensions`](../code/asset-manager.md#assetmanager-getoriginaldimensions) on `cat.bmp` gives 16 × 8.

<a id="assetmanagertest-dimensionsofunknownhandlethrow"></a>
#### `AssetManagerTest.DimensionsOfUnknownHandleThrow`

Handle 42 throws `std::out_of_range`.

<a id="assetmanagertest-rendererdimensionquerymatchestexture"></a>
#### `AssetManagerTest.RendererDimensionQueryMatchesTexture`

[`Renderer::getTextureDimensions`](../code/renderer.md#renderer-gettexturedimensions) on `red.bmp` gives 4.0 × 4.0.

<a id="assetmanagertest-rendererdimensionqueryrejectsinvalidhandle"></a>
#### `AssetManagerTest.RendererDimensionQueryRejectsInvalidHandle`

`getTextureDimensions(0)` and `(999)` throw `std::invalid_argument`.

## Running these tests

**Target:** `engine_tests`. **Filter:** `ctest --test-dir build -R AssetManagerTest`. Needs no real screen (hidden window, software renderer); in a headless Linux container also set `SDL_VIDEO_DRIVER=offscreen`.

## Coverage gaps

Not tested: a map-insert failure (`logic_error` path), very long paths, case-insensitive file systems (`Red.bmp` vs `red.bmp` would be two cache entries).

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
