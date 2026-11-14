#pragma once
#include "entity.h"
#include <string>

class AssetManager;

enum class EditResult{

Applied,
NoChange,
Rejected

};

EditResult applyTransformEdit(Entity& entity, float x, float y, float width, float height);

EditResult applyColliderEdit(Entity& entity, float offsetX, float offsetY, float width, float height, bool enabled);

EditResult applySpriteEdit(Entity& entity, AssetManager& assetManager, const std::string& relativePath);
