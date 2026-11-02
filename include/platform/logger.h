#pragma once
#include <string>
enum class logger_level { Info, Warning, Error };

void logMessage(logger_level level, const std::string& message);