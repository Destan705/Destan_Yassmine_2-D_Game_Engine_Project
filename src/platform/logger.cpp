#include "logger.h"
#include <iostream>
#include <string>
#include <vector>

namespace{
    std::vector<std::pair<std::uint64_t, LogListener>> listeners;
    std::uint64_t idCounter= 1;

}

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

    auto current = listeners;
    for(auto& listener:current){
       listener.second(level, message);
    }
}

std::uint64_t subscribeToLog(LogListener listener){
    
    listeners.push_back({idCounter, listener});
    std::uint64_t oldId = idCounter;
    idCounter++;
    return oldId;
}

void unsubscribeFromLog(std::uint64_t id){
    for(auto it = listeners.begin(); it != listeners.end(); ++it){
        if(it->first==id){
            listeners.erase(it);
            break;
        }
    }
}
