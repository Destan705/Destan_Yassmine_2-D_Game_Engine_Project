# ColliderSystem — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/colliderSystem.h`](../../include/colliderSystem.h) and [`src/colliderSystem.cpp`](../../src/colliderSystem.cpp).

**In one sentence:** Looks at every pair of entities in a scene once and returns the pairs whose hitboxes overlap.

**Why it exists:** Collision **detection** is kept separate from collision **response**. This system only answers "who overlaps whom?"; gameplay ([`handleCollisions`](game.md#handlecollisions)) decides what to do about it.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | Axis-aligned rectangle (AABB) overlap checks |
| Gets information from | Each entity's [Transform](transform.md) and [ColliderComponent](collider-component.md) |
| Changes | Nothing. It's `const` and keeps no state between calls. |
| Used by | [engine](engine.md#engine), once per frame after `update` |
| Depends on | [Scene](scene.md#scene-getentities), [Entity](entity.md), [Transform](transform.md), [ColliderComponent](collider-component.md) |

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `ColliderSystem` | Stateless detector | [Class](#class-collidersystem) |
| `CollisionPair` | `std::pair<uint64_t, uint64_t>` of entity IDs | [Type](#collidersystem-collisionpair) |
| `detectCollisions(Scene&)` | Find all overlapping pairs | [Function](#collidersystem-detectcollisions) |
| `rectangle`, `overlaps`, `getWorldBounds` | Private helpers | [Helpers](#helpers) |
| The overlap rule | Why touching isn't colliding | [Rule](#collision-rule) |

## Class: `ColliderSystem`

<a id="class-collidersystem"></a>

**How it's created:** `ColliderSystem colliderSystem;`. It has no fields, so it is cheap and can be used with any scene.

<a id="collidersystem-collisionpair"></a>
**`using CollisionPair = std::pair<std::uint64_t, std::uint64_t>;`** Two entity IDs. `first` is always the entity that comes **earlier** in the scene. IDs are only meaningful inside that scene.

## The overlap rule

<a id="collision-rule"></a>

Two rectangles collide only if they overlap on **both** axes, using **strict** `<` / `>`:

```text
horizontal: A.right > B.left  and  A.left < B.right
vertical:   A.bottom > B.top  and  A.top  < B.bottom
```

| Situation | Collides? |
| --- | --- |
| Overlap by any amount (even 0.5 px) | Yes |
| One inside the other, or identical | Yes |
| Edges exactly touching (`A.right == B.left`) | **No** |
| Corners exactly touching | **No** |
| Overlap on one axis only | No |

## Functions

<a id="collidersystem-detectcollisions"></a>
### `std::vector<ColliderSystem::CollisionPair> ColliderSystem::detectCollisions(Scene& scene) const`

**What it does:** Returns every overlapping pair, each **once**, in scene order.

**What happens, in order**

1. Gets the entity list with [`scene.getEntities()`](scene.md#scene-getentities) (no copy).
2. For each entity `i`: works out its world rectangle with `getWorldBounds`. If it has none, skips it.
3. For each **later** entity `j = i + 1 …`: works out its rectangle (skips if none). If `overlaps`, appends `(id_i, id_j)`.
4. Returns the list (empty if nothing overlaps).

Starting `j` at `i + 1` means an entity is never paired with itself and `(1,2)` is never also reported as `(2,1)`.

**Calls other project functions**

| Calls | Why | Where |
| --- | --- | --- |
| `Scene::getEntities` | Walk every entity | [getEntities](scene.md#scene-getentities) |
| `Entity::getComponent<Transform/ColliderComponent>` | Find position and hitbox | [getComponent](entity.md#entity-getcomponent) |
| `ColliderComponent` getters / `isEnabled` | Local bounds and on/off | [ColliderComponent](collider-component.md#collidercomponent-getters) |
| `Entity::getId` | Fill the pair | [getId](entity.md#entity-getid) |

**Cost:** checks every pair, so the work grows with *n²*. Fine for a few hundred entities.

**Edge cases:** the result is recomputed from scratch every call, using the **current** Transforms. A pair that stays overlapping is returned every frame (no "enter"/"exit" events).

**Related tests:** [ColliderSystemTest](../tests/test-collision.md#collidersystem-tests).

## Helpers

<a id="helpers"></a>

Private, in an anonymous namespace in the `.cpp`.

| Helper | What it does |
| --- | --- |
| `struct rectangle { left, right, top, bottom }` | World-space box, easier to compare than x/y/w/h |
| `bool overlaps(const rectangle&, const rectangle&)` | The strict rule above |
| `std::optional<rectangle> getWorldBounds(Entity&)` | Returns `nullopt` (skip) if there's no Transform, no collider, the collider is disabled, any edge isn't finite, or the box has no area; otherwise Transform position + offsets |

## Things worth knowing

- **Detection only:** nothing is pushed apart or blocked. That's gameplay's job.
- **No continuous collision:** a fast, thin object can jump past another between frames.
- In the demo, the player (entity 3) and the trigger (entity 2) have colliders. [`handleCollisions`](game.md#handlecollisions) turns "overlapping this frame" into "just entered".
- `detectCollisions` takes `Scene&`, but `const Scene&` would also compile: `getEntities()` is `const`, and a `const unique_ptr<Entity>` still gives a non-const `Entity`, so the non-const `getComponent` can be called.

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
