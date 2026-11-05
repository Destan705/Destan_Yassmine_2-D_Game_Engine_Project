# Render types — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/rendering/rendertypes.h`](../../include/rendering/rendertypes.h) (header only, no `.cpp`).

**In one sentence:** Three tiny engine-owned types (`textureHandle`, `Color`, `Bounds`) that describe *what* to draw without mentioning SDL.

**Why it exists:** Game code, components and scenes need to talk about colours, rectangles and textures. If they used SDL types directly, every file would depend on SDL. These types keep SDL inside the [Renderer](renderer.md), which converts them to SDL types internally.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | Plain data types shared by rendering, components and game code |
| Gets information from | Nothing; it only defines types |
| Changes | Nothing |
| Used by | [Renderer](renderer.md), [Sprite](sprite.md), [AnimationComponent](animation-component.md), [platform](platform.md), [render API](render-api.md), [game](game.md) |
| Depends on | `<cstdint>` for `std::uint8_t` |

## Contents — jump to an explanation

| Symbol | What it means | Jump |
| --- | --- | --- |
| `textureHandle` | Number that identifies a loaded texture | [State & types](#rendertypes-texturehandle) |
| `Color` | An RGBA colour, 0–255 per channel | [Struct](#struct-color) |
| `Bounds` | A rectangle: position and size in pixels | [Struct](#struct-bounds) |

## State and types

<a id="state-and-types"></a>

| Name | Kind / type | Value or initialization | What it means / where used |
| --- | --- | --- | --- |
| <a id="rendertypes-texturehandle"></a>`textureHandle` | type alias for `unsigned int` | — | An index into the [Renderer's texture list](renderer.md#renderer-textures). `0` is reserved and always means "no texture". Returned by [`Renderer::LoadTexture`](renderer.md#renderer-loadtexture) and [`loadPlatformTexture`](platform.md#loadplatformtexture); stored by [`Sprite`](sprite.md#sprite-currenttexture). |

## Struct: `Color`

<a id="struct-color"></a>

**Purpose:** One colour with red, green, blue and alpha (opacity) channels.

**How it's created and destroyed:** Plain value type. Copy it freely. Every field defaults to `255`, so `Color{}` is **opaque white**.

### Fields

| Field | Type / initial value | Meaning and units | Who changes it / ownership |
| --- | --- | --- | --- |
| <a id="color-red"></a>`red` | `std::uint8_t`, `255` | Red amount, 0–255 | Whoever creates the value |
| <a id="color-green"></a>`green` | `std::uint8_t`, `255` | Green amount, 0–255 | Whoever creates the value |
| <a id="color-blue"></a>`blue` | `std::uint8_t`, `255` | Blue amount, 0–255 | Whoever creates the value |
| <a id="color-alpha"></a>`alpha` | `std::uint8_t`, `255` | Opacity, 0 = invisible, 255 = solid | Whoever creates the value |

**Watch out:** because the defaults are 255, `Color{0, 0, 255}` is blue with alpha 255, but `Color{}` is white, not black.

## Struct: `Bounds`

<a id="struct-bounds"></a>

**Purpose:** An axis-aligned rectangle. Used for **where to draw** (destination) and, for animations, **which part of an image to cut out** (source).

**How it's created and destroyed:** Plain value type. All fields default to `0.0f`.

### Fields

| Field | Type / initial value | Meaning and units | Who changes it / ownership |
| --- | --- | --- | --- |
| <a id="bounds-x"></a>`x` | `float`, `0.0f` | Left edge, in pixels | Creator |
| <a id="bounds-y"></a>`y` | `float`, `0.0f` | Top edge, in pixels (y grows downward, SDL convention) | Creator |
| <a id="bounds-w"></a>`w` | `float`, `0.0f` | Width, in pixels | Creator |
| <a id="bounds-h"></a>`h` | `float`, `0.0f` | Height, in pixels | Creator |

**Meaning depends on use:**

- As a **destination** (`DrawTexture`'s `bounds`, `DrawFilledRect`), units are **screen pixels**.
- As a **source** (`DrawTexture`'s `src`, animation frames), units are **pixels inside the texture image**.

## Dependencies and interactions

**Includes:** only `<cstdint>`. This header deliberately does **not** include SDL.

## Things worth knowing

- **Naming:** the file comment says `RenderTypes.hpp`, but the real file is `rendertypes.h`.
- **No validation:** `Bounds` accepts negative or zero sizes. Validation happens where it's used (see [`Renderer::DrawTexture`](renderer.md#renderer-drawtexture)).
- `Transform` uses `width`/`height` while `Bounds` uses `w`/`h`. [`Sprite::render`](sprite.md#sprite-render) converts between them.

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
