#include "audio_service.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <system_error>
#include <vector>

namespace {

// Writes a minimal 16-bit mono PCM WAV file with `sampleCount` silent samples.
void WriteWav(const std::filesystem::path& path, std::uint32_t sampleRate,
              std::uint32_t sampleCount) {
    const std::uint16_t channels = 1;
    const std::uint16_t bitsPerSample = 16;
    const std::uint32_t byteRate = sampleRate * channels * bitsPerSample / 8;
    const std::uint16_t blockAlign = channels * bitsPerSample / 8;
    const std::uint32_t dataSize = sampleCount * blockAlign;

    std::ofstream file(path, std::ios::binary);
    auto write32 = [&](std::uint32_t value) {
        for (int i = 0; i < 4; ++i) {
            file.put(static_cast<char>((value >> (8 * i)) & 0xFF));
        }
    };
    auto write16 = [&](std::uint16_t value) {
        file.put(static_cast<char>(value & 0xFF));
        file.put(static_cast<char>((value >> 8) & 0xFF));
    };

    file.write("RIFF", 4);
    write32(36 + dataSize);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    write32(16);
    write16(1); // PCM
    write16(channels);
    write32(sampleRate);
    write32(byteRate);
    write16(blockAlign);
    write16(bitsPerSample);
    file.write("data", 4);
    write32(dataSize);
    const std::vector<char> silence(dataSize, 0);
    file.write(silence.data(), static_cast<std::streamsize>(silence.size()));
}

std::filesystem::path MakeTempFolder() {
    std::random_device random;
    for (int attempt = 0; attempt < 100; ++attempt) {
        auto candidate = std::filesystem::temp_directory_path() /
                         ("audio-service-test-" + std::to_string(random()));
        if (std::filesystem::create_directory(candidate)) {
            return candidate;
        }
    }
    return {};
}

// SDL audio running on the "dummy" driver, so tests work without speakers.
class AudioServiceTest : public ::testing::Test {
  protected:
    std::filesystem::path root;

    void SetUp() override {
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_AUDIO_DRIVER, "dummy", SDL_HINT_OVERRIDE));
        ASSERT_TRUE(SDL_Init(SDL_INIT_AUDIO)) << SDL_GetError();

        root = MakeTempFolder();
        ASSERT_FALSE(root.empty()) << "Could not create a temporary asset folder";
        std::filesystem::create_directories(root / "audio");
        WriteWav(root / "audio/click.wav", 8000, 800); // 0.1 s
        WriteWav(root / "audio/beep.wav", 8000, 1600); // 0.2 s
    }

    void TearDown() override {
        SDL_Quit();
        SDL_ResetHint(SDL_HINT_AUDIO_DRIVER);
        if (!root.empty()) {
            std::error_code error;
            std::filesystem::remove_all(root, error);
        }
    }
};

// ---------- Device ----------

TEST_F(AudioServiceTest, IsAvailableWithWorkingDevice) {
    AudioService audio(root);
    EXPECT_TRUE(audio.isAvailable());
}

TEST_F(AudioServiceTest, CanBeCreatedAndDestroyedRepeatedly) {
    for (int i = 0; i < 3; ++i) {
        AudioService audio(root);
        EXPECT_TRUE(audio.isAvailable());
    }
}

// ---------- Loading and caching ----------

TEST_F(AudioServiceTest, LoadsValidWav) {
    AudioService audio(root);
    EXPECT_NE(audio.loadWav("audio/click.wav"), 0u);
}

TEST_F(AudioServiceTest, SamePathReturnsSameId) {
    AudioService audio(root);
    const auto first = audio.loadWav("audio/click.wav");
    EXPECT_EQ(audio.loadWav("audio/click.wav"), first);
}

TEST_F(AudioServiceTest, EquivalentPathSpellingsShareOneId) {
    AudioService audio(root);
    const auto id = audio.loadWav("audio/click.wav");
    EXPECT_EQ(audio.loadWav("./audio/click.wav"), id);
    EXPECT_EQ(audio.loadWav("audio\\click.wav"), id);
}

TEST_F(AudioServiceTest, DifferentFilesGetDifferentIds) {
    AudioService audio(root);
    EXPECT_NE(audio.loadWav("audio/click.wav"), audio.loadWav("audio/beep.wav"));
}

// ---------- Failures (return 0, never throw) ----------

