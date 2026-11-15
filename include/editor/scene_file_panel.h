#pragma once
#include <string>

class EditorState;

enum class SceneFileCommandType{
    None,
    Open,
    Save,
    SaveAs,
    ConfirmSave,
    ConfirmDiscard,
    ConfirmCancel,
    ProvideSavePath
};

struct SceneFileCommand{
    SceneFileCommandType type;
    std::string path;
};

class SceneFilePanel{

    private:
        char pathBuffer[1024] = {};
        std::string feedbackMessage = "";

        char savePathBuffer[1024] = {};
        bool standaloneSaveRequested = false;

        void drawConfirmationPopup(const EditorState& editorState, SceneFileCommand& command);

        void drawSavePathPopup(const EditorState& editorState, SceneFileCommand& command);

    public:
        SceneFileCommand draw(const EditorState& editorState);
        void setFeedbackMessage(const std::string& message);
        bool isStandaloneSaveRequested() const;
};