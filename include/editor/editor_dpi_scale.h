#pragma once

#include <algorithm>
#include <cmath>

// Editor-only layout policy. No SDL or ImGui dependency, so it can be tested.
namespace EditorDpi {

inline float normalizedScale(float requested, float previous = 1.0f) {
    if (!std::isfinite(requested) || requested <= 0.0f) {
        requested = previous;
    }
    if (!std::isfinite(requested) || requested <= 0.0f) {
        requested = 1.0f;
    }
    return std::clamp(requested, 0.75f, 4.0f);
}

inline bool scaleChanged(float previous, float next) {
    return !std::isfinite(previous) || std::fabs(previous - next) > 0.01f;
}

// File and View menus remain accessible on compact layouts.
inline bool compactToolbar(float availableWidth, float dpi) {
    return availableWidth < 680.0f * normalizedScale(dpi);
}

} // namespace EditorDpi
