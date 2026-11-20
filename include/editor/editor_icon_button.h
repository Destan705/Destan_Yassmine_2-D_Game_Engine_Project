#pragma once

#include "imgui.h"
#include <algorithm>

// DPI-aware, font-independent icons for the editor toolbar.
// Icons are drawn with Dear ImGui's existing draw list: no external font/assets.
namespace EditorIconButton {

enum class Icon { Play, Pause, Open, Save, SaveAs, Colliders };

inline bool draw(const char* id, Icon icon, const char* tooltip, bool active = false) {
    // FrameHeight includes DPI-scaled font and frame padding.
    const float buttonSize = std::max(ImGui::GetFrameHeight(), ImGui::GetFontSize() * 1.85f);
    const bool clicked = ImGui::Button(id, ImVec2(buttonSize, buttonSize));
    const ImVec2 topLeft = ImGui::GetItemRectMin();
    const ImVec2 bottomRight = ImGui::GetItemRectMax();
    const ImVec2 center{(topLeft.x + bottomRight.x) * 0.5f,
                        (topLeft.y + bottomRight.y) * 0.5f};
    const float radius = buttonSize * 0.225f;
    const float lineWidth = std::max(1.5f, ImGui::GetFontSize() * 0.135f);
    const ImU32 ink = ImGui::GetColorU32(ImGuiCol_Text);
    ImDrawList* list = ImGui::GetWindowDrawList();

    const auto point = [&](float x, float y) {
        return ImVec2(center.x + x * radius, center.y + y * radius);
    };
    const auto line = [&](float x0, float y0, float x1, float y1) {
        list->AddLine(point(x0, y0), point(x1, y1), ink, lineWidth);
    };

    switch (icon) {
    case Icon::Play:
        list->AddTriangleFilled(point(-0.55f, -0.92f), point(0.92f, 0.0f),
                                point(-0.55f, 0.92f), ink);
        break;
    case Icon::Pause:
        list->AddRectFilled(point(-0.75f, -0.88f), point(-0.22f, 0.88f), ink, radius * 0.09f);
        list->AddRectFilled(point(0.22f, -0.88f), point(0.75f, 0.88f), ink, radius * 0.09f);
        break;
    case Icon::Open:
        // Folder tab and open front flap.
        line(-1.0f, -0.67f, -0.30f, -0.67f);
        line(-0.30f, -0.67f, -0.10f, -0.34f);
        line(-0.10f, -0.34f, 0.90f, -0.34f);
        line(-1.0f, -0.67f, -1.0f, 0.78f);
        line(-1.0f, 0.78f, 0.74f, 0.78f);
        line(0.74f, 0.78f, 1.10f, -0.05f);
        line(1.10f, -0.05f, -0.63f, -0.05f);
        line(-0.63f, -0.05f, -1.0f, 0.78f);
        break;
    case Icon::Save:
    case Icon::SaveAs:
        // Floppy disk, with a plus badge for Save As.
        list->AddRect(point(-0.90f, -0.95f), point(0.90f, 0.95f), ink, radius * 0.12f, 0, lineWidth);
        list->AddRectFilled(point(-0.48f, -0.95f), point(0.48f, -0.23f), ink);
        list->AddRectFilled(point(-0.47f, 0.38f), point(0.47f, 0.95f), ink);
        if (icon == Icon::SaveAs) {
            const float cx = center.x + radius * 0.92f;
            const float cy = center.y + radius * 0.70f;
            const float b = radius * 0.55f;
            const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
            list->AddCircleFilled(ImVec2(cx, cy), b * 0.94f, bg);
            list->AddCircle(ImVec2(cx, cy), b * 0.94f, ink, 0, lineWidth);
            list->AddLine(ImVec2(cx - b * 0.49f, cy), ImVec2(cx + b * 0.49f, cy), ink, lineWidth);
            list->AddLine(ImVec2(cx, cy - b * 0.49f), ImVec2(cx, cy + b * 0.49f), ink, lineWidth);
        }
        break;
    case Icon::Colliders:
        // Eye icon: diagonal slash means collider outlines are hidden.
        list->AddBezierCubic(point(-1.05f, 0.0f), point(-0.40f, -0.94f),
                             point(0.40f, -0.94f), point(1.05f, 0.0f), ink, lineWidth);
        list->AddBezierCubic(point(-1.05f, 0.0f), point(-0.40f, 0.94f),
                             point(0.40f, 0.94f), point(1.05f, 0.0f), ink, lineWidth);
        list->AddCircleFilled(center, radius * 0.32f, ink);
        if (!active) {
            line(-0.96f, 1.04f, 0.96f, -1.04f);
        }
        break;
    }

    if (active) {
        const ImU32 highlight = ImGui::GetColorU32(ImGuiCol_CheckMark);
        const float inset = std::max(1.0f, lineWidth * 0.70f);
        list->AddRect(ImVec2(topLeft.x + inset, topLeft.y + inset),
                      ImVec2(bottomRight.x - inset, bottomRight.y - inset),
                      highlight, ImGui::GetStyle().FrameRounding, 0, lineWidth * 0.75f);
    }

    if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return clicked;
}

} // namespace EditorIconButton
