#pragma once
#include "behavior.h"

// Moves its entity with WASD. Reusable by any game.
class KeyboardMove : public Behavior {
public:
    explicit KeyboardMove(float speed = 200.0f);

    void update(float dt) override;

    float getSpeed() const { return speed; }
    // Throws std::invalid_argument if speed is negative or not finite.
    void setSpeed(float newSpeed);

private:
    float speed; // pixels per second
};