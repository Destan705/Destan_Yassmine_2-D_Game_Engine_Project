# test_animation_component.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/ecs/test_animation_component.cpp`](../../tests/ecs/test_animation_component.cpp)

**In one sentence:** It checks [AnimationComponent](../code/animation-component.md) validation and, by reading **real pixels**, that the right frame of a sprite sheet is drawn.

**Why these tests matter:** An animation can set the Sprite's source rectangle perfectly while the Renderer ignores it. Reading the drawn pixel catches that; checking only fields would not.

## Setup and shared helpers

| Item | What it does |
| --- | --- |
| `kFrames` | Two 2×2 frames: left `(0,0)` and right `(2,0)` |
| `AnimationComponentContractTest::SetUp()` | Forces the `software` renderer, starts SDL video, opens a hidden 32×32 window, makes a temp folder |
| `CreateTwoFrameBmp()` | Saves a 4×2 BMP: left half **red**, right half **green** |
| `SampleEntityColor(renderer, entity)` | Clears to black, renders the entity (16×16 at 0,0), reads pixel (8, 8) |
| `ExpectColor(color, r, g, b)` | Compares the colour |

So: **red = frame 0, green = frame 1.**

## Individual tests

### Validation (no SDL)

<a id="animationcomponentvalidationtest-rejectsemptyframes"></a>
#### `AnimationComponentValidationTest.RejectsEmptyFrames`

An empty frame list throws `std::invalid_argument`.

<a id="animationcomponentvalidationtest-rejectsinvalidframeduration"></a>
#### `AnimationComponentValidationTest.RejectsInvalidFrameDuration`

Duration 0, −1, infinity or NaN throws.

<a id="animationcomponentvalidationtest-exposesstableconfigurationforserialization"></a>
#### `AnimationComponentValidationTest.ExposesStableConfigurationForSerialization`

The [getters](../code/animation-component.md#animationcomponent-getframes) return the duration, loop flag and frames that were passed in.

<a id="animationcomponentvalidationtest-safelyupdateswithoutownerorsprite"></a>
#### `AnimationComponentValidationTest.SafelyUpdatesWithoutOwnerOrSprite`

`update` doesn't crash with no owner, or with an owner that has no Sprite.

### Pixel contract (real rendering)

<a id="animationcomponentcontracttest-firstframedrawsonlyitssourcerectangle"></a>
#### `AnimationComponentContractTest.FirstFrameDrawsOnlyItsSourceRectangle`

After `update(0)`, the pixel is red: only frame 0 is drawn, not the whole sheet.

<a id="animationcomponentcontracttest-advancingframechangesdisplayedsourcerectangle"></a>
#### `AnimationComponentContractTest.AdvancingFrameChangesDisplayedSourceRectangle`

After `update(0.25)`, the pixel is green.

<a id="animationcomponentcontracttest-pausedanimationdoesnotadvanceuntilresumed"></a>
#### `AnimationComponentContractTest.PausedAnimationDoesNotAdvanceUntilResumed`

Paused, `update(0.5)` stays red; resumed, `update(0.25)` turns green.

<a id="animationcomponentcontracttest-loopinganimationwrapstofirstframe"></a>
#### `AnimationComponentContractTest.LoopingAnimationWrapsToFirstFrame`

Looping, `update(0.5)` passes both frames and wraps back to red.

<a id="animationcomponentcontracttest-nonloopinganimationstaysonlastframe"></a>
#### `AnimationComponentContractTest.NonLoopingAnimationStaysOnLastFrame`

Not looping, two `update(1.0)` calls stay green.

<a id="animationcomponentcontracttest-restartresetsdisplayedframeafterupdate"></a>
#### `AnimationComponentContractTest.RestartResetsDisplayedFrameAfterUpdate`

Green, then [`restart`](../code/animation-component.md#animationcomponent-restart) + `update(0)` → red again.

## Running these tests

**Target:** `engine_tests`. **Filter:** `ctest --test-dir build -R AnimationComponent`. Headless Linux: set `SDL_VIDEO_DRIVER=offscreen`.

## Coverage gaps

Not tested: leftover time across several small updates (e.g. 0.1 + 0.1 + 0.1), negative `dt`, and frames outside the texture (that path is in the Renderer).

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
