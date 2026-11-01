#pragma once
#include "component.h"

class Transform : public Component {
  public:
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};
