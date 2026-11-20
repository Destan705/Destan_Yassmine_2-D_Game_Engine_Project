#include "console_commands.h"

#include <string>

bool consoleLogMatches(const LogEntry& entry, bool showInfo, bool showWarning,
                       bool showError, std::string_view search) {
    if (entry.severity == logger_level::Info && !showInfo) return false;
    if (entry.severity == logger_level::Warning && !showWarning) return false;
    if (entry.severity == logger_level::Error && !showError) return false;
    return entry.message.find(search) != std::string::npos;
}

void executeConsoleCommand(std::string_view command, LogBuffer& logBuffer,
                           EditorState& editorState) {
    const auto first = command.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return;
    const auto last = command.find_last_not_of(" \t\r\n");
    command = command.substr(first, last - first + 1);

    const auto space = command.find_first_of(" \t\r\n");
    const std::string name(command.substr(0, space));
    const bool hasArguments = space != std::string_view::npos &&
        command.find_first_not_of(" \t\r\n", space) != std::string_view::npos;

    if (name != "help" && name != "clear" && name != "pause" && name != "resume") {
        logBuffer.add(logger_level::Error, "Unknown command: " + name);
    } else if (hasArguments) {
        logBuffer.add(logger_level::Error, name + " takes no arguments");
    } else if (name == "help") {
        logBuffer.add(logger_level::Info, "Commands: help, clear, pause, resume");
    } else if (name == "clear") {
        logBuffer.clear();
    } else if (name == "pause") {
        editorState.pause();
    } else if (name == "resume") {
        editorState.resume();
    }
}
