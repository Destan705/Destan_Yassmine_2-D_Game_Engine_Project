#pragma once
#include "log_buffer.h"
#include "editor_state.h"

class ConsolePanel{
    public:

    void draw(LogBuffer& logBuffer, EditorState& editorState);

    private:

    bool showInfo = true;
    bool showWarning = true;
    bool showError = true;
    char searchBuffer[128] = "";
    char commandBuffer[128] = "";

};