#include "editor_theme.h"

namespace {
ImVec4 rgb(int r, int g, int b, int a = 255) {
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

const ImVec4 baseBackground = rgb(22, 21, 26);
const ImVec4 panelBackground = rgb(31, 29, 36);
const ImVec4 raisedBackground = rgb(43, 40, 48);
const ImVec4 borderColor = rgb(66, 60, 70);
const ImVec4 accent = rgb(116, 108, 159);
const ImVec4 accentHovered = rgb(137, 128, 180);
const ImVec4 accentActive = rgb(99, 92, 141);
const ImVec4 softText = rgb(228, 224, 233);
const ImVec4 secondaryText = rgb(171, 165, 180);
}

void EditorTheme::apply(ImGuiStyle& style) {
    // Keep deliberately modest rounding: Figma's compact desktop-editor feel.
    style.WindowPadding = ImVec2(10.0f, 9.0f);
    style.FramePadding = ImVec2(8.0f, 5.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(7.0f, 5.0f);
    style.CellPadding = ImVec2(8.0f, 6.0f);
    style.IndentSpacing = 18.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;
    style.WindowRounding = 3.0f;
    style.ChildRounding = 3.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 7.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.DisabledAlpha = 0.57f;

    auto& c = style.Colors;
    c[ImGuiCol_Text] = softText;
    c[ImGuiCol_TextDisabled] = secondaryText;
    c[ImGuiCol_WindowBg] = panelBackground;
    c[ImGuiCol_ChildBg] = rgb(27, 26, 32);
    c[ImGuiCol_PopupBg] = rgb(37, 34, 42);
    c[ImGuiCol_Border] = borderColor;
    c[ImGuiCol_BorderShadow] = rgb(0, 0, 0, 0);
    c[ImGuiCol_FrameBg] = raisedBackground;
    c[ImGuiCol_FrameBgHovered] = rgb(58, 53, 65);
    c[ImGuiCol_FrameBgActive] = rgb(67, 60, 76);
    c[ImGuiCol_TitleBg] = rgb(37, 34, 43);
    c[ImGuiCol_TitleBgActive] = rgb(52, 45, 62);
    c[ImGuiCol_TitleBgCollapsed] = rgb(37, 34, 43);
    c[ImGuiCol_MenuBarBg] = baseBackground;
    c[ImGuiCol_ScrollbarBg] = rgb(25, 24, 30);
    c[ImGuiCol_ScrollbarGrab] = rgb(69, 64, 76);
    c[ImGuiCol_ScrollbarGrabHovered] = rgb(87, 79, 97);
    c[ImGuiCol_ScrollbarGrabActive] = rgb(112, 102, 124);
    c[ImGuiCol_CheckMark] = rgb(198, 192, 225);
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accentHovered;
    c[ImGuiCol_Button] = accent;
    c[ImGuiCol_ButtonHovered] = accentHovered;
    c[ImGuiCol_ButtonActive] = accentActive;
    c[ImGuiCol_Header] = rgb(67, 60, 87);
    c[ImGuiCol_HeaderHovered] = rgb(85, 75, 106);
    c[ImGuiCol_HeaderActive] = rgb(105, 92, 135);
    c[ImGuiCol_Separator] = borderColor;
    c[ImGuiCol_SeparatorHovered] = accentHovered;
    c[ImGuiCol_SeparatorActive] = accent;
    c[ImGuiCol_ResizeGrip] = rgb(111, 102, 141, 80);
    c[ImGuiCol_ResizeGripHovered] = rgb(134, 123, 175, 160);
    c[ImGuiCol_ResizeGripActive] = accent;
    c[ImGuiCol_Tab] = rgb(46, 42, 54);
    c[ImGuiCol_TabHovered] = rgb(93, 83, 119);
    c[ImGuiCol_TabSelected] = rgb(73, 63, 95);
    c[ImGuiCol_TextSelectedBg] = rgb(111, 98, 146, 115);
    c[ImGuiCol_TableHeaderBg] = raisedBackground;
    c[ImGuiCol_TableBorderStrong] = borderColor;
    c[ImGuiCol_TableBorderLight] = rgb(53, 49, 59);
    c[ImGuiCol_TableRowBg] = rgb(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt] = rgb(255, 255, 255, 7);
}

ImVec4 EditorTheme::accentText() { return rgb(194, 184, 237); }
ImVec4 EditorTheme::mutedText() { return secondaryText; }
ImVec4 EditorTheme::warningText() { return rgb(234, 189, 116); }
ImVec4 EditorTheme::playbackText(bool paused) {
    return paused ? secondaryText : rgb(136, 211, 166);
}
ImVec4 EditorTheme::logText(logger_level severity) {
    switch (severity) {
    case logger_level::Warning: return rgb(229, 181, 105);
    case logger_level::Error: return rgb(236, 124, 130);
    case logger_level::Info: return softText;
    }
    return softText;
}

void EditorTheme::pushPlaybackButton(bool paused) {
    const ImVec4 normal = paused ? rgb(60, 112, 95) : rgb(138, 91, 86);
    const ImVec4 hovered = paused ? rgb(73, 137, 114) : rgb(163, 108, 100);
    const ImVec4 active = paused ? rgb(49, 96, 82) : rgb(116, 76, 73);
    ImGui::PushStyleColor(ImGuiCol_Button, normal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
}
void EditorTheme::pushPrimaryButton() {
    ImGui::PushStyleColor(ImGuiCol_Button, accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, accentHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, accentActive);
}
void EditorTheme::popButton() { ImGui::PopStyleColor(3); }
