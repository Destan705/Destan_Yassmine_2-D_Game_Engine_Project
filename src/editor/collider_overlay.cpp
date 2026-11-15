#include "collider_overlay.h"
#include "transform_component.h"
#include "collider_component.h"
#include <cmath>


void drawColliderOverlay(Renderer& renderer, Scene& scene, const EditorState& editorState){
    if (editorState.getShowColliders()){
   for( auto& entity: scene.getEntities()){
    TransformComponent* transform = entity->getComponent<TransformComponent>();
    if(transform != nullptr){
    
    ColliderComponent* collider = entity->getComponent<ColliderComponent>();

        if(collider != nullptr && collider->isEnabled()){
            
            float offsetX= collider->getOffsetX();
            float offsetY= collider->getOffsetY();
           
            float x_Entity= transform->x;
            float y_Entity= transform->y;

            float x= offsetX+x_Entity;
            float y= offsetY+y_Entity;

            float w =  collider->getWidth();
            float h =  collider->getHeight();


            if(std::isfinite(x)&&std::isfinite(y)&&std::isfinite(w)&&std::isfinite(h)){
            Bounds box{ x, y, w, h };

            Color color = Color{0, 255, 0, 255};


            for(auto& collision : editorState.getCollisions()){
                if(collision.first==entity->getId()||collision.second==entity->getId()){
                    color = Color{255, 0, 0, 255}; 
                }
               
            }

            if(editorState.getSelectedId() == entity->getId()){
               color = Color{255, 255, 0, 255};
            }

            renderer.DrawRectOutline(color, box);
        }

        }
    }
    
   }
}

}
