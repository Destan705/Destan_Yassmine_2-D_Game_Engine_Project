// PHASE4_WIDGET_POLISH_V1
#include "hierarchy_panel.h"
#include "editor_layout.h"
#include "editor_state.h"
#include "scene.h"
#include "imgui.h"
#include "editor_actions.h"
#include "editor_theme.h"
#include "entity.h"
#include <cstdio>
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
    if (ImGui::Begin("Entities", nullptr, EditorLayout::panelFlags)) {

        if (activeScene == nullptr) {
            ImGui::TextDisabled("No active scene");
        } else {
            std::string sceneName = activeScene->getSceneName();
            ImGui::TextDisabled("ACTIVE SCENE");
            ImGui::TextColored(EditorTheme::accentText(), "%s", sceneName.c_str());
            ImGui::Spacing();

            // Create: only while paused.
            ImGui::BeginDisabled(!editorState.getPause());
            EditorTheme::pushPrimaryButton();
            if (ImGui::Button("Create")) {
                createEntityAction(*activeScene, editorState);
            }
            EditorTheme::popButton();
            ImGui::EndDisabled();

            ImGui::Separator();
            ImGui::Spacing();
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

                    std::string entityLabel = entity->getName();

                    ImGui::PushID(uniqueId.c_str());

                    bool isSelected =
                        editorState.getSelectedId() == entityId;

                    if (ImGui::Selectable(entityLabel.c_str(), isSelected)) {
                        editorState.select(entityId);
                    }

                    ImGui::PopID();
                }
            }

            // Rename / Delete: only when something is selected.
            auto selected = editorState.getSelectedId();
            if (selected.has_value()) {
                // Find the selected entity in this scene.
                Entity* target = nullptr;
                for (const auto& entity : entities) {
                    if (entity->getId() == selected.value()) {
                        target = entity.get();
                        break;
                    }
                }

                if (target != nullptr) {
                    // Refill the box when the selection changes.
                    if (renameTargetId != selected || editorState.getSceneGeneration() != lastSceneGeneration) {
                        std::snprintf(renameBuffer, sizeof(renameBuffer), "%s",
                                      target->getName().c_str());
                        renameTargetId = selected;
                        lastSceneGeneration = editorState.getSceneGeneration();
                    }
                    

                    ImGui::Separator();
                    ImGui::BeginDisabled(!editorState.getPause());

                    ImGui::TextDisabled("Entity name");
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputText("##RenameEntity", renameBuffer, sizeof(renameBuffer));
                    if (ImGui::Button("Rename")) {
                        renameEntityAction(*target, renameBuffer, editorState);
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Delete")) {
                        ImGui::OpenPopup("Confirm delete");
                    }

                    ImGui::EndDisabled();

                    // "Are you sure?" window. Only requests the delete;
                    // the engine removes the entity at the end of the frame.
                    if (ImGui::BeginPopupModal("Confirm delete", nullptr,
                                               ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Delete \"%s\"?", target->getName().c_str());

                        if (ImGui::Button("Delete")) {
                            editorState.requestDelete(target->getId());
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel")) {
                            ImGui::CloseCurrentPopup(); // nothing happens
                        }
                        ImGui::EndPopup();
                    }
                }
            } else {
                renameTargetId.reset();
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