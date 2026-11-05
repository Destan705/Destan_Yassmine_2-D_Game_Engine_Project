# test_transform_sprite.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/ecs/test_transform_sprite.cpp`](../../tests/ecs/test_transform_sprite.cpp)

**In one sentence:** It checks that [`Transform`](../code/transform.md) stores values and that [`Sprite`](../code/sprite.md) attaches correctly and safely skips drawing without a Transform.

## Individual tests

<a id="transformtest-defaultstozero"></a>
### `TransformTest.DefaultsToZero`

A new Transform has x, y, width and height all equal to 0.

<a id="transformtest-storespositionandsize"></a>
### `TransformTest.StoresPositionAndSize`

The values written can be read back unchanged.

<a id="transformtest-valuescanbeupdated"></a>
### `TransformTest.ValuesCanBeUpdated`

`+=` and `-=` work: x = 15, y = 10.

<a id="transformtest-canbeattachedtoentity"></a>
### `TransformTest.CanBeAttachedToEntity`

`getComponent<Transform>()` returns the same object, and `getOwner()` is the entity.

<a id="transformtest-differententitieshaveindependenttransforms"></a>
### `TransformTest.DifferentEntitiesHaveIndependentTransforms`

Two entities have separate Transforms (25 vs 75).

<a id="spritetest-canbeattachedtoentity"></a>
### `SpriteTest.CanBeAttachedToEntity`

`addComponent<Sprite>(1u)` is found by `getComponent`, and its owner is set.

<a id="spritetest-spriteandtransformsharesameowner"></a>
### `SpriteTest.SpriteAndTransformShareSameOwner`

Both components point to the same entity.

<a id="spritetest-cannotattachduplicatesprite"></a>
### `SpriteTest.CannotAttachDuplicateSprite`

A second Sprite throws `std::logic_error`.

<a id="spritetest-cannotattachduplicatetransform"></a>
### `SpriteTest.CannotAttachDuplicateTransform`

A second Transform throws `std::logic_error`.

<a id="spritetest-missingtransformcanbedetected"></a>
### `SpriteTest.MissingTransformCanBeDetected`

An entity with only a Sprite gives `getComponent<Transform>() == nullptr`.

<a id="spritetest-differententitiescanhavedifferentsprites"></a>
### `SpriteTest.DifferentEntitiesCanHaveDifferentSprites`

Two Sprites are different objects, each with its own owner.

<a id="spritetest-missingtransformsafelyskipsdrawing"></a>
### `SpriteTest.MissingTransformSafelySkipsDrawing`

**Setup:** dummy video + software renderer, a hidden 320×240 window and a real `Renderer`. An entity with a Sprite but no Transform. **Then:** `entity.render(renderer)` doesn't throw. This proves the early return in [`Sprite::render`](../code/sprite.md#sprite-render). Cleans up the window, SDL and hints at the end. If an assertion fails before that, clean-up is skipped (this test isn't a fixture).

## Coverage gaps

- No test that Sprite **draws the right pixels** at the Transform.
- No test for [`sourceSet`](../code/sprite.md#sprite-sourceset) or cropped drawing.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
