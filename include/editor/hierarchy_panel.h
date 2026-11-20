#pragma once

#include <optional>
#include <cstdint>

class EditorState;
class Scene;

class HierarchyPanel {
    public:
        void draw(EditorState& editorState, Scene* activeScene);
    private:
    std::uint64_t lastSceneGeneration = 0;
    char renameBuffer[128] = "";
    std::optional<std::uint64_t> renameTargetId;   // which entity the buffer was filled for
};