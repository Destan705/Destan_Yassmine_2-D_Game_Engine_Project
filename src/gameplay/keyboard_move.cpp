#include "keyboard_move.h"
#include "entity.h"
#include "transform_component.h"
#include "input.h"
#include <cmath>
#include <stdexcept>

KeyboardMove::KeyboardMove(float speed){
    setSpeed(speed);
}

void KeyboardMove::setSpeed(float newSpeed){
    if(newSpeed < 0||!std::isfinite(newSpeed)){
        throw std::invalid_argument("Invalid argument the speed is supposed to be postive and finite.");
    }
    speed = newSpeed;
}

void KeyboardMove::update(float dt){
    Entity* entity = getOwner();
    if(entity==nullptr){
        return;
    }

    auto* transform = entity->getComponent<TransformComponent>();
    if(transform==nullptr){
        return;
    }

    float movement = speed * dt;

     // Left
        if (isKeyHeld(Key::A)) {
            transform->x -= movement;
        }

        // Right
        if (isKeyHeld(Key::D)) {
            transform->x += movement;
        }

        // Up
        if (isKeyHeld(Key::W)) {
            transform->y -= movement;
        }

        // Down
        if (isKeyHeld(Key::S)) {
            transform->y += movement;
        }

}