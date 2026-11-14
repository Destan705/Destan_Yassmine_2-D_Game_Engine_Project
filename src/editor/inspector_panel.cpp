#include "inspector_panel.h"
#include "transform_component.h"
#include "imgui.h"
#include "editor_actions.h"
#include "collider_component.h"
#include "animation_component.h"
#include "sprite_component.h"
#include <stdexcept>



// Draws the Inspector window for the selected entity. Call once per frame between beginFrame() and render().
// scene: where the selected entity is looked up and edited.
// editorState: selection, pause state, and dirty flag (marked dirty when an edit is applied).
void InspectorPanel::draw(EditorState& editorState,Scene& scene,const AssetManager& assetManager ){
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Inspector")){
            if(editorState.getSelectedId()==std::nullopt){
                ImGui::Text("Nothing selected");

            }else{

                std::uint64_t id = editorState.getSelectedId().value();

                Entity* selected = nullptr;

                for(auto& entity : scene.getEntities()){
                    if(entity->getId() == id){
                        selected=entity.get();
                        break;
                    }

                }

                if(selected==nullptr){
                
                    ImGui::Text("Nothing selected");

                }else{
                    ImGui::Text("Entity %llu",static_cast<unsigned long long>(id));
                        
                    TransformComponent* transformComponent =selected->getComponent<TransformComponent>();
                    ColliderComponent* colliderComponent = selected->getComponent<ColliderComponent>();

                        bool pausedNow = editorState.getPause();
                        bool justPaused = pausedNow && !lastPaused;

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

                            lastSceneGeneration = editorState.getSceneGeneration();
                            lastSelectedId = editorState.getSelectedId();
                        }

                        lastPaused = pausedNow;

                            ImGui::SeparatorText("Transform");
                            if(transformComponent != nullptr){

                                ImGui::BeginDisabled( !editorState.getPause() );
    
                                ImGui::InputFloat("X", &editX);
                                ImGui::InputFloat("Y", &editY);
                                ImGui::InputFloat("width", &editWidth);
                                ImGui::InputFloat("height", &editHeight);
                        
                                bool frame = ImGui::Button("Apply##Transform");

                                ImGui::EndDisabled();

                                if(frame){
                                EditResult editResult = applyTransformEdit(*selected,editX,editY,editWidth,editHeight);
                                if(editResult==EditResult::Applied){
                                    editorState.markDirty();
                                }

                                }

                        } else {
                            ImGui::Text("No Transform Component");
                        }

                                                    
                        ImGui::SeparatorText("Collider");
                        if (colliderComponent != nullptr){
                                
                            ImGui::BeginDisabled( !editorState.getPause() );

                            ImGui::InputFloat("Offset X: ", &editColliderOffsetX);
                            ImGui::InputFloat("Offset Y: ", &editColliderOffsetY);
                            ImGui::InputFloat("Width: ", &editColliderWidth);
                            ImGui::InputFloat("Height: ", &editColliderHeight);
                            ImGui::Checkbox("Enabled", &editColliderEnabled);

                            bool frame = ImGui::Button("Apply##Collider");

                            ImGui::EndDisabled();

                            if(frame){
                            EditResult editResult = applyColliderEdit(*selected,editColliderOffsetX,editColliderOffsetY,editColliderWidth,editColliderHeight, editColliderEnabled);
                            if(editResult==EditResult::Applied){
                                editorState.markDirty();
                            }

                            }
                        } else {
                            ImGui::Text("No Collider Component");
                        }

                        AnimationComponent* animationComponent = selected->getComponent<AnimationComponent>();

                        if(animationComponent !=nullptr){
                            ImGui::SeparatorText("Animation");

                            ImGui::Text("Amount of frames: %zu", animationComponent->getFrames().size());
                            ImGui::Text("Duration: %.2f", animationComponent->getDuration());
                            ImGui::Text("Repeat: %s", animationComponent->getRepeat()? "Yes" : "No");
                        }
                        
                        SpriteComponent* sprite = selected->getComponent<SpriteComponent>();

                        if(sprite != nullptr){
                            ImGui::SeparatorText("Sprite");
                            try{
                            ImGui::Text("Path: %s", assetManager.getPath(sprite->getTexture()).c_str());
                            }catch(const std::out_of_range&){
                                ImGui::Text("Path: (unknown)");
                            }

                        }
                         
                }
                
            }
        }

   
       ImGui::End();
}
    
