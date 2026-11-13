#include "audio_service.h"
#include "logger.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

constexpr std::size_t maxWavSize = 2u * 1024u * 1024u;
constexpr std::size_t maxCacheSize = 8u * 1024u * 1024u;
constexpr std::uint64_t maxDurationSeconds = 5;

struct CachedSound {
    std::vector<std::uint8_t> samples;
    SDL_AudioSpec spec{};
};

// Only allow relative paths inside the configured asset root.
std::optional<std::filesystem::path> normalizePath(const std::filesystem::path& filePath) {
    std::string stringPath = filePath.generic_string();
    std::replace(stringPath.begin(), stringPath.end(), '\\', '/');

    if (stringPath.empty()) {
        logMessage(logger_level::Warning, "Empty paths are not allowed");
        return std::nullopt;
    }

    if (stringPath.size() >= 2 &&
        std::isalpha(static_cast<unsigned char>(stringPath[0])) &&
        stringPath[1] == ':') {
        logMessage(logger_level::Warning, "Paths starting with Windows drive prefixes are not allowed");
        return std::nullopt;
    }

    const std::filesystem::path standardizedPath = stringPath;
    if (standardizedPath.is_absolute() || standardizedPath.has_root_path() ||
        stringPath.front() == '/') {
        logMessage(logger_level::Warning, "Absolute or rooted paths are not allowed");
        return std::nullopt;
    }

    for (const auto& component : standardizedPath) {
        if (component == "..") {
            logMessage(logger_level::Warning, ".. traversals in paths are not allowed");
            return std::nullopt;
        }
    }

    std::filesystem::path normalizedPath = standardizedPath.lexically_normal();
    if (normalizedPath.empty() || normalizedPath == ".") {
        logMessage(logger_level::Warning, "Empty paths are not allowed");
        return std::nullopt;
    }

    return normalizedPath;
}

// Check decoded size, audio format, complete frames, and duration.
bool validateWav(const SDL_AudioSpec& wavSpec, Uint32 audioLength) {
    if (audioLength == 0 || audioLength > maxWavSize) {
        logMessage(logger_level::Warning, "Audio size too big or empty");
        return false;
    }

    if (wavSpec.freq <= 0 || wavSpec.channels <= 0) {
        logMessage(logger_level::Warning, "Spec freq and channels must be positive");
        return false;
    }

    const auto bytesPerFrame = SDL_AUDIO_FRAMESIZE(wavSpec);
    if (bytesPerFrame <= 0 || audioLength % bytesPerFrame != 0) {
        logMessage(logger_level::Warning, "Audio data ends with an incomplete or invalid frame");
        return false;
    }

    const std::uint64_t maxDurationBytes =
        maxDurationSeconds * static_cast<std::uint64_t>(bytesPerFrame) *
        static_cast<std::uint64_t>(wavSpec.freq);

    if (static_cast<std::uint64_t>(audioLength) > maxDurationBytes) {
        logMessage(logger_level::Warning, "Duration can't be longer than 5 seconds");
        return false;
    }

    return true;
}

} // namespace

struct AudioService::Impl {
    std::filesystem::path assetRoot;
    std::unordered_map<std::string, SoundId> pathToId;
    std::unordered_map<SoundId, CachedSound> idToSound;
    SoundId nextSoundId = 1;
    std::size_t cachedBytes = 0;
    SDL_AudioStream* stream = nullptr;

    // Either update both maps or undo the first insertion.
    bool insertSound(const std::string& normalizedPath, SoundId soundId, CachedSound&& sound) {
        auto [it, inserted] = idToSound.emplace(soundId, std::move(sound));
        if (!inserted) {
            return false;
        }

        try {
            if (!pathToId.emplace(normalizedPath, soundId).second) {
                idToSound.erase(it);
                return false;
            }
        } catch (...) {
            idToSound.erase(it);
            throw;
        }

        return true;
    }

    ~Impl() {
        if (stream) {
            SDL_DestroyAudioStream(stream);
        }
        stream = nullptr;
    }
};


