#pragma once

#include "imgui.h"

// Geometry of a permanent editor panel (in main-viewport coordinates).
struct EditorPanelRect {
    ImVec2 position{};
    ImVec2 size{};
};

struct EditorLayoutRegions {
    EditorPanelRect toolbar;
    EditorPanelRect hierarchy;
    EditorPanelRect statistics;
    EditorPanelRect scene;
    EditorPanelRect console;
    EditorPanelRect inspector;
    EditorPanelRect sceneFile;
    EditorPanelRect overlay;
    EditorPanelRect status;
};

class EditorLayout {
public:
    // Recompute each frame to react to window resizing.
    static EditorLayoutRegions calculate(const ImGuiViewport& viewport);

    // Apply before the corresponding panel calls ImGui::Begin().
    static void place(const EditorPanelRect& rect);

    static constexpr ImGuiWindowFlags panelFlags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    static constexpr ImGuiWindowFlags barFlags =
        panelFlags |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;
};
