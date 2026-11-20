#pragma once

#include "log_buffer.h"
#include "editor_state.h"
#include <string_view>

// Pure filter used by the Console UI and tested separately.
bool consoleLogMatches(const LogEntry& entry, bool showInfo, bool showWarning,
                       bool showError, std::string_view search);

// Exactly four supported commands: help, clear, pause, resume.
// Empty input has no effect. Errors are added to the viewer, never executed.
void executeConsoleCommand(std::string_view command, LogBuffer& logBuffer,
                           EditorState& editorState);
