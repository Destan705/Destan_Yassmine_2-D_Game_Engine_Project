# test_collision.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/ecs/test_collision.cpp`](../../tests/ecs/test_collision.cpp)

**In one sentence:** It checks [ColliderComponent](../code/collider-component.md) validation and every [ColliderSystem](../code/collider-system.md) overlap rule, with no SDL at all.

**Why these tests matter:** Collision bugs are usually edge cases (touching edges, offsets, disabled colliders, duplicates). Each one has its own small test here.

## Setup and shared helpers

| Item | What it does |
| --- | --- |
| `Pair` | Short name for `ColliderSystem::CollisionPair` |
| `AddBox(scene, x, y, w, h, enabled = true)` | Adds an entity with a Transform at (x, y) and a collider covering exactly that area (offset 0, 0) |
| `Detect(scene)` | Runs a fresh `ColliderSystem` on the scene |

Entity IDs in the expected results come from creation order (1, 2, 3…).

## Individual tests

<a id="collidercomponent-tests"></a>

### ColliderComponent

<a id="collidercomponenttest-storeslocalbounds"></a>
#### `ColliderComponentTest.StoresLocalBounds`

Getters return the constructor values, including a negative offset.

<a id="collidercomponenttest-enabledbydefault"></a>
#### `ColliderComponentTest.EnabledByDefault`

Without the 5th argument, the collider is enabled.

<a id="collidercomponenttest-canbecreateddisabledandtoggled"></a>
#### `ColliderComponentTest.CanBeCreatedDisabledAndToggled`

Created disabled, then [`setEnabled(true)`](../code/collider-component.md#collidercomponent-setenabled) turns it on.

<a id="collidercomponenttest-rejectsnonpositivesize"></a>
#### `ColliderComponentTest.RejectsNonPositiveSize`

Width 0 or height −1 throws `std::invalid_argument`.

<a id="collidercomponenttest-rejectsnonfinitevalues"></a>
#### `ColliderComponentTest.RejectsNonFiniteValues`

NaN or infinity in any of the four values throws.

<a id="collidercomponenttest-failedupdatekeepspreviousbounds"></a>
#### `ColliderComponentTest.FailedUpdateKeepsPreviousBounds`

A failing [`setLocalBounds`](../code/collider-component.md#collidercomponent-setlocalbounds) leaves all four old values in place.

<a id="collidersystem-tests"></a>

### ColliderSystem: overlap rules

<a id="collidersystemtest-overlappingboxesarereported"></a>
#### `ColliderSystemTest.OverlappingBoxesAreReported`

Boxes at (0,0) and (5,5), 10×10 each → exactly `{1,2}`.

<a id="collidersystemtest-touchingedgesisnotacollision"></a>
#### `ColliderSystemTest.TouchingEdgesIsNotACollision`

Right edge = left edge, and bottom edge = top edge → nothing.

<a id="collidersystemtest-touchingcornersisnotacollision"></a>
#### `ColliderSystemTest.TouchingCornersIsNotACollision`

Only corners meet → nothing.

<a id="collidersystemtest-tinyoverlappasttheedgeisacollision"></a>
#### `ColliderSystemTest.TinyOverlapPastTheEdgeIsACollision`

Overlap of 0.5 px → one pair.

<a id="collidersystemtest-separatedononeaxisisnotacollision"></a>
#### `ColliderSystemTest.SeparatedOnOneAxisIsNotACollision`

Overlapping on x but not y (and the reverse) → nothing.

<a id="collidersystemtest-containmentisacollision"></a>
#### `ColliderSystemTest.ContainmentIsACollision`

A small box fully inside a big one → `{1,2}`.

<a id="collidersystemtest-identicalboxescollide"></a>
#### `ColliderSystemTest.IdenticalBoxesCollide`

Two identical boxes → one pair.

<a id="collidersystemtest-workswithnegativepositions"></a>
#### `ColliderSystemTest.WorksWithNegativePositions`

Overlap at negative coordinates is found.

### ColliderSystem: offsets and current Transform

<a id="collidersystemtest-offsetisaddedtotransformposition"></a>
#### `ColliderSystemTest.OffsetIsAddedToTransformPosition`

Transform at 0 with offset 45 reaches a box at 50 → collision.

<a id="collidersystemtest-offsetcanmovecolliderawayfromsprite"></a>
#### `ColliderSystemTest.OffsetCanMoveColliderAwayFromSprite`

Same Transform, collider offset 100 down → no collision.

<a id="collidersystemtest-usescurrenttransformeverypass"></a>
#### `ColliderSystemTest.UsesCurrentTransformEveryPass`

Moving the Transform between calls changes the result each time.

### ColliderSystem: what gets skipped

<a id="collidersystemtest-disabledcollidersareskipped"></a>
#### `ColliderSystemTest.DisabledCollidersAreSkipped`

A disabled collider on top of another → nothing.

<a id="collidersystemtest-reenabledcolliderisdetectedagain"></a>
#### `ColliderSystemTest.ReenabledColliderIsDetectedAgain`

Turning it back on → one pair.

<a id="collidersystemtest-entitywithouttransformisskipped"></a>
#### `ColliderSystemTest.EntityWithoutTransformIsSkipped`

Collider but no Transform → ignored.

<a id="collidersystemtest-entitywithoutcolliderisskipped"></a>
#### `ColliderSystemTest.EntityWithoutColliderIsSkipped`

Transform but no collider → ignored.

<a id="collidersystemtest-nonfinitetransformisskipped"></a>
#### `ColliderSystemTest.NonFiniteTransformIsSkipped`

Transform `x` = infinity → ignored.

<a id="collidersystemtest-emptyscenereportsnothing"></a>
#### `ColliderSystemTest.EmptySceneReportsNothing`

No entities → empty result.

### ColliderSystem: pair reporting

<a id="collidersystemtest-singleentitynevercollideswithitself"></a>
#### `ColliderSystemTest.SingleEntityNeverCollidesWithItself`

One box alone → nothing.

<a id="collidersystemtest-eachpairisreportedonceininsertionorder"></a>
#### `ColliderSystemTest.EachPairIsReportedOnceInInsertionOrder`

Three overlapping boxes → exactly `{1,2}, {1,3}, {2,3}`.

<a id="collidersystemtest-onlyoverlappingpairsarereported"></a>
#### `ColliderSystemTest.OnlyOverlappingPairsAreReported`

A far-away box in the middle is left out → `{1,3}`.

<a id="collidersystemtest-repeateddetectiongivessameresult"></a>
#### `ColliderSystemTest.RepeatedDetectionGivesSameResult`

Calling twice gives the same answer (no hidden state).

<a id="collidersystemtest-newsceneonlyreportsitsownentities"></a>
#### `ColliderSystemTest.NewSceneOnlyReportsItsOwnEntities`

One system used on two scenes: each result only contains that scene's IDs.

## Running these tests

**Target:** `engine_tests`. **Filter:** `ctest --test-dir build -R Collider`. No SDL needed.

## Coverage gaps

Not tested: [`handleCollisions`](../code/game.md#handlecollisions) (it only logs) and performance with many entities.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
