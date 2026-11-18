#include "editor_actions.h"
#include "transform_component.h"
#include <cmath>
#include <string>

EditResult applyTransformEdit(Entity& entity, float x, float y, float width, float height){

    TransformComponent* transform = entity.getComponent<TransformComponent>();

    if(transform == nullptr||!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) || width <= 0 || !std::isfinite(height) || height <= 0 ){
        return EditResult::Rejected;
    }
    
    if(transform->x == x&&transform->y == y&&transform->width == width&&transform->height == height){
        return EditResult::NoChange;
    }

    transform->x = x;
    transform->y = y;
    transform->width = width;
    transform->height = height;
    
    return EditResult::Applied;
    
}

std::uint64_t createEntityAction(Scene& scene, EditorState& editorState){

    Entity& entity =scene.createEntity();

    entity.addComponent<TransformComponent>();

    entity.setName("Entity " + std::to_string(entity.getId()));

    editorState.select(entity.getId());

    editorState.markDirty();

    return entity.getId();

}

EditResult renameEntityAction(Entity& entity, const std::string& newName, EditorState& editorState) {
    // Characters we treat as "blank" at the start or end of a name.
    const std::string whitespace = " \t\n\r";

    // Position of the first non-blank character.
    // npos means the name is empty or only blanks, so reject it.
    std::size_t start = newName.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return EditResult::Rejected;
    }

    // Position of the last non-blank character.
    std::size_t end = newName.find_last_not_of(whitespace);

    // Keep only the part between them: "  Player  " -> "Player".
    std::string trimmed = newName.substr(start, end - start + 1);

    // Same as the current name: nothing to change, so the scene stays clean.
    if (trimmed == entity.getName()) {
        return EditResult::NoChange;
    }

    // Real change: store it and mark the scene as having unsaved changes.
    // Duplicate names are allowed; IDs are what identify entities.
    entity.setName(trimmed);
    editorState.markDirty();
    return EditResult::Applied;
}
