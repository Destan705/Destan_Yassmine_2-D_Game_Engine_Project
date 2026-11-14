#include "transform_component.h"
#include "sprite_component.h"
#include "entity.h"
#include "renderer.h"

SpriteComponent::SpriteComponent(textureHandle handle){
    currentTexture = handle;
}

void SpriteComponent::render(Renderer& renderer){

    Entity* owner= getOwner();
    if(owner==nullptr){
        return;
    }
    
    TransformComponent* transform = owner->getComponent<TransformComponent>();
    if(transform==nullptr){
        return;
    }

    Bounds bounds{transform->x, transform->y, transform->width, transform->height};
    renderer.DrawTexture(currentTexture, bounds, src);
}

void SpriteComponent::sourceSet(std::optional<Bounds> source) {
    src = source;
}

textureHandle SpriteComponent::getTexture() const {
    return currentTexture;
}

void SpriteComponent::setTexture(textureHandle newTexture) {
    currentTexture = newTexture;
}