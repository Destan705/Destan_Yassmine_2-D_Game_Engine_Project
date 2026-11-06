// each frame, reads transform and asks renderer to draw the image

#pragma once
#include "component.h"
#include "rendertypes.h"
#include <optional>

class SpriteComponent : public Component {

  public:
    explicit SpriteComponent(textureHandle handle);
    void render(Renderer& renderer) override;
    void sourceSet(std::optional<Bounds> source);
    textureHandle getTexture() const;

  private:
    textureHandle currentTexture = 0;
    std::optional<Bounds> src = std::nullopt;
};
