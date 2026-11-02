#include "animationComponent.h"
#include "entity.h"
#include "sprite.h"
#include <cmath>
#include <stdexcept>

AnimationComponent::AnimationComponent(const std::vector<Bounds>& newFrames, float newDuration,
                                       bool newRepeat) {
    if (newFrames.empty()) {
        throw std::invalid_argument("Empty frames");
    }

    if (newDuration <= 0 || !std::isfinite(newDuration)) {
        throw std::invalid_argument("Invalid duration");
    }

    frames = newFrames;
    duration = newDuration;
    repeat = newRepeat;
}

void AnimationComponent::update(float dt) {
    if (paused) {
        return;
    }
    elapsed += dt;

    while (elapsed >= duration) {
        elapsed -= duration;
        currentFrame++;

        if (currentFrame >= frames.size()) {
            if (repeat) {
                currentFrame = 0;
            } else {
                currentFrame = frames.size() - 1;
            }
        }
    }

    Entity* owner = getOwner();

    if (owner == nullptr) {
        return;
    }

    Sprite* sprite = owner->getComponent<Sprite>();

    if (sprite == nullptr) {
        return;
    }

    sprite->sourceSet(frames[currentFrame]);
}

void AnimationComponent::restart() {
    currentFrame = 0;
    elapsed = 0.0f;
}
void AnimationComponent::pause() {
    paused = true;
}
void AnimationComponent::resume() {
    paused = false;
}