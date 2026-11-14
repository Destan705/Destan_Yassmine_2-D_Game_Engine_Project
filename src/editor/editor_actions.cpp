#include "editor_actions.h"
#include "transform_component.h"
#include "collider_component.h"
#include "sprite_component.h"
#include "animation_component.h"
#include "asset_manager.h"
#include <cmath>
#include <exception>
#include <utility>

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

EditResult applySpriteEdit(Entity& entity, AssetManager& assetManager, const std::string& relativePath) {

    SpriteComponent* sprite = entity.getComponent<SpriteComponent>();


    if (sprite == nullptr) {
        return EditResult::Rejected;
    }


    try {
        textureHandle candidateTexture = assetManager.loadTexture(relativePath);

        if (sprite->getTexture() == candidateTexture) {
            return EditResult::NoChange;
        }

        AnimationComponent* animation = entity.getComponent<AnimationComponent>();

        std::pair<int, int> originalDimensions = assetManager.getOriginalDimensions(candidateTexture);

        if (originalDimensions.first <= 0 || originalDimensions.second <= 0){
            return EditResult::Rejected;
        }

        if (animation != nullptr){
            for (const auto& frame : animation->getFrames()) {
                bool checkFinite = std::isfinite(frame.x) && std::isfinite(frame.y) && std::isfinite(frame.w) && std::isfinite(frame.h);
                bool checkBounds = frame.x >= 0 && frame.y >= 0 && frame.w > 0 && frame.h > 0;
                bool checkBoundaries = (frame.x + frame.w <= originalDimensions.first) && (frame.y + frame.h  <= originalDimensions.second);

                if (!checkFinite || !checkBounds || !checkBoundaries) {
                    return EditResult::Rejected;
                }
            }
        }

        sprite->setTexture(candidateTexture);

        return EditResult::Applied;

    } catch (const std::exception&) {
        return EditResult::Rejected;
    }
}