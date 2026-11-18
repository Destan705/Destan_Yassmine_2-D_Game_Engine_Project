#pragma once
#include "entity.h"
#include "scene.h"
#include "editor_state.h"

enum class EditResult{

Applied,
NoChange,
Rejected

};

EditResult applyTransformEdit(Entity& entity, float x, float y, float width, float height);
std::uint64_t createEntityAction(Scene& scene, EditorState& editorState);
EditResult renameEntityAction(Entity& entity, const std::string& newName, EditorState& editorState);