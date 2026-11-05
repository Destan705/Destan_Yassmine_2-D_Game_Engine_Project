# ColliderComponent — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/ecs/colliderComponent.h`](../../include/ecs/colliderComponent.h) and [`src/ecs/colliderComponent.cpp`](../../src/ecs/colliderComponent.cpp).

**In one sentence:** Stores an entity's **hitbox**: a rectangle placed relative to its [`Transform`](transform.md), plus an on/off switch.

**Why it exists:** The visible sprite and the area that "counts" for collisions are often different (a character's feet, a smaller coin). The collider describes that area; the [ColliderSystem](collider-system.md) does the checking.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | Holding valid local bounds and the enabled flag |
| Gets information from | Its constructor / setters |
| Changes | Only its own fields (it has no `update` or `render`) |
| Used by | [ColliderSystem](collider-system.md), [SceneSerializer](scene-serializer.md) (the demo JSON gives colliders to the player and the trigger) |
| Depends on | [Component](component.md) |

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `ColliderComponent` | Local hitbox data | [Class](#class-collidercomponent) |
| Constructor | Validate and store bounds | [Function](#collidercomponent-collidercomponent) |
| `setLocalBounds(...)` | Change the hitbox | [Function](#collidercomponent-setlocalbounds) |
| `setEnabled(bool)` / `isEnabled()` | Turn on/off, check | [setEnabled](#collidercomponent-setenabled) · [isEnabled](#collidercomponent-isenabled) |
| `getOffsetX/Y`, `getWidth`, `getHeight` | Read the bounds | [Getters](#collidercomponent-getters) |

## Class: `ColliderComponent`

<a id="class-collidercomponent"></a>

**Parent class:** [`Component`](component.md#class-component). It overrides nothing, so the default `update`/`render` do nothing.

**How it's created:** `entity.addComponent<ColliderComponent>(offsetX, offsetY, width, height)` or with a 5th argument `enabled` (default `true`).

**Local vs world:** the stored values are **local** (relative to the Transform's top-left). The world rectangle is worked out each frame by the ColliderSystem:

```text
left = transform.x + offsetX      right  = left + width
top  = transform.y + offsetY      bottom = top  + height
```

So moving the Transform moves the hitbox automatically.

**Important rules:** offsets are finite; width and height are finite and `> 0`. Offsets may be negative.

### Fields

| Field | Type | Meaning (units) | Who changes it |
| --- | --- | --- | --- |
| <a id="collidercomponent-offsetx"></a>`offsetX` | `float` | Pixels right of the Transform's x | `setLocalBounds` |
| <a id="collidercomponent-offsety"></a>`offsetY` | `float` | Pixels below the Transform's y | `setLocalBounds` |
| <a id="collidercomponent-width"></a>`width` | `float` | Hitbox width (pixels, > 0) | `setLocalBounds` |
| <a id="collidercomponent-height"></a>`height` | `float` | Hitbox height (pixels, > 0) | `setLocalBounds` |
| <a id="collidercomponent-enabled"></a>`enabled` | `bool` | When false, the system ignores this collider | Constructor, `setEnabled` |

## Functions

<a id="collidercomponent-collidercomponent"></a>
### `ColliderComponent::ColliderComponent(float offsetX, float offsetY, float width, float height, bool enabled = true)`

Stores `enabled`, then calls [`setLocalBounds`](#collidercomponent-setlocalbounds), which validates. A throw means the component is never attached to the entity.

<a id="collidercomponent-setlocalbounds"></a>
### `void ColliderComponent::setLocalBounds(float offsetX, float offsetY, float width, float height)`

**What it does:** Checks all four values first, then stores them.

**Errors (`std::invalid_argument`):**

- `Offset values must be finite` (NaN or infinity offset).
- `Width must be finite and greater than zero`.
- `Height must be finite and greater than zero`.

**Strong guarantee:** because every check comes before the first assignment, a failed call leaves the **old** bounds unchanged. **Related test:** [FailedUpdateKeepsPreviousBounds](../tests/test-collision.md#collidercomponenttest-failedupdatekeepspreviousbounds).

<a id="collidercomponent-setenabled"></a>
### `void ColliderComponent::setEnabled(bool enabled)`

Sets the flag. Disabled colliders keep their bounds and are just skipped by detection (good for a trigger that should only work sometimes).

<a id="collidercomponent-isenabled"></a>
### `bool ColliderComponent::isEnabled() const`

Returns the flag.

<a id="collidercomponent-getters"></a>
### `float getOffsetX() const` · `float getOffsetY() const` · `float getWidth() const` · `float getHeight() const`

Return the stored local values. Used by [`ColliderSystem`](collider-system.md) to build the world rectangle and by [`SceneSerializer::save`](scene-serializer.md#sceneserializer-save).

**No direct project-function calls** in any of these functions.

## Things worth knowing

- The collider doesn't know its world position; there is no "getWorldBounds" on the component.
- Nothing stops you adding a collider to an entity without a Transform; the system just skips it.
- **Tests:** [ColliderComponentTest](../tests/test-collision.md#collidercomponent-tests).

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
