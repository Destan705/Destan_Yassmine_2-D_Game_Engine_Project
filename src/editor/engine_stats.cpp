#include "engine_stats.h"
#include <cmath>


void EngineStats::recordFrame(double deltaTime) {

    if (!std::isfinite(deltaTime) || deltaTime <= 0){
        return;
    }

    frameDurations.emplace_back(deltaTime);
    totalDuration += deltaTime;

    while (frameDurations.size() > 1 && totalDuration - frameDurations.front() >= windowDuration){
        totalDuration -= frameDurations.front();
        frameDurations.pop_front();
    }

    while (frameDurations.size() > maxSamples){
        totalDuration -= frameDurations.front();
        frameDurations.pop_front();
    }
}

std::size_t EngineStats::getSampleCount() const{
    return frameDurations.size();
}

std::optional<double> EngineStats::getFPS() const {
    if (frameDurations.empty() || totalDuration <= 0 || !std::isfinite(totalDuration)){
        return std::nullopt;
    }
    return (getSampleCount()/totalDuration);
}

std::optional<double> EngineStats::getAverageFrameTimeMs() const {
    if (frameDurations.empty() || totalDuration <= 0 || !std::isfinite(totalDuration)){
        return std::nullopt;
    }
    return ((totalDuration*1000)/getSampleCount());
}

