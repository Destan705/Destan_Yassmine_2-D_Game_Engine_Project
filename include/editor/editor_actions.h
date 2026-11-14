#pragma once
#include "entity.h"

enum class EditResult{

Applied,
NoChange,
Rejected

};

EditResult applyTransformEdit(Entity& entity, float x, float y, float width, float height);

EditResult applyColliderEdit(Entity& entity, float offsetX, float offsetY, float width, float height, bool enabled);

