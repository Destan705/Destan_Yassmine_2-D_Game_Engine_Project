# Component — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/ecs/component.h`](../../include/ecs/component.h) and [`src/ecs/component.cpp`](../../src/ecs/component.cpp).

**In one sentence:** The base class for every piece of behaviour or data attached to an [`Entity`](entity.md): it has optional `update` and `render` hooks and knows its owner.

**Why it exists:** An `Entity` stores a list of `Component` pointers and calls `update`/`render` on each one without knowing the concrete type. Subclasses ([`Transform`](transform.md), [`Sprite`](sprite.md), [`AnimationComponent`](animation-component.md)) override only what they need.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | The common interface and the owner back-pointer |
| Used by | [Entity](entity.md) (calls hooks, sets owner); every concrete component |
| Depends on | Forward declarations of `Entity` and `Renderer` only (no SDL) |

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `Component` | Base class | [Class](#class-component) |
| `~Component()` | Virtual destructor | [Function](#component-destructor) |
| `update(float)` | Per-frame logic hook | [Function](#component-update) |
| `render(Renderer&)` | Per-frame drawing hook | [Function](#component-render) |
| `getOwner()` | Which entity owns me | [Function](#component-getowner) |
| `setOwner(Entity&)` | Set by Entity only | [Function](#component-setowner) |

## Class: `Component`

<a id="class-component"></a>

**Purpose:** One instance is one piece of an entity, such as its position, image or animation.

**How it's created and destroyed:** Created by [`Entity::addComponent<T>`](entity.md#entity-addcomponent) and owned by a `std::unique_ptr<Component>` inside that entity. It is destroyed when the entity is destroyed. The **virtual** destructor makes deleting through a `Component*` run the subclass destructor correctly.

**Friend:** `Entity` is a `friend`, so it alone can call the private `setOwner`.

**Important rules:** `owner` is `nullptr` until the component is added to an entity, then points at that entity for life. Components created outside an entity (as in some tests) keep `owner == nullptr`.

### Fields

| Field | Type / initial value | Meaning | Who changes it / ownership |
| --- | --- | --- | --- |
| <a id="component-owner"></a>`owner` | `Entity*`, `nullptr` | The entity this component belongs to | Set once by [`setOwner`](#component-setowner). **Non-owning** (the entity owns the component, not the other way round). |

## Functions

<a id="component-destructor"></a>
### `virtual Component::~Component()`

Defaulted in the `.cpp`. It exists so that subclasses are destroyed properly through base pointers. **No direct project-function calls.**

<a id="component-update"></a>
### `virtual void Component::update(float deltaTime)`

**What it does:** Nothing by default. Subclasses override it for per-frame logic. `deltaTime` is in seconds. **Called by:** [`Entity::update`](entity.md#entity-update). **Overridden by:** [`AnimationComponent::update`](animation-component.md#animationcomponent-update).

<a id="component-render"></a>
### `virtual void Component::render(Renderer& renderer)`

**What it does:** Nothing by default. Subclasses override it to draw. **Called by:** [`Entity::render`](entity.md#entity-render). **Overridden by:** [`Sprite::render`](sprite.md#sprite-render).

<a id="component-getowner"></a>
### `Entity* Component::getOwner() const`

**Returns:** the owning entity, or `nullptr` if not attached. **Used by:** [`Sprite::render`](sprite.md#sprite-render) and [`AnimationComponent::update`](animation-component.md#animationcomponent-update), which use it to find sibling components.

<a id="component-setowner"></a>
### `void Component::setOwner(Entity& entity)` (private)

Stores `&entity` in `owner`. **Called only by** [`Entity::addComponent`](entity.md#entity-addcomponent).

## Things worth knowing

- Unused parameters in the default `update`/`render` may produce compiler warnings. They're harmless.
- Components find each other through `getOwner()->getComponent<T>()`. That's how [Sprite reads Transform](sprite.md#sprite-render) and [AnimationComponent writes to Sprite](animation-component.md#animationcomponent-update).

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
