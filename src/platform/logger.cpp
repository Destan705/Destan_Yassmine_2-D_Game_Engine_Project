#include "logger.h"
#include <iostream>
#include <string>

std::string loggerLevelToString(logger_level level) {
    switch (level) {
    case logger_level::Info:
        return "INFO";
    case logger_level::Warning:
        return "WARNING";
    case logger_level::Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

void logMessage(logger_level level, const std::string& message) {
    std::string levelStr = loggerLevelToString(level);
    if (level == logger_level::Error) {
        std::cerr << "[" << levelStr << "] " << message << std::endl;
    } else {
        std::cout << "[" << levelStr << "] " << message << std::endl;
    }
}