# test_renderer.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/rendering/test_renderer.cpp`](../../tests/rendering/test_renderer.cpp)

**In one sentence:** It checks that [`Renderer`](../code/renderer.md) creates and releases SDL resources correctly, and that it draws the **right pixels** for clears, rectangles and textures.

**Why these tests matter:** They read actual pixels back, so they catch swapped x/y, width/height or colour channels, and leaked or double-freed SDL objects.

## Test coverage at a glance

| Test | What it checks | Jump | Code under test |
| --- | --- | --- | --- |
| `static_assert`s | Renderer can't be copied | [Below](#compile-time-checks) | [Renderer class](../code/renderer.md#class-renderer) |
| `RendererTest.CreatesRendererForValidWindow` | Construction attaches a renderer | [Jump](#renderertest-createsrendererforvalidwindow) | [Constructor](../code/renderer.md#renderer-renderer) |
| `RendererTest.RejectsNullWindow` | Null window throws | [Jump](#renderertest-rejectsnullwindow) | [Constructor](../code/renderer.md#renderer-renderer) |
| `RendererTest.ReleasesRendererWhenItGoesOutOfScope` | RAII clean-up | [Jump](#renderertest-releasesrendererwhenitgoesoutofscope) | [Destructor](../code/renderer.md#renderer-destructor) |
| `RendererTest.ClearScreenFillsEntireDrawingAreaWithRequestedColor` | Every pixel = clear colour | [Jump](#renderertest-clearscreenfillsentiredrawingareawithrequestedcolor) | [ClearScreen](../code/renderer.md#renderer-clearscreen) |
| `RendererTest.DrawFilledRectUsesRequestedColorPositionAndSize` | Exact rectangle pixels | [Jump](#renderertest-drawfilledrectusesrequestedcolorpositionandsize) | [DrawFilledRect](../code/renderer.md#renderer-drawfilledrect) |
| `RendererTest.ClearScreenRemovesPreviouslyDrawnContent` | Clear erases | [Jump](#renderertest-clearscreenremovespreviouslydrawncontent) | [ClearScreen](../code/renderer.md#renderer-clearscreen) |
| `RendererTest.LaterRectangleOverwritesOverlapAndPreservesOtherPixels` | Draw order | [Jump](#renderertest-laterrectangleoverwritesoverlapandpreservesotherpixels) | [DrawFilledRect](../code/renderer.md#renderer-drawfilledrect) |
| `TextureTest.LoadsValidBmpAndDrawsAtRequestedPositionAndSize` | Load + draw texture | [Jump](#texturetest-loadsvalidbmpanddrawsatrequestedpositionandsize) | [LoadTexture](../code/renderer.md#renderer-loadtexture), [DrawTexture](../code/renderer.md#renderer-drawtexture) |
| `TextureTest.MissingFileThrowsRuntimeError` | Missing file throws | [Jump](#texturetest-missingfilethrowsruntimeerror) | [LoadTexture](../code/renderer.md#renderer-loadtexture) |
| `TextureTest.InvalidBmpThrowsRuntimeError` | Bad file throws | [Jump](#texturetest-invalidbmpthrowsruntimeerror) | [LoadTexture](../code/renderer.md#renderer-loadtexture) |
| `TextureTest.DrawingWithoutTextureLeavesScreenUnchanged` | Handle 0 draws nothing | [Jump](#texturetest-drawingwithouttextureleavesscreenunchanged) | [DrawTexture](../code/renderer.md#renderer-drawtexture) |
| `TextureTest.LoadedTexturesReceiveDistinctHandles` | Unique, non-zero handles | [Jump](#texturetest-loadedtexturesreceivedistincthandles) | [LoadTexture](../code/renderer.md#renderer-loadtexture) |
| `TextureTest.DrawsTwoDifferentTexturesInSameFrame` | Two textures, one frame | [Jump](#texturetest-drawstwodifferenttexturesinsameframe) | [DrawTexture](../code/renderer.md#renderer-drawtexture) |
| `TextureTest.InvalidHandleDoesNotChangeScreen` | Huge handle is safe | [Jump](#texturetest-invalidhandledoesnotchangescreen) | [DrawTexture](../code/renderer.md#renderer-drawtexture) |
| `TextureTest.LoadingSecondTexturePreservesFirstHandle` | Handles stay valid | [Jump](#texturetest-loadingsecondtexturepreservesfirsthandle) | [LoadTexture](../code/renderer.md#renderer-loadtexture) |
| `TextureTest.DestroysRendererWithMultipleLoadedTextures` | Clean-up with textures | [Jump](#texturetest-destroysrendererwithmultipleloadedtextures) | [Destructor](../code/renderer.md#renderer-destructor) |

## Setup and shared helpers

<a id="compile-time-checks"></a>
**Compile-time checks:** `static_assert(!std::is_copy_constructible_v<Renderer>)` and `!is_copy_assignable_v`. If someone re-enables copying, the build fails.

`SurfacePtr`: a `unique_ptr<SDL_Surface>` that frees with `SDL_DestroySurface`.

### `RendererTest` fixture

| Member / method | What it prepares or cleans up | Why |
| --- | --- | --- |
| `SetUp()` | Forces the `software` render driver, `SDL_Init(VIDEO)`, and a hidden 64×64 window | Pixel results don't depend on the GPU |
| `TearDown()` | Destroys the window, `SDL_Quit`, resets the hint | Isolation between tests |
| `window` | The 64×64 hidden window | Target for each `Renderer` |
| `ReadPixels()` | Copies the back buffer into a surface | Read **before** `Present()`, since the back buffer isn't kept after presenting |
| `ExpectPixel(surface, x, y, color)` | Asserts one pixel's RGBA | Labels failures with the pixel position |

### `TextureTest` fixture (extends `RendererTest`)

| Member / method | What it does | Why |
| --- | --- | --- |
| `SetUp()` | Also creates a unique temp folder (random name, up to 100 tries) | Parallel tests don't share files |
| `TearDown()` | Deletes the temp folder | No leftovers |
| `FilePath(name)` | UTF-8 path inside the temp folder | SDL expects UTF-8 on every OS |
| `CreateBmp(path, color)` | Saves a 2×2 solid-colour BMP | Known test image |
| `ExpectSolidScreen(color)` | Every pixel equals `color` | "Nothing was drawn" checks |

## Individual tests

<a id="renderertest-createsrendererforvalidwindow"></a>
### `RendererTest.CreatesRendererForValidWindow`

| Stage | What happens |
| --- | --- |
| Given | A window with no renderer (`SDL_GetRenderer == nullptr`) |
| When | `Renderer renderer(window)` |
| Then | No throw, and the window now has a renderer |

<a id="renderertest-rejectsnullwindow"></a>
### `RendererTest.RejectsNullWindow`

**When:** `Renderer(nullptr)`. **Then:** throws `std::runtime_error`. **Why:** a failed creation must not produce a half-made object.

<a id="renderertest-releasesrendererwhenitgoesoutofscope"></a>
### `RendererTest.ReleasesRendererWhenItGoesOutOfScope`

A renderer in an inner scope. After the scope ends, the window has no renderer, and a new `Renderer` can be created. This catches a missing `SDL_DestroyRenderer`.

<a id="renderertest-clearscreenfillsentiredrawingareawithrequestedcolor"></a>
### `RendererTest.ClearScreenFillsEntireDrawingAreaWithRequestedColor`

Clears to `{17,83,149,255}` and checks **all** 64×64 pixels. The unusual values catch swapped channels.

<a id="renderertest-drawfilledrectusesrequestedcolorpositionandsize"></a>
### `RendererTest.DrawFilledRectUsesRequestedColorPositionAndSize`

Draws a red rectangle at `{7,13,19,11}` on black. Pixels with `7 ≤ x < 26` and `13 ≤ y < 24` must be red, and all others black. The distinct numbers catch an x/y or w/h swap.

<a id="renderertest-clearscreenremovespreviouslydrawncontent"></a>
### `RendererTest.ClearScreenRemovesPreviouslyDrawnContent`

Draws a rectangle, then clears to a new colour. Every pixel must be the new colour.

<a id="renderertest-laterrectangleoverwritesoverlapandpreservesotherpixels"></a>
### `RendererTest.LaterRectangleOverwritesOverlapAndPreservesOtherPixels`

Red, then an overlapping blue rectangle. Blue must win where they overlap, red must stay elsewhere, and the background must stay black. Proves "later draw on top".

<a id="texturetest-loadsvalidbmpanddrawsatrequestedpositionandsize"></a>
### `TextureTest.LoadsValidBmpAndDrawsAtRequestedPositionAndSize`

Loads a red 2×2 BMP and draws it at `{7,13,19,11}`. That rectangle is red (the texture is stretched) and the rest is black.

<a id="texturetest-missingfilethrowsruntimeerror"></a>
### `TextureTest.MissingFileThrowsRuntimeError`

`LoadTexture` on a path that doesn't exist throws `std::runtime_error`.

<a id="texturetest-invalidbmpthrowsruntimeerror"></a>
### `TextureTest.InvalidBmpThrowsRuntimeError`

A file named `.bmp` that contains text throws `std::runtime_error`.

<a id="texturetest-drawingwithouttextureleavesscreenunchanged"></a>
### `TextureTest.DrawingWithoutTextureLeavesScreenUnchanged`

`DrawTexture(0, …)` doesn't throw and the screen stays the clear colour. Proves handle 0 means "nothing".

<a id="texturetest-loadedtexturesreceivedistincthandles"></a>
### `TextureTest.LoadedTexturesReceiveDistinctHandles`

Two loads produce two non-zero, different handles.

<a id="texturetest-drawstwodifferenttexturesinsameframe"></a>
### `TextureTest.DrawsTwoDifferentTexturesInSameFrame`

Red at x 5–25 and blue at x 35–55. It samples (10,15) = red, (40,15) = blue and (30,15) = background.

<a id="texturetest-invalidhandledoesnotchangescreen"></a>
### `TextureTest.InvalidHandleDoesNotChangeScreen`

`DrawTexture(9999u, …)` doesn't throw and changes no pixels.

<a id="texturetest-loadingsecondtexturepreservesfirsthandle"></a>
### `TextureTest.LoadingSecondTexturePreservesFirstHandle`

After loading the blue texture, the **red** handle still draws red. This catches a vector-reallocation or index bug.

<a id="texturetest-destroysrendererwithmultipleloadedtextures"></a>
### `TextureTest.DestroysRendererWithMultipleLoadedTextures`

Loads two textures inside a scope. Afterwards the window has no renderer and a new one can be created, which proves the destructor handles textures.

## Running these tests

**Build target:** `engine_tests`. **Command:** `cmake --build build && ctest --test-dir build --output-on-failure` (optionally `-R "RendererTest|TextureTest"`).

**What a failure usually points to:** [`Renderer`](../code/renderer.md) drawing maths, or SDL resource lifetime.

## Coverage gaps

- **There's no test for `DrawTexture` with a source rectangle** (valid crop, out-of-bounds crop, negative/zero size). This is the main Phase 3 rendering change.
- The warn-once behaviour (`hasWarned`) isn't checked.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