AudioService::AudioService(std::filesystem::path assetRoot) {
    impl_ = std::make_unique<Impl>();
    impl_->assetRoot = std::move(assetRoot);
    impl_->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr, nullptr, nullptr);
    if (!impl_->stream) {
        logMessage(logger_level::Warning, std::string("Stream was not initialized: ") + SDL_GetError());
        return;
    }
    if (!SDL_ResumeAudioStreamDevice(impl_->stream)) {
        logMessage(logger_level::Warning, std::string("Resuming audio stream was unsuccesful: ") + SDL_GetError());
        SDL_DestroyAudioStream(impl_->stream);
        impl_->stream = nullptr;
    }
}

AudioService::~AudioService() = default;

AudioService::SoundId AudioService::loadWav(std::filesystem::path filePath) {
    try {
        const auto normalizedPath = normalizePath(filePath);
        if (!normalizedPath) {
            return SoundId{0};
        }

        const std::string normalizedPathString = normalizedPath->generic_string();
        const auto existing = impl_->pathToId.find(normalizedPathString);
        if (existing != impl_->pathToId.end()) {
            return existing->second;
        }

        // Reserve 0 for invalid sounds, and never wrap or reuse IDs.
        if (impl_->nextSoundId == std::numeric_limits<SoundId>::max()) {
            logMessage(logger_level::Warning, "No sound IDs remain");
            return SoundId{0};
        }

        const std::filesystem::path fullPath = impl_->assetRoot / *normalizedPath;
        const std::string fullPathString = fullPath.generic_string();

        SDL_AudioSpec wavSpec{};
        Uint8* audioBuffer = nullptr;
        Uint32 audioLength = 0;

        if (!SDL_LoadWAV(fullPathString.c_str(), &wavSpec, &audioBuffer, &audioLength)) {
            logMessage(logger_level::Warning, std::string("Failed to load WAV file: ") + SDL_GetError());
            return SoundId{0};
        }

        // Free SDL's buffer on every exit path, including exceptions.
        std::unique_ptr<Uint8, decltype(&SDL_free)> audioBufferPointer(audioBuffer, &SDL_free);

        if (!validateWav(wavSpec, audioLength)) {
            return SoundId{0};
        }

        if (impl_->cachedBytes > maxCacheSize ||
            audioLength > maxCacheSize - impl_->cachedBytes) {
            logMessage(logger_level::Warning, "No space left in the cache");
            return SoundId{0};
        }

        CachedSound cachedSound;
        cachedSound.spec = wavSpec;
        cachedSound.samples.assign(audioBufferPointer.get(), audioBufferPointer.get() + audioLength);

        const SoundId soundId = impl_->nextSoundId;
        if (!impl_->insertSound(normalizedPathString, soundId, std::move(cachedSound))) {
            logMessage(logger_level::Warning, "Failed to insert WAV into the cache");
            return SoundId{0};
        }

        impl_->cachedBytes += audioLength;
        ++impl_->nextSoundId;
        return soundId;

    } catch (const std::exception& error) {
        logMessage(logger_level::Warning, std::string("Failed to cache WAV: ") + error.what());
        return SoundId{0};
    }
}

bool AudioService::isAvailable() const {
    if (!impl_->stream) {
        return false;
    }
    return true;
}

void AudioService::stop() {
    if (!isAvailable()) {
        return;
    }
    if (!SDL_ClearAudioStream(impl_->stream)) {
        logMessage(logger_level::Warning, std::string("Clearing audio stream was unsuccesful: ") + SDL_GetError());
    }
}

bool AudioService::play(SoundId soundId) {
    if (!isAvailable()) {
        return false;
    }
    auto iterator = impl_->idToSound.find(soundId);

    if (iterator == impl_->idToSound.end()) {
        return false;
    }

    const CachedSound& cachedSound = iterator->second;

    if (!SDL_ClearAudioStream(impl_->stream)) {
        logMessage(logger_level::Warning, std::string("Clearing audio stream was unsuccesful: ") + SDL_GetError());
        return false;
    }

    if (!SDL_SetAudioStreamFormat(impl_->stream, &cachedSound.spec, nullptr)) {
        logMessage(logger_level::Warning, std::string("Setting audio stream format was unsuccesful: ") + SDL_GetError());
        return false;    
    }

    if (!SDL_PutAudioStreamData(impl_->stream, cachedSound.samples.data(), static_cast<int>(cachedSound.samples.size()))) {
        logMessage(logger_level::Warning, std::string("Putting audio stream data was unsuccesful: ") + SDL_GetError());
        return false; 
    }

    return true;
}