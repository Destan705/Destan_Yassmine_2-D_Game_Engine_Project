#include "stats_panel.h"
#include "imgui.h"

void StatsPanel::draw(const StatsSnapshot& stats) {

    if (ImGui::Begin("Statistics")) {

        ImGui::Text("PERFORMANCE");
        ImGui::Separator();

        if (stats.fps.has_value()) {
            ImGui::Text("FPS: %.1f", stats.fps.value());
        } else {
            ImGui::TextDisabled("FPS: N/A");
        }

        if (stats.averageFrameTimeMs.has_value()) {
            ImGui::Text("Average frame time: %.2f ms", stats.averageFrameTimeMs.value());
        } else {
            ImGui::TextDisabled("Average frame time: N/A");
        }

        ImGui::Spacing();

        ImGui::Text("SCENE");
        ImGui::Separator();

        ImGui::Text("Entities: %zu", stats.entityCount);
        ImGui::Text("Valid colliders: %zu", stats.validColliderCount);
        ImGui::Text("Overlapping pairs: %zu", stats.overlapPairCount);

        ImGui::Spacing();

        ImGui::Text("RENDERING");
        ImGui::Separator();

        ImGui::Text("Cached textures: %zu", stats.cachedTextureCount);
        ImGui::Text("Sprite draw submissions: %zu", stats.spriteDrawCount);
    }

    ImGui::End();
}