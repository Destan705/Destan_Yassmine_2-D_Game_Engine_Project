# Transform — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/ecs/transform.h`](../../include/ecs/transform.h) (header only).

**In one sentence:** A component that stores where an entity is and how big it is on screen.

**Why it exists:** [`Sprite`](sprite.md) needs a position and size to draw at. Keeping them in their own component means other systems (collision, the serializer) can read them without knowing about sprites.

## Class: `Transform`

<a id="class-transform"></a>

**Parent class:** [`Component`](component.md#class-component). It doesn't override `update` or `render`, so it's pure data.

**How it's created:** `entity.addComponent<Transform>()`, then set the fields through the returned reference.

### Fields

| Field | Type / initial value | Meaning and units | Who changes it |
| --- | --- | --- | --- |
| <a id="transform-x"></a>`x` | `float`, `0.0f` | Left edge, screen pixels | Any code with the reference (currently [engine](engine.md#engine)) |
| <a id="transform-y"></a>`y` | `float`, `0.0f` | Top edge, screen pixels | Same |
| <a id="transform-width"></a>`width` | `float`, `0.0f` | Width, pixels | Same |
| <a id="transform-height"></a>`height` | `float`, `0.0f` | Height, pixels | Same |

The fields are **public** with no validation. A size of 0 draws nothing.

**Read by:** [`Sprite::render`](sprite.md#sprite-render). **Related tests:** [TransformTest](../tests/test-transform-sprite.md).

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
