//each frame, reads transform and asks renderer to draw the image

#pragma once 
#include  "component.h"
#include "rendertypes.h"
#include <optional>

class Sprite: public Component{
    public:
    explicit Sprite(textureHandle handle);
    void render(Renderer& renderer) override;
    void sourceSet(std::optional<Bounds> source);
  
    private:
    textureHandle currentTexture=0;
    std::optional<Bounds> src = std::nullopt;
   

};