# Render API (Phase 1) — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/rendering/render_api.h`](../../include/rendering/render_api.h). **Implemented in** [`src/platform/platform.cpp`](../../src/platform/platform.cpp).

**In one sentence:** Four free functions that let code draw without knowing about `Renderer` or SDL: begin a frame, draw a rectangle, draw the start-up texture, and present.

**Why it exists:** It was the Phase 1 drawing interface for the old cat/chef game (removed in Phase 3), and it let `game_tests` swap in **fake** versions. It is kept and still tested, but nothing in the engine uses it now.

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `beginFrame()` | Clear to black | [Docs](platform.md#beginframe) |
| `drawFilledRect(Color, Bounds)` | Draw a rectangle | [Docs](platform.md#drawfilledrect) |
| `drawTexture()` | Draw the start-up texture | [Docs](platform.md#drawtexture) |
| `presentFrame()` | Show the frame | [Docs](platform.md#presentframe) |

The full function explanations live on the [Platform page](platform.md), because that's where the code is.

## Things worth knowing

- **No include guard:** `render_api.h` has no `#pragma once`. Including it twice in one file would be harmless here (it only has declarations), but it's inconsistent with the other headers.
- **Current status:** the Phase 2/3 engine loop draws through [`renderPlatform`](platform.md#renderplatform) instead. This API is used only by tests ([CanRenderFrameThroughPublicRenderingApi](../tests/test-platform.md#platformtest-canrenderframethroughpublicrenderingapi)). It is a candidate for removal.

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
