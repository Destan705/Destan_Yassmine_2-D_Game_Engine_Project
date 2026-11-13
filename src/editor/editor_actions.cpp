#include "editor_actions.h"
#include "transform_component.h"
#include <cmath>

EditResult applyTransformEdit(Entity& entity, float x, float y, float width, float height){

    TransformComponent* transform = entity.getComponent<TransformComponent>();

    if(transform == nullptr||!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) || width <= 0 || !std::isfinite(height) || height <= 0 ){
        return EditResult::Rejected;
    }
    
    if(transform->x == x&&transform->y == y&&transform->width == width&&transform->height == height){
        return EditResult::NoChange;
    }

    transform->x = x;
    transform->y = y;
    transform->width = width;
    transform->height = height;
    
    return EditResult::Applied;
    
}
