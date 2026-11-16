#pragma once
#include <string>
#include <cstdint>

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
    SceneFileCommandType type = SceneFileCommandType::None;
    std::string path;
};

class SceneFilePanel{

    private:
        char pathBuffer[1024] = {};
        std::string feedbackMessage = "";

        char savePathBuffer[1024] = {};
        bool standaloneSaveRequested = false;

        std::uint64_t lastSceneGeneration = 0;

        void drawConfirmationPopup(const EditorState& editorState, SceneFileCommand& command);

        void drawSavePathPopup(const EditorState& editorState, SceneFileCommand& command);


    public:
        SceneFileCommand draw(const EditorState& editorState);
        void setFeedbackMessage(const std::string& message);
        bool isStandaloneSaveRequested() const;
};