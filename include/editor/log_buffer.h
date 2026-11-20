#pragma once 
#include "logger.h"
#include <cstdint>
#include <string>
#include <deque>


struct LogEntry{
    logger_level severity;
    
    std::uint64_t logCounter;

    std::string message;
};

class LogBuffer{
private:
std::deque<LogEntry> logEntries;
std::uint64_t seqNum=1;
static constexpr std::size_t maxEntries = 1000;
static constexpr std::size_t maxMessageBytes = 4096;

public:
void add(logger_level severity, const std::string&  message);
const std::deque<LogEntry>& getEntries() const;
void clear();
};