TEST_F(AudioServiceTest, MissingFileReturnsZero) {
    AudioService audio(root);
    EXPECT_EQ(audio.loadWav("audio/missing.wav"), 0u);
}

TEST_F(AudioServiceTest, FailedLoadIsNotCachedAndCanBeRetried) {
    AudioService audio(root);
    EXPECT_EQ(audio.loadWav("audio/later.wav"), 0u);

    WriteWav(root / "audio/later.wav", 8000, 400);
    EXPECT_NE(audio.loadWav("audio/later.wav"), 0u);
}

TEST_F(AudioServiceTest, NonWavFileReturnsZero) {
    std::ofstream(root / "audio/not-a-wav.wav") << "this is just text";
    AudioService audio(root);
    EXPECT_EQ(audio.loadWav("audio/not-a-wav.wav"), 0u);
}

TEST_F(AudioServiceTest, SoundLongerThanFiveSecondsIsRejected) {
    WriteWav(root / "audio/long.wav", 8000, 8000 * 6); // 6 seconds
    AudioService audio(root);
    EXPECT_EQ(audio.loadWav("audio/long.wav"), 0u);
}

TEST_F(AudioServiceTest, EmptySoundIsRejected) {
    WriteWav(root / "audio/empty.wav", 8000, 0);
    AudioService audio(root);
    EXPECT_EQ(audio.loadWav("audio/empty.wav"), 0u);
}

TEST_F(AudioServiceTest, InvalidPathsReturnZero) {
    AudioService audio(root);
    EXPECT_EQ(audio.loadWav(""), 0u);
    EXPECT_EQ(audio.loadWav("."), 0u);
    EXPECT_EQ(audio.loadWav("/audio/click.wav"), 0u);
    EXPECT_EQ(audio.loadWav((root / "audio/click.wav").string()), 0u);
    EXPECT_EQ(audio.loadWav("C:/audio/click.wav"), 0u);
    EXPECT_EQ(audio.loadWav("../audio/click.wav"), 0u);
    EXPECT_EQ(audio.loadWav("audio/../audio/click.wav"), 0u);
}

TEST_F(AudioServiceTest, FailedLoadsDoNotUseUpIds) {
    AudioService audio(root);
    const auto first = audio.loadWav("audio/click.wav");
    audio.loadWav("audio/missing.wav");
    audio.loadWav("../nope.wav");
    EXPECT_EQ(audio.loadWav("audio/beep.wav"), first + 1);
}

// ---------- Playback ----------

TEST_F(AudioServiceTest, PlaysLoadedSound) {
    AudioService audio(root);
    const auto id = audio.loadWav("audio/click.wav");
    ASSERT_NE(id, 0u);
    EXPECT_TRUE(audio.play(id));
}

TEST_F(AudioServiceTest, PlayingAgainRestartsWithoutError) {
    AudioService audio(root);
    const auto click = audio.loadWav("audio/click.wav");
    const auto beep = audio.loadWav("audio/beep.wav");
    EXPECT_TRUE(audio.play(click));
    EXPECT_TRUE(audio.play(click));
    EXPECT_TRUE(audio.play(beep)); // replaces the current sound
}

TEST_F(AudioServiceTest, PlayingUnknownIdFails) {
    AudioService audio(root);
    EXPECT_FALSE(audio.play(0));
    EXPECT_FALSE(audio.play(12345));
}

TEST_F(AudioServiceTest, StopIsSafeBeforeAndAfterPlaying) {
    AudioService audio(root);
    EXPECT_NO_THROW(audio.stop());
    audio.play(audio.loadWav("audio/click.wav"));
    EXPECT_NO_THROW(audio.stop());
    EXPECT_NO_THROW(audio.stop());
}

// ---------- No audio device ----------

TEST(AudioServiceWithoutDeviceTest, DegradesGracefully) {
    // SDL audio is deliberately not initialized, like a machine without a sound device.
    const auto root = MakeTempFolder();
    ASSERT_FALSE(root.empty());
    std::filesystem::create_directories(root / "audio");
    WriteWav(root / "audio/click.wav", 8000, 800);

    {
        AudioService audio(root);
        EXPECT_FALSE(audio.isAvailable());

        const auto id = audio.loadWav("audio/click.wav"); // loading may still work
        EXPECT_FALSE(audio.play(id));
        EXPECT_NO_THROW(audio.stop());
    }

    std::error_code error;
    std::filesystem::remove_all(root, error);
}

} // namespace
