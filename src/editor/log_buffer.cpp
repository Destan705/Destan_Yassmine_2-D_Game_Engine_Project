#include "log_buffer.h"

void LogBuffer::add(logger_level severity, const std::string&  message){
    std::string text = message;
    const std::string mark = " [truncated]";
    if(maxMessageBytes< message.size()){
        text.resize(maxMessageBytes-mark.size());
        text+=mark;
    }



    if(logEntries.size()>= maxEntries){
        logEntries.pop_front();
        logEntries.push_back({ severity, seqNum, text });
        seqNum++;
    }else{
        logEntries.push_back({ severity, seqNum, text });
        seqNum++;
    }

}

const std::deque<LogEntry>& LogBuffer::getEntries() const{
    return logEntries;
}

void LogBuffer::clear(){
    logEntries.clear();
}