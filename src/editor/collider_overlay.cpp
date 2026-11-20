#include "collider_overlay.h"
#include "collider_component.h"
#include "renderer.h"
#include "transform_component.h"

#include <cmath>

std::vector<ColliderOutline> collectColliderOutlines(Scene& scene,
                                                      const EditorState& editorState) {
    std::vector<ColliderOutline> outlines;
    if (!editorState.getShowColliders()) return outlines;

    for (const auto& entity : scene.getEntities()) {
        const auto* transform = entity->getComponent<TransformComponent>();
        const auto* collider = entity->getComponent<ColliderComponent>();
        if (!transform || !collider || !collider->isEnabled()) continue;

        const float x = transform->x + collider->getOffsetX();
        const float y = transform->y + collider->getOffsetY();
        const float w = collider->getWidth();
        const float h = collider->getHeight();
        if (!std::isfinite(x) || !std::isfinite(y) ||
            !std::isfinite(w) || !std::isfinite(h) ||
            w <= 0.0f || h <= 0.0f ||
            !std::isfinite(x + w) || !std::isfinite(y + h)) continue;

        Color color{0, 255, 0, 255};
        for (const auto& pair : editorState.getCollisions()) {
            if (pair.first == entity->getId() || pair.second == entity->getId()) {
                color = Color{255, 0, 0, 255};
                break;
            }
        }
        if (editorState.getSelectedId() == entity->getId()) {
            color = Color{255, 255, 0, 255};
        }
        outlines.push_back({entity->getId(), Bounds{x, y, w, h}, color});
    }
    return outlines;
}

void drawColliderOverlay(Renderer& renderer, Scene& scene, const EditorState& editorState) {
    for (const auto& outline : collectColliderOutlines(scene, editorState)) {
        renderer.DrawRectOutline(outline.color, outline.bounds);
    }
}
