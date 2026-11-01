// each frame, reads transform and asks renderer to draw the image

#pragma once
#include "component.h"
#include "rendertypes.h"

class Sprite : public Component {
  public:
    explicit Sprite(textureHandle handle);
    void render(Renderer& renderer) override;

  private:
    textureHandle currentTexture = 0;
};