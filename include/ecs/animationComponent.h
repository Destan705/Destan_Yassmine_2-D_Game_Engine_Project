#pragma once
#include "component.h"
#include "rendertypes.h"
#include <vector>

class AnimationComponent : public Component {

  private:
    std::vector<Bounds> frames;   // array of frames
    float duration = 0.0f;        // frame duration
    bool repeat = true;           // decided whather to repeat the animation or not
    std::size_t currentFrame = 0; // which frame is currently being displayed
    float elapsed = 0.0f;         // how much time has passed since the last frame change
    bool paused = false;

  public:
    AnimationComponent(const std::vector<Bounds>& newFrames, float newDuration, bool newRepeat);
    void update(float dt) override;
    void pause();
    void resume();
    void restart();
};