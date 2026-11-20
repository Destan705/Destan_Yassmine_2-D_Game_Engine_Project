#pragma once

#include <cmath>
#include <optional>
#include <utility>

// Screen coordinates are ImGui logical pixels. Scene coordinates are render-target
// pixels, with (0, 0) at the top-left of the Scene image (not the SDL window).
// This type intentionally has no SDL or ImGui dependency so the mapping can be tested.
struct SceneViewportGeometry {
    float screenX = 0.0f;
    float screenY = 0.0f;
    float screenWidth = 0.0f;
    float screenHeight = 0.0f;
    int targetWidth = 0;
    int targetHeight = 0;

    std::optional<std::pair<float, float>> screenToScene(float x, float y) const {
        if (screenWidth <= 0.0f || screenHeight <= 0.0f ||
            targetWidth <= 0 || targetHeight <= 0 ||
            !std::isfinite(x) || !std::isfinite(y) ||
            !std::isfinite(screenX) || !std::isfinite(screenY) ||
            !std::isfinite(screenWidth) || !std::isfinite(screenHeight)) {
            return std::nullopt;
        }
        // Right/bottom boundaries are excluded: pixel indexes run [0, size).
        if (x < screenX || y < screenY ||
            x >= screenX + screenWidth || y >= screenY + screenHeight) {
            return std::nullopt;
        }
        return std::pair<float, float>{
            (x - screenX) * static_cast<float>(targetWidth) / screenWidth,
            (y - screenY) * static_cast<float>(targetHeight) / screenHeight
        };
    }
};
