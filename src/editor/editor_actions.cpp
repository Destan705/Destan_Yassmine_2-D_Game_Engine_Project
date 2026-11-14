#include "editor_actions.h"
#include "transform_component.h"
#include "collider_component.h"
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

EditResult applyColliderEdit(Entity& entity, float offsetX, float offsetY, float width, float height, bool enabled) {
    
    ColliderComponent* collider = entity.getComponent<ColliderComponent>();

    if (collider == nullptr ||  !std::isfinite(offsetX) || !std::isfinite(offsetY) || !std::isfinite(width) || width <= 0 || !std::isfinite(height) || height <= 0 ){
        return EditResult::Rejected;
    }

    if (collider->getOffsetX() == offsetX && collider->getOffsetY() == offsetY && collider->getWidth() == width && collider->getHeight() == height && collider->isEnabled() == enabled){
        return EditResult::NoChange;
    }

    collider->setLocalBounds(offsetX, offsetY, width, height);
    collider->setEnabled(enabled);

    return EditResult::Applied;
}