#include "inspector_panel.h"
#include "transform.h"
#include "imgui.h"
#include "editor_actions.h"
#include "colliderComponent.h"
#include "animationComponent.h"
#include "sprite.h"
#include <stdexcept>



// Draws the Inspector window for the selected entity. Call once per frame between beginFrame() and render().
// scene: where the selected entity is looked up and edited.
// editorState: selection, pause state, and dirty flag (marked dirty when an edit is applied).
void InspectorPanel::draw(EditorState& editorState,Scene& scene,const AssetManager& assetManager ){
    
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
                        
                    Transform* transform =selected->getComponent<Transform>();

                        bool pausedNow = editorState.getPause();
                        bool justPaused = pausedNow && !lastPaused;

                        if (editorState.getSceneGeneration() != lastSceneGeneration
                            || editorState.getSelectedId() != lastSelectedId
                            || justPaused) {

                            if (transform != nullptr) {
                                editX = transform->x;
                                editY = transform->y;
                                editWidth = transform->width;
                                editHeight = transform->height;
                            }

                            lastSceneGeneration = editorState.getSceneGeneration();
                            lastSelectedId = editorState.getSelectedId();
                        }

                        lastPaused = pausedNow;


                            if(transform != nullptr){
                            ImGui::SeparatorText("Transform");

                            ImGui::BeginDisabled( !editorState.getPause() );
  
                            ImGui::InputFloat("X", &editX);
                            ImGui::InputFloat("Y", &editY);
                            ImGui::InputFloat("width", &editWidth);
                            ImGui::InputFloat("height", &editHeight);
                       
                            bool frame = ImGui::Button("Apply");

                            ImGui::EndDisabled();

                            if(frame){
                            EditResult editResult = applyTransformEdit(*selected,editX,editY,editWidth,editHeight);
                            if(editResult==EditResult::Applied){
                                editorState.markDirty();
                            }

                            }

                        }

                                                    
                        ColliderComponent* colliderComponent = selected->getComponent<ColliderComponent>();

                        if (colliderComponent != nullptr){
                        ImGui::SeparatorText("Collider");
                                
                            ImGui::Text("Offset X: %.2f", colliderComponent->getOffsetX());
                            ImGui::Text("Offset Y: %.2f", colliderComponent->getOffsetY());
                            ImGui::Text("Width: %.2f", colliderComponent->getWidth());
                            ImGui::Text("Heigh: %.2f", colliderComponent->getHeight());
                            ImGui::Text("Enabled: %s",  colliderComponent->isEnabled() ? "Yes" : "No");
                        }

                        AnimationComponent* animationComponent = selected->getComponent<AnimationComponent>();

                        if(animationComponent !=nullptr){
                            ImGui::SeparatorText("Animation");

                            ImGui::Text("Amount of frames: %zu", animationComponent->getFrames().size());
                            ImGui::Text("Duration: %.2f", animationComponent->getDuration());
                            ImGui::Text("Repeat: %s", animationComponent->getRepeat()? "Yes" : "No");
                        }
                        
                        Sprite* sprite = selected->getComponent<Sprite>();

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
    
