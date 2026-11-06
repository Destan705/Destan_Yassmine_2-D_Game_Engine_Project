#include "component.h"

Component::~Component() = default;

void Component::update(float deltaTime) {
    // Is overidden in derived classes, when they have update behaviour
}

void Component::render(Renderer& renderer) {
    // Is overidden in derived classes, when they have render behaviour
}

Entity* Component::getOwner() const {
    return owner;
}

void Component::setOwner(Entity& entity) {
    owner = &entity;
}