#pragma once

#include "editor_state.h"
#include "rendertypes.h"
#include "scene.h"
#include <cstdint>
#include <vector>

class Renderer;

struct ColliderOutline {
    std::uint64_t entityId;
    Bounds bounds;
    Color color;
};

// Returns current-scene outlines with no renderer side effects and no retained pointers.
// Normal = green, overlapping = red, selected = yellow (highest priority).
std::vector<ColliderOutline> collectColliderOutlines(Scene& scene,
                                                      const EditorState& editorState);

// Draws after sprites and before ImGui, using the engine renderer's coordinates.
void drawColliderOverlay(Renderer& renderer, Scene& scene, const EditorState& editorState);
