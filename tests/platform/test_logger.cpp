#include "logger.h"

#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <streambuf>

namespace {

// Redirect the logger's output so each test can inspect it.
// Restore both streams after every test so other tests are unaffected.
class LoggerTest : public ::testing::Test {
  protected:
    std::ostringstream output;
    std::ostringstream errors;
    std::streambuf* originalOutput = nullptr;
    std::streambuf* originalErrors = nullptr;

    void SetUp() override {
        originalOutput = std::cout.rdbuf(output.rdbuf());
        originalErrors = std::cerr.rdbuf(errors.rdbuf());
    }

    void TearDown() override {
        std::cout.rdbuf(originalOutput);
        std::cerr.rdbuf(originalErrors);
    }
};

TEST_F(LoggerTest, InfoUsesCorrectLabelAndStandardOutput) {
    logMessage(logger_level::Info, "Engine started");

    EXPECT_EQ(output.str(), "[INFO] Engine started\n");
    EXPECT_TRUE(errors.str().empty());
}

TEST_F(LoggerTest, WarningUsesCorrectLabelAndStandardOutput) {
    logMessage(logger_level::Warning, "Using default settings");

    EXPECT_EQ(output.str(), "[WARNING] Using default settings\n");
    EXPECT_TRUE(errors.str().empty());
}

TEST_F(LoggerTest, ErrorUsesCorrectLabelAndStandardError) {
    logMessage(logger_level::Error, "Renderer initialization failed");

    EXPECT_EQ(errors.str(), "[ERROR] Renderer initialization failed\n");
    EXPECT_TRUE(output.str().empty());
}

TEST_F(LoggerTest, EmptyMessageStillIncludesLabelAndNewline) {
    logMessage(logger_level::Info, "");

    EXPECT_EQ(output.str(), "[INFO] \n");
    EXPECT_TRUE(errors.str().empty());
}

TEST_F(LoggerTest, ConsecutiveMessagesRemainSeparateAndUseCorrectStreams) {
    logMessage(logger_level::Info, "Starting");
    logMessage(logger_level::Error, "Texture missing");
    logMessage(logger_level::Warning, "Using fallback texture");
    logMessage(logger_level::Info, "Shutting down");

    EXPECT_EQ(output.str(), "[INFO] Starting\n"
                            "[WARNING] Using fallback texture\n"
                            "[INFO] Shutting down\n");
    EXPECT_EQ(errors.str(), "[ERROR] Texture missing\n");
}

TEST_F(LoggerTest, MessagePunctuationAndWhitespaceArePreserved) {
    logMessage(logger_level::Info, "  Loading [player]: 100%\tready  ");

    EXPECT_EQ(output.str(), "[INFO]   Loading [player]: 100%\tready  \n");
    EXPECT_TRUE(errors.str().empty());
}

} // namespace
