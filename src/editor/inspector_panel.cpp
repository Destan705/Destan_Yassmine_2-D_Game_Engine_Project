
#include "inspector_panel.h"
#include "editor_layout.h"
#include "transform_component.h"
#include "imgui.h"
#include "editor_actions.h"
#include "editor_theme.h"
#include "collider_component.h"
#include "animation_component.h"
#include "sprite_component.h"
#include <stdexcept>
#include <cstdio>
#include <exception>
#include <cmath>
#include <cstddef>



// PHASE4_WIDGET_POLISH_V1: consistent full-width property fields.
namespace {
void propertyFloat(const char* caption, const char* id, float* value) {
    ImGui::TextDisabled("%s", caption);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputFloat(id, value);
}

void propertyText(const char* caption, const char* id, char* value, std::size_t capacity) {
    ImGui::TextDisabled("%s", caption);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText(id, value, capacity);
}
}

// Draws the Inspector window for the selected entity.
// Call once per frame between beginFrame() and render().
// scene: where the selected entity is looked up and edited.
// editorState: selection, pause state, and dirty flag.
// assetManager: retrieves current texture paths and loads replacement textures.

void InspectorPanel::draw(EditorState& editorState, Scene& scene, AssetManager& assetManager) {

    if (ImGui::Begin("Inspector", nullptr, EditorLayout::panelFlags)) {
        if (editorState.getSelectedId() == std::nullopt) {
            ImGui::Spacing();
            ImGui::TextDisabled("Select an entity to edit its properties.");
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
                ImGui::Spacing();
            ImGui::TextDisabled("Select an entity to edit its properties.");
                lastSelectedId.reset();

            } else {
                ImGui::TextColored(EditorTheme::accentText(), "Entity %llu",
                                    static_cast<unsigned long long>(id));
                ImGui::Text("Name: %s", selected->getName().c_str());
                ImGui::Separator();

                TransformComponent* transformComponent = selected->getComponent<TransformComponent>();
                ColliderComponent* colliderComponent = selected->getComponent<ColliderComponent>();
                SpriteComponent* spriteComponent = selected->getComponent<SpriteComponent>();
                AnimationComponent* animationComponent = selected->getComponent<AnimationComponent>();


                enum class ComponentRemoval {
                    None,
                    Sprite,
                    Collider,
                    Animation
                };

                ComponentRemoval removalRequested = ComponentRemoval::None;


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
                    colliderAddFeedback.clear();

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

                    propertyFloat("X", "##TransformX", &editX);
                    propertyFloat("Y", "##TransformY", &editY);
                    propertyFloat("Width", "##TransformWidth", &editWidth);
                    propertyFloat("Height", "##TransformHeight", &editHeight);

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

                    propertyFloat("Offset X", "##ColliderOffsetX", &editColliderOffsetX);
                    propertyFloat("Offset Y", "##ColliderOffsetY", &editColliderOffsetY);
                    propertyFloat("Width", "##ColliderWidth", &editColliderWidth);
                    propertyFloat("Height", "##ColliderHeight", &editColliderHeight);
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

                    ImGui::BeginDisabled(!editorState.getPause());

                    if (ImGui::Button("Remove Collider")) {
                        removalRequested = ComponentRemoval::Collider;
                    }

                    ImGui::EndDisabled();

                } else {
                    ImGui::TextDisabled("No Collider Component");

                    ImGui::BeginDisabled(!editorState.getPause());

                    if (ImGui::Button("Add Collider")) {
                        // Use Transform dimensions when valid.
                        // Otherwise, use 32x32.
                        float width = transformComponent != nullptr &&  std::isfinite(transformComponent->width) && transformComponent->width > 0.0f
                                ? transformComponent->width
                                : 32.0f;

                        float height = transformComponent != nullptr && std::isfinite(transformComponent->height) && transformComponent->height > 0.0f
                                ? transformComponent->height
                                : 32.0f;

                        try {
                            selected->addComponent<ColliderComponent>(
                                0.0f, 0.0f, width, height, true
                            );

                            // Prepare the Inspector fields for the next frame.
                            editColliderOffsetX = 0.0f;
                            editColliderOffsetY = 0.0f;
                            editColliderWidth = width;
                            editColliderHeight = height;
                            editColliderEnabled = true;

                            editorState.markDirty();
                            colliderAddFeedback.clear();

                        } catch (const std::exception& e) {
                            colliderAddFeedback =
                                std::string("Could not add Collider: ") + e.what();
                        }
                    }

                    ImGui::EndDisabled();

                    if (!colliderAddFeedback.empty()) {
                        ImGui::TextWrapped("%s", colliderAddFeedback.c_str());
                    }
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


                    ImGui::BeginDisabled(!editorState.getPause());

                    if (ImGui::Button("Remove Animation")) {
                        removalRequested = ComponentRemoval::Animation;
                    }

                    ImGui::EndDisabled();

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

                    propertyText("Texture path", "##EditSpritePath", editSpritePath, sizeof(editSpritePath));

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


                    ImGui::BeginDisabled(!editorState.getPause());

                    if (ImGui::Button("Remove Sprite")) {
                        if (animationComponent != nullptr) {
                            spriteEditFeedback = "Remove Animation before removing Sprite.";
                        } else {
                            removalRequested = ComponentRemoval::Sprite;
                        }
                    }

                    ImGui::EndDisabled();

         
                } else {
                    ImGui::TextDisabled("No Sprite Component");

                    ImGui::BeginDisabled(!editorState.getPause());

                    ImGui::TextDisabled("Texture path");
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputTextWithHint("##AddSpritePath", "test.bmp", editSpritePath, sizeof(editSpritePath));

                    if (ImGui::Button("Add Sprite")) {
                        try {
                            // Load an existing texture or retrieve its cached handle
                            textureHandle handle = assetManager.loadTexture(std::string(editSpritePath));

                            // Attach the component to the selected entity
                            selected->addComponent<SpriteComponent>(handle);

                            editorState.markDirty();
                            spriteEditFeedback = "Sprite added successfully.";

                        } catch (const std::exception& e) {
                            spriteEditFeedback =
                                std::string("Could not add Sprite: ") + e.what();
                        }
                    }

                    ImGui::EndDisabled();

                    if (!spriteEditFeedback.empty()) {
                        ImGui::TextWrapped("%s", spriteEditFeedback.c_str());
                    }
                }



                if (editorState.getPause()) {
                    bool removed = false;

                    switch (removalRequested) {
                        case ComponentRemoval::Collider:
                            removed = selected->removeComponent<ColliderComponent>();
                            break;

                        case ComponentRemoval::Sprite:
                            if (selected->getComponent<AnimationComponent>() == nullptr) {
                                removed = selected->removeComponent<SpriteComponent>();
                            }
                            break;

                        case ComponentRemoval::Animation:
                            // Restore normal full-texture rendering 
                            if (spriteComponent != nullptr) {
                                spriteComponent->sourceSet(std::nullopt);
                            }

                            removed = selected->removeComponent<AnimationComponent>();
                            break;

                        case ComponentRemoval::None:
                            break;
                    }

                    if (removed) {
                        editorState.markDirty();
                    }
                }

            }
        }
    }

    ImGui::End();
}
