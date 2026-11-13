
#include "hierarchy_panel.h"
#include "editor_state.h"
#include "scene.h"
#include "imgui.h"
#include <string>
#include <cstdint>

void HierarchyPanel::draw(EditorState& editorState, Scene* activeScene) {

    // Validate selection before drawing, even if the panel is collapsed.
    if (activeScene == nullptr) {
        editorState.unselect();
    } else {
        auto selectedId = editorState.getSelectedId();

        if (selectedId.has_value()) {
            bool matchEntity = false;

            for (const auto& entity : activeScene->getEntities()) {
                if (entity->getId() == selectedId.value()) {
                    matchEntity = true;
                    break;
                }
            }

            if (!matchEntity) {
                editorState.unselect();
            }
        }
    }

    // Draw the Hierarchy panel.
    if (ImGui::Begin("Entities")) {

        if (activeScene == nullptr) {
            ImGui::TextDisabled("No active scene");
        } else {
            std::string sceneName = activeScene->getSceneName();
            ImGui::Text("%s", sceneName.c_str());

            uint64_t sceneGeneration = editorState.getSceneGeneration();
            const auto& entities = activeScene->getEntities();

            if (entities.empty()) {
                ImGui::TextDisabled("No entities in this scene");
            } else {
                for (const auto& entity : entities) {

                    uint64_t entityId = entity->getId();

                    std::string uniqueId =
                        std::to_string(sceneGeneration) + ":" +
                        std::to_string(entityId);

                    std::string entityLabel =
                        "Entity " + std::to_string(entityId);

                    ImGui::PushID(uniqueId.c_str());

                    bool isSelected =
                        editorState.getSelectedId() == entityId;

                    if (ImGui::Selectable(entityLabel.c_str(), isSelected)) {
                        editorState.select(entityId);
                    }

                    ImGui::PopID();
                }
            }

            // Clear selection when clicking empty panel space.
            bool clearSelection =
                ImGui::IsWindowHovered() &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
                !ImGui::IsAnyItemHovered();

            if (clearSelection) {
                editorState.unselect();
            }
        }
    }

    ImGui::End();
}
