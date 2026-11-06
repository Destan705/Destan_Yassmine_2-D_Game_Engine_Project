#include "entity.h"

Entity::Entity(std::uint64_t id)
    : id(id) { // Using a member initializer list to initialize the id member variable with the
               // provided id parameter
}

std::uint64_t Entity::getId() const {
    return id;
}

void Entity::update(float deltaTime) {
    for (const auto& component : components) {
        component->update(deltaTime);
    }
}

void Entity::render(Renderer& renderer) {
    for (const auto& component : components) {
        component->render(renderer);
    }
}