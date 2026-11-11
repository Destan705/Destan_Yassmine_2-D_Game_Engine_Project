#include "inspector_panel.h"
#include "transform.h"
#include "imgui.h"
#include "editor_actions.h"



// Draws the Inspector window for the selected entity. Call once per frame between beginFrame() and render().
// scene: where the selected entity is looked up and edited.
// editorState: selection, pause state, and dirty flag (marked dirty when an edit is applied).
void InspectorPanel::draw(EditorState& editorState,Scene& scene ){
    
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

                    if(editorState.getSceneGeneration()!=lastSceneGeneration||editorState.getSelectedId()!=lastSelectedId){
                        
                        if(transform != nullptr){

                            editX = transform->x;
                            editY = transform->y;
                            editWidth = transform->width;
                            editHeight = transform->height;

                        }
                         
                        lastSceneGeneration=editorState.getSceneGeneration();
                        lastSelectedId = editorState.getSelectedId();
                    }

                                         if(transform != nullptr){
                            ImGui::SeparatorText("Transform");

                            ImGui::InputFloat("X", &editX);
                            ImGui::InputFloat("Y", &editY);
                            ImGui::InputFloat("width", &editWidth);
                            ImGui::InputFloat("height", &editHeight);
                       
                            bool frame = ImGui::Button("Apply");

                            if(frame){
                            EditResult editResult = applyTransformEdit(*selected,editX,editY,editWidth,editHeight);
                            if(editResult==EditResult::Applied){
                                editorState.markDirty();
                            }

                            }
                        }
                         
                }
                
            }
        }

   
       ImGui::End();
}
    
