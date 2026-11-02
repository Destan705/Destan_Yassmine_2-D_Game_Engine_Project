#include "colliderComponent.h"
#include <cmath>
#include <stdexcept>


ColliderComponent::ColliderComponent(float newOffsetX, float newOffsetY, float newWidth, float newHeight, bool newEnabled) : enabled(newEnabled) {
    setLocalBounds(newOffsetX, newOffsetY, newWidth, newHeight);
}

void ColliderComponent::setLocalBounds(float newOffsetX, float newOffsetY, float newWidth, float newHeight) {
    if (!std::isfinite(newOffsetX) || !std::isfinite(newOffsetY) ) {
        throw std::invalid_argument("Offset values must be finite");
    }

    if (!std::isfinite(newWidth) || newWidth <= 0) {
        throw std::invalid_argument("Width must be finite and greater than zero");
    }

    if (!std::isfinite(newHeight) ||newHeight <= 0) {
        throw std::invalid_argument("Height must be finite and greater than zero");
    }
    offsetX = newOffsetX;
    offsetY = newOffsetY;
    width =  newWidth;
    height =  newHeight;
}

float ColliderComponent::getOffsetX() const {
    return offsetX;
}


float ColliderComponent::getOffsetY() const {
    return offsetY;
}

float ColliderComponent::getWidth() const {
    return width;
}

float ColliderComponent::getHeight() const {
    return height;
}

bool ColliderComponent::isEnabled() const {
    return enabled;
}

void ColliderComponent::setEnabled(bool enabled) {
    this->enabled = enabled;
}