#pragma once
#include "component.h"


class ColliderComponent : public Component {
private:
    float offsetX;
    float offsetY;
    float width;
    float height;
    bool enabled;

public:
    ColliderComponent(float offsetX, float offsetY, float width, float height, bool enabled = true);
    float getOffsetX() const;
    float getOffsetY() const;
    float getWidth() const;
    float getHeight() const;
    bool isEnabled() const;
    void setLocalBounds(float offsetX, float offsetY, float width, float height);
    void setEnabled(bool enabled);
};