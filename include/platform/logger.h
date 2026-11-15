#pragma once
#include <string>
#include <cstdint>
#include <functional>
#include <string>

enum class logger_level { Info, Warning, Error };

void logMessage(logger_level level, const std::string& message);

// A function that receives every logged message (e.g. the editor's log console).
using LogListener = std::function<void(logger_level, const std::string&)>;

// Registers a listener that is called for every logMessage(), after it prints.
// Returns an ID to pass to unsubscribeFromLog().
std::uint64_t subscribeToLog(LogListener listener);

// Removes the listener with this ID. Unknown IDs are ignored.
void unsubscribeFromLog(std::uint64_t id);