# Sprite — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/ecs/sprite.h`](../../include/ecs/sprite.h) and [`src/ecs/sprite.cpp`](../../src/ecs/sprite.cpp).

**In one sentence:** A component that draws one texture at its entity's [`Transform`](transform.md), optionally cropping it to a source rectangle.

**Why it exists:** It connects the ECS to the [Renderer](renderer.md). The texture is a **handle** (a number), so the sprite never owns GPU memory.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | Drawing one texture per frame |
| Gets information from | Its owner's `Transform` (destination) and its own `src` (source crop) |
| Changes | Nothing except the screen; `src` changes via `sourceSet` |
| Used by | [Entity::render](entity.md#entity-render); [AnimationComponent](animation-component.md) sets its source; [SceneSerializer::save](scene-serializer.md#sceneserializer-save) reads its handle |
| Depends on | [Component](component.md), [Transform](transform.md), [Renderer](renderer.md), [render types](render-types.md) |

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `Sprite` | Draws a texture | [Class](#class-sprite) |
| `Sprite(textureHandle)` | Create with a texture | [Function](#sprite-sprite) |
| `render(Renderer&)` | Draw it | [Function](#sprite-render) |
| `sourceSet(optional<Bounds>)` | Choose which part of the image to draw | [Function](#sprite-sourceset) |
| `getTexture()` | Read the texture handle | [Function](#sprite-gettexture) |

## Class: `Sprite`

<a id="class-sprite"></a>

**Parent class:** [`Component`](component.md#class-component); overrides `render`.

**How it's created:** `entity.addComponent<Sprite>(handle)`.

**Important rules:** needs a `Transform` on the **same entity** to draw anything.

### Fields

| Field | Type / initial value | Meaning | Who changes it |
| --- | --- | --- | --- |
| <a id="sprite-currenttexture"></a>`currentTexture` | `textureHandle`, `0` | Which texture to draw | Constructor only |
| <a id="sprite-src"></a>`src` | `std::optional<Bounds>`, `std::nullopt` | Source crop in texture pixels; empty = whole image | [`sourceSet`](#sprite-sourceset), usually from [`AnimationComponent::update`](animation-component.md#animationcomponent-update) |

## Functions

<a id="sprite-sprite"></a>
### `explicit Sprite::Sprite(textureHandle handle)`

Stores `handle` in `currentTexture`. It isn't validated here; a bad handle is caught later by [`DrawTexture`](renderer.md#renderer-drawtexture). **No direct project-function calls.**

<a id="sprite-render"></a>
### `void Sprite::render(Renderer& renderer)` (override)

**What it does:** Draws the texture at the owner's Transform.

**What happens, in order**

1. Gets the owner with [`getOwner()`](component.md#component-getowner). If null, it returns.
2. Finds the owner's [`Transform`](transform.md) with [`getComponent<Transform>()`](entity.md#entity-getcomponent). If missing, it returns (draws nothing, no warning).
3. Builds `Bounds{x, y, width, height}` from the Transform.
4. Calls [`renderer.DrawTexture(currentTexture, bounds, src)`](renderer.md#renderer-drawtexture).

**Calls other project functions**

| Calls | Why this call is needed here | Where |
| --- | --- | --- |
| `Component::getOwner` | Reach the entity to find the Transform | [getOwner](component.md#component-getowner) |
| `Entity::getComponent<Transform>` | Get the destination position and size | [getComponent](entity.md#entity-getcomponent) |
| `Renderer::DrawTexture` | Actually draw (with optional crop) | [DrawTexture](renderer.md#renderer-drawtexture) |

**Errors:** none thrown. Missing pieces are silently skipped. **Related tests:** [SpriteTest.MissingTransformSafelySkipsDrawing](../tests/test-transform-sprite.md#spritetest-missingtransformsafelyskipsdrawing).

<a id="sprite-sourceset"></a>
### `void Sprite::sourceSet(std::optional<Bounds> source)`

**What it does:** Replaces `src`. Pass a `Bounds` to crop, or `std::nullopt` to draw the whole image again.

**Why/when it's used:** Every frame by [`AnimationComponent::update`](animation-component.md#animationcomponent-update), which passes the current frame's rectangle.

**Validation:** none here. [`DrawTexture`](renderer.md#renderer-drawtexture) rejects rectangles outside the image.

<a id="sprite-gettexture"></a>
### `textureHandle Sprite::getTexture() const`

Returns `currentTexture`. **Why it exists:** [`SceneSerializer::save`](scene-serializer.md#sceneserializer-save) turns the handle back into a path with [`AssetManager::getPath`](asset-manager.md#assetmanager-getpath), because files store paths, never handles. **No direct project-function calls.**

## Things worth knowing

- **Low coupling:** `Sprite` doesn't know about animation. The dependency goes one way only (`AnimationComponent` → `Sprite`).
- `sprite.cpp` isn't fully formatted with `.clang-format` yet.
- **Tests:** cropped drawing through a sprite is checked pixel-by-pixel in [test_animation_component](../tests/test-animation-component.md); `getTexture` is used throughout [test_scene_serializer](../tests/test-scene-serializer.md).

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
