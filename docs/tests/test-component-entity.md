# test_component_entity.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/ecs/test_component_entity.cpp`](../../tests/ecs/test_component_entity.cpp)

**In one sentence:** It checks the basic [`Entity`](../code/entity.md) contract: ID, add, find, owner, reject duplicates, and forward `update`.

## Setup and shared helpers

| Helper | What it is |
| --- | --- |
| `FakeComponent`, `FakeComponent2` | Empty `Component` subclasses (`FakeComponent2` isn't used) |
| `UpdateTracker` | A component whose `update` sets `updated = true` |

## Individual tests

<a id="entitytest-addcomponentreturnsthecomponent"></a>
### `EntityTest.AddComponentReturnsTheComponent`

The reference from [`addComponent`](../code/entity.md#entity-addcomponent) is the same object [`getComponent`](../code/entity.md#entity-getcomponent) finds.

<a id="entitytest-addingduplicatecomponentthrows"></a>
### `EntityTest.AddingDuplicateComponentThrows`

A second `addComponent<FakeComponent>()` throws `std::logic_error`.

<a id="entitytest-getmissingcomponentreturnsnullptr"></a>
### `EntityTest.GetMissingComponentReturnsNullptr`

`getComponent` on an empty entity returns `nullptr`.

<a id="entitytest-getidreturnsconstructorid"></a>
### `EntityTest.GetIdReturnsConstructorId`

`Entity(42).getId()` is 42 ([`getId`](../code/entity.md#entity-getid)).

<a id="entitytest-addedcomponentknowsitsowner"></a>
### `EntityTest.AddedComponentKnowsItsOwner`

After `addComponent`, the component's [`getOwner()`](../code/component.md#component-getowner) is the entity.

<a id="entitytest-updatereachescomponents"></a>
### `EntityTest.UpdateReachesComponents`

[`entity.update(0.1f)`](../code/entity.md#entity-update) reaches the component's override.

## Coverage gaps

No test for `render` forwarding at entity level (covered at scene level) or the exact-type rule for subclasses.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
