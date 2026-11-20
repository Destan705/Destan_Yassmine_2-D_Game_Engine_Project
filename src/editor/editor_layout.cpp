#include "editor_layout.h"

#include <algorithm>

namespace {
EditorPanelRect rect(float x, float y, float width, float height) {
    return {{x, y}, {std::max(1.0f, width), std::max(1.0f, height)}};
}
} // namespace

EditorLayoutRegions EditorLayout::calculate(const ImGuiViewport& viewport) {
    // Geometry is in ImGui coordinates (pixels on Windows, points on macOS).
    // Scale fixed sizes so text and controls fit at 200% display scaling.
    const float dpi = std::clamp(ImGui::GetStyle().FontScaleDpi, 0.75f, 4.0f);
    const float width = std::max(1.0f, viewport.WorkSize.x);
    const float height = std::max(1.0f, viewport.WorkSize.y);
    const float gap = std::min(8.0f * dpi, std::min(width, height) / 24.0f);
    const float margin = gap;

    const float workX = viewport.WorkPos.x + margin;
    const float workY = viewport.WorkPos.y + margin;
    const float workW = std::max(1.0f, width - 2.0f * margin);
    const float workH = std::max(1.0f, height - 2.0f * margin);

    const float toolbarH = std::min(66.0f * dpi, workH * 0.16f);
    const float statusH = std::min(26.0f * dpi, workH * 0.08f);
    const float bodyY = workY + toolbarH + gap;
    const float bodyH = std::max(1.0f, workH - toolbarH - statusH - 2.0f * gap);
    const float statusY = bodyY + bodyH + gap;

    // Leave space for the Scene area even on smaller windows.
    const float minCenterW = std::min(260.0f * dpi, workW * 0.4f);
    const float sideBudget = std::max(0.0f, workW - 2.0f * gap - minCenterW);
    const float leftW = std::min(std::clamp(workW * 0.22f, 150.0f * dpi, 300.0f * dpi), sideBudget * 0.48f);
    const float rightW = std::min(std::clamp(workW * 0.27f, 200.0f * dpi, 360.0f * dpi), sideBudget - leftW);
    const float centerW = std::max(1.0f, workW - leftW - rightW - 2.0f * gap);
    const float leftX = workX;
    const float centerX = leftX + leftW + gap;
    const float rightX = centerX + centerW + gap;

    // Left side: Hierarchy above Statistics.
    const float leftSplit = std::max(1.0f, bodyH - gap);
    const float statsH = std::min(std::clamp(leftSplit * 0.32f, 90.0f * dpi, 240.0f * dpi), leftSplit * 0.48f);
    const float hierarchyH = std::max(1.0f, leftSplit - statsH);

    // Middle: Scene above Console. The Scene window is a TEMPORARY
    // transparent region until off-screen rendering is added in step 2.
    const float middleSplit = std::max(1.0f, bodyH - gap);
    const float consoleH = std::min(std::clamp(middleSplit * 0.29f, 120.0f * dpi, 260.0f * dpi), middleSplit * 0.5f);
    const float sceneH = std::max(1.0f, middleSplit - consoleH);

    // Right side: Inspector, scene file operations, collider overlay toggle.
    const float rightSplit = std::max(1.0f, bodyH - 2.0f * gap);
    const float overlayH = std::min(76.0f * dpi, rightSplit * 0.18f);
    const float sceneFileH = std::min(std::clamp(rightSplit * 0.33f, 120.0f * dpi, 230.0f * dpi), rightSplit * 0.45f);
    const float inspectorH = std::max(1.0f, rightSplit - overlayH - sceneFileH);

    EditorLayoutRegions regions{};
    regions.toolbar = rect(workX, workY, workW, toolbarH);
    regions.status = rect(workX, statusY, workW, statusH);
    regions.hierarchy = rect(leftX, bodyY, leftW, hierarchyH);
    regions.statistics = rect(leftX, bodyY + hierarchyH + gap, leftW, statsH);
    regions.scene = rect(centerX, bodyY, centerW, sceneH);
    regions.console = rect(centerX, bodyY + sceneH + gap, centerW, consoleH);
    // Scene file and collider toggles moved to the top toolbar.
    // The Inspector now uses all available right-hand space.
    regions.inspector = rect(rightX, bodyY, rightW, bodyH);
    regions.sceneFile = rect(rightX, bodyY + inspectorH + gap, rightW, sceneFileH);
    regions.overlay = rect(rightX, bodyY + inspectorH + gap + sceneFileH + gap, rightW, overlayH);
    return regions;
}

void EditorLayout::place(const EditorPanelRect& region) {
    ImGui::SetNextWindowPos(region.position, ImGuiCond_Always);
    ImGui::SetNextWindowSize(region.size, ImGuiCond_Always);
}
