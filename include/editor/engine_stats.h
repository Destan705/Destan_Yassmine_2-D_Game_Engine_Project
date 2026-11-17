#pragma once
#include <deque>
#include <optional>
#include <cstddef>


struct StatsSnapshot {
    std::optional<double> fps = std::nullopt;
    std::optional<double> averageFrameTimeMs = std::nullopt;
    std::size_t entityCount = 0;
    std::size_t validColliderCount = 0;
    std::size_t overlapPairCount = 0;
    std::size_t cachedTextureCount = 0;
    std::size_t spriteDrawCount = 0;
};

class EngineStats {
    private:
        std::deque<double> frameDurations;

        double totalDuration = 0.0;

        static constexpr std::size_t maxSamples = 4096;

        static constexpr double windowDuration = 1.0;

    public:
    
    void recordFrame(double deltaTime);

    std::optional<double> getFPS() const;

    std::optional<double> getAverageFrameTimeMs() const;

    std::size_t getSampleCount() const;
};