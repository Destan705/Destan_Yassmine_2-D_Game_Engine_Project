
#include "inspector_panel.h"
#include "transform_component.h"
#include "imgui.h"
#include "editor_actions.h"
#include "collider_component.h"
#include "animation_component.h"
#include "sprite_component.h"
#include <stdexcept>
#include <cstdio>
#include <exception>

// Draws the Inspector window for the selected entity.
// Call once per frame between beginFrame() and render().
// scene: where the selected entity is looked up and edited.
// editorState: selection, pause state, and dirty flag.
// assetManager: retrieves current texture paths and loads replacement textures.

void InspectorPanel::draw(EditorState& editorState, Scene& scene, AssetManager& assetManager) {
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Inspector")) {
        if (editorState.getSelectedId() == std::nullopt) {
            ImGui::Text("Nothing selected");
            lastSelectedId.reset();

        } else {
            std::uint64_t id = editorState.getSelectedId().value();
            Entity* selected = nullptr;

            for (auto& entity : scene.getEntities()) {
                if (entity->getId() == id) {
                    selected = entity.get();
                    break;
                }
            }

            if (selected == nullptr) {
                ImGui::Text("Nothing selected");
                lastSelectedId.reset();

            } else {
                ImGui::Text("Entity %llu", static_cast<unsigned long long>(id));

                TransformComponent* transformComponent = selected->getComponent<TransformComponent>();
                ColliderComponent* colliderComponent = selected->getComponent<ColliderComponent>();
                SpriteComponent* spriteComponent = selected->getComponent<SpriteComponent>();
                AnimationComponent* animationComponent = selected->getComponent<AnimationComponent>();

                bool pausedNow = editorState.getPause();
                bool justPaused = pausedNow && !lastPaused;

                // Refresh editable values when the selection or scene changes,
                // or when the game transitions from running to paused.
                if (editorState.getSceneGeneration() != lastSceneGeneration
                    || editorState.getSelectedId() != lastSelectedId
                    || justPaused) {

                    if (transformComponent != nullptr) {
                        editX = transformComponent->x;
                        editY = transformComponent->y;
                        editWidth = transformComponent->width;
                        editHeight = transformComponent->height;
                    }

                    if (colliderComponent != nullptr) {
                        editColliderOffsetX = colliderComponent->getOffsetX();
                        editColliderOffsetY = colliderComponent->getOffsetY();
                        editColliderWidth = colliderComponent->getWidth();
                        editColliderHeight = colliderComponent->getHeight();
                        editColliderEnabled = colliderComponent->isEnabled();
                    }

                    spriteEditFeedback.clear();

                    if (spriteComponent != nullptr) {
                        try {
                            int editSpritePathResult = std::snprintf(
                                editSpritePath,
                                sizeof(editSpritePath),
                                "%s",
                                assetManager.getPath(spriteComponent->getTexture()).c_str()
                            );

                            if (editSpritePathResult < 0
                                || editSpritePathResult >= static_cast<int>(sizeof(editSpritePath))) {

                                editSpritePath[0] = '\0';
                                spriteEditFeedback = "Unable to copy current texture path.";
                            }

                        } catch (const std::exception&) {
                            editSpritePath[0] = '\0';
                            spriteEditFeedback = "Unable to retrieve current texture path.";
                        }

                    } else {
                        editSpritePath[0] = '\0';
                    }

                    lastSceneGeneration = editorState.getSceneGeneration();
                    lastSelectedId = editorState.getSelectedId();
                }

                lastPaused = pausedNow;

                // -------------------- Transform --------------------

                ImGui::SeparatorText("Transform");

                if (transformComponent != nullptr) {
                    ImGui::BeginDisabled(!editorState.getPause());

                    ImGui::InputFloat("X", &editX);
                    ImGui::InputFloat("Y", &editY);
                    ImGui::InputFloat("Width", &editWidth);
                    ImGui::InputFloat("Height", &editHeight);

                    bool frame = ImGui::Button("Apply##Transform");

                    ImGui::EndDisabled();

                    if (frame) {
                        EditResult editResult = applyTransformEdit(
                            *selected,
                            editX,
                            editY,
                            editWidth,
                            editHeight
                        );

                        if (editResult == EditResult::Applied) {
                            editorState.markDirty();
                        }
                    }

                } else {
                    ImGui::Text("No Transform Component");
                }

                // -------------------- Collider --------------------

                ImGui::SeparatorText("Collider");

                if (colliderComponent != nullptr) {
                    ImGui::BeginDisabled(!editorState.getPause());

                    ImGui::InputFloat("Offset X", &editColliderOffsetX);
                    ImGui::InputFloat("Offset Y", &editColliderOffsetY);
                    ImGui::InputFloat("Width##Collider", &editColliderWidth);
                    ImGui::InputFloat("Height##Collider", &editColliderHeight);
                    ImGui::Checkbox("Enabled", &editColliderEnabled);

                    bool frame = ImGui::Button("Apply##Collider");

                    ImGui::EndDisabled();

                    if (frame) {
                        EditResult editResult = applyColliderEdit(
                            *selected,
                            editColliderOffsetX,
                            editColliderOffsetY,
                            editColliderWidth,
                            editColliderHeight,
                            editColliderEnabled
                        );

                        if (editResult == EditResult::Applied) {
                            editorState.markDirty();
                        }
                    }

                } else {
                    ImGui::Text("No Collider Component");
                }

                // -------------------- Animation --------------------

                if (animationComponent != nullptr) {
                    ImGui::SeparatorText("Animation");

                    ImGui::Text(
                        "Amount of frames: %zu",
                        animationComponent->getFrames().size()
                    );

                    ImGui::Text(
                        "Duration: %.2f",
                        animationComponent->getDuration()
                    );

                    ImGui::Text(
                        "Repeat: %s",
                        animationComponent->getRepeat() ? "Yes" : "No"
                    );
                }

                // -------------------- Sprite --------------------

                ImGui::SeparatorText("Sprite");

                if (spriteComponent != nullptr) {

                    // Display the actual texture currently used by the Sprite.
                    try {
                        ImGui::Text(
                            "Current texture: %s",
                            assetManager.getPath(spriteComponent->getTexture()).c_str()
                        );

                    } catch (const std::out_of_range&) {
                        ImGui::Text("Current texture: (unknown)");
                    }

                    // Editable texture path.
                    ImGui::BeginDisabled(!editorState.getPause());

                    ImGui::InputText("Path", editSpritePath, sizeof(editSpritePath));

                    bool frame = ImGui::Button("Apply##Sprite");

                    ImGui::EndDisabled();

                    if (frame) {
                        EditResult editResult = applySpriteEdit(
                            *selected,
                            assetManager,
                            editSpritePath
                        );

                        switch (editResult) {
                            case EditResult::Applied:
                                editorState.markDirty();
                                spriteEditFeedback = "Texture updated successfully.";
                                break;

                            case EditResult::NoChange:
                                spriteEditFeedback = "No changes made.";
                                break;

                            case EditResult::Rejected:
                                spriteEditFeedback = "Invalid path or incompatible texture.";
                                break;
                        }
                    }

                    // Keep displaying feedback after the Apply button is clicked.
                    if (!spriteEditFeedback.empty()) {
                        ImGui::TextUnformatted(spriteEditFeedback.c_str());
                    }

                } else {
                    ImGui::Text("No Sprite Component");
                }
            }
        }
    }

    ImGui::End();
}
