#pragma once

#include <filesystem>
#include <cstdint>
#include <memory>


class AudioService {

    public:
        using SoundId = std::uint32_t;

        explicit AudioService(std::filesystem::path assetRoot);
        
        ~AudioService();

        SoundId loadWav(std::filesystem::path filePath);

        bool play(SoundId soundId);

        void stop();

        bool isAvailable() const;

    private:

        struct Impl;

        std::unique_ptr<Impl> impl_;

};


