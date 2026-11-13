#pragma once

class EditorState;
class Scene;

class HierarchyPanel {
    public:
        void draw(EditorState& editorState, Scene* activeScene);
};