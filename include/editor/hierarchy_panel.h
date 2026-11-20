#pragma once
#include <optional>
class EditorState;
class Scene;

class HierarchyPanel {
    public:
        void draw(EditorState& editorState, Scene* activeScene);
    private:
    char renameBuffer[128] = "";
    std::optional<std::uint64_t> renameTargetId;   // which entity the buffer was filled for
};