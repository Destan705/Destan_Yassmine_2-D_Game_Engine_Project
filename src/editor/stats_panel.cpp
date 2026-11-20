// PHASE4_WIDGET_POLISH_V1
#include "stats_panel.h"
#include "editor_layout.h"
#include "imgui.h"
#include "editor_theme.h"

void StatsPanel::draw(const StatsSnapshot& stats) {
    if (ImGui::Begin("Statistics")) {
        auto label = [](const char* text) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("%s", text);
            ImGui::TableSetColumnIndex(1);
        };

        ImGui::TextColored(EditorTheme::accentText(), "PERFORMANCE");
        ImGui::Separator();
        if (ImGui::BeginTable("##PerformanceMetrics", 2, ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch, 0.58f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.42f);
            label("FPS");
            if (stats.fps) { ImGui::Text("%.1f", *stats.fps); }
            else { ImGui::TextDisabled("N/A"); }
            label("Frame time");
            if (stats.averageFrameTimeMs) { ImGui::Text("%.2f ms", *stats.averageFrameTimeMs); }
            else { ImGui::TextDisabled("N/A"); }
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::TextColored(EditorTheme::accentText(), "SCENE");
        ImGui::Separator();
        if (ImGui::BeginTable("##SceneMetrics", 2, ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch, 0.65f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.35f);
            label("Entities"); ImGui::Text("%zu", stats.entityCount);
            label("Colliders"); ImGui::Text("%zu", stats.validColliderCount);
            label("Overlaps"); ImGui::Text("%zu", stats.overlapPairCount);
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::TextColored(EditorTheme::accentText(), "RENDERING");
        ImGui::Separator();
        if (ImGui::BeginTable("##RenderingMetrics", 2, ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch, 0.65f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.35f);
            label("Cached textures"); ImGui::Text("%zu", stats.cachedTextureCount);
            label("Sprites drawn"); ImGui::Text("%zu", stats.spriteDrawCount);
            ImGui::EndTable();
        }
    }
    ImGui::End();
}