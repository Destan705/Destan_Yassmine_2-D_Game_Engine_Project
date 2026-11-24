// Extra Phase 4 Log Console tests; does not test concurrent logging.
#include "console_commands.h"
#include "editor_state.h"
#include "log_buffer.h"
#include "logger.h"

#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>

TEST(ConsoleRegressionTest, CapturesAllSeveritiesWithoutSuppressingTerminalOutput) {
    LogBuffer buffer;
    std::ostringstream standardOutput;
    std::ostringstream standardError;

    // Keep the terminal streams restored before running any assertions.
    auto* previousOut = std::cout.rdbuf(standardOutput.rdbuf());
    auto* previousErr = std::cerr.rdbuf(standardError.rdbuf());
    const auto listener = subscribeToLog([&](logger_level level, const std::string& msg) {
        buffer.add(level, msg);
    });
    logMessage(logger_level::Info, "Info message");
    logMessage(logger_level::Warning, "Warning message");
    logMessage(logger_level::Error, "Error message");
    unsubscribeFromLog(listener);
    logMessage(logger_level::Info, "After unsubscribe");
    std::cout.rdbuf(previousOut);
    std::cerr.rdbuf(previousErr);

    const auto& entries = buffer.getEntries();
    ASSERT_EQ(entries.size(), 3u);
    EXPECT_EQ(entries[0].severity, logger_level::Info);
    EXPECT_EQ(entries[1].severity, logger_level::Warning);
    EXPECT_EQ(entries[2].severity, logger_level::Error);
    EXPECT_EQ(entries[0].message, "Info message");
    EXPECT_EQ(entries[1].message, "Warning message");
    EXPECT_EQ(entries[2].message, "Error message");
    EXPECT_LT(entries[0].logCounter, entries[1].logCounter);
    EXPECT_LT(entries[1].logCounter, entries[2].logCounter);
    EXPECT_NE(standardOutput.str().find("[INFO] Info message"), std::string::npos);
    EXPECT_NE(standardOutput.str().find("[WARNING] Warning message"), std::string::npos);
    EXPECT_NE(standardError.str().find("[ERROR] Error message"), std::string::npos);
    EXPECT_NE(standardOutput.str().find("After unsubscribe"), std::string::npos);
}

TEST(ConsoleRegressionTest, EachSeverityFilterMatchesOnlyItsOwnMessages) {
    LogBuffer buffer;
    buffer.add(logger_level::Info, "Shared keyword");
    buffer.add(logger_level::Warning, "Shared keyword");
    buffer.add(logger_level::Error, "Shared keyword");
    const auto& entries = buffer.getEntries();
    ASSERT_EQ(entries.size(), 3u);

    EXPECT_TRUE(consoleLogMatches(entries[0], true, false, false, "keyword"));
    EXPECT_FALSE(consoleLogMatches(entries[1], true, false, false, "keyword"));
    EXPECT_FALSE(consoleLogMatches(entries[2], true, false, false, "keyword"));

    EXPECT_FALSE(consoleLogMatches(entries[0], false, true, false, "keyword"));
    EXPECT_TRUE(consoleLogMatches(entries[1], false, true, false, "keyword"));
    EXPECT_FALSE(consoleLogMatches(entries[2], false, true, false, "keyword"));

    EXPECT_FALSE(consoleLogMatches(entries[0], false, false, true, "keyword"));
    EXPECT_FALSE(consoleLogMatches(entries[1], false, false, true, "keyword"));
    EXPECT_TRUE(consoleLogMatches(entries[2], false, false, true, "keyword"));
}

TEST(ConsoleRegressionTest, ReadingAndFilteringDoNotMutateBuffer) {
    LogBuffer buffer;
    buffer.add(logger_level::Info, "Alpha");
    buffer.add(logger_level::Warning, "Beta");
    const auto originalFirstSequence = buffer.getEntries().front().logCounter;

    for (int i = 0; i < 10; ++i) {
        for (const auto& entry : buffer.getEntries()) {
            (void)consoleLogMatches(entry, true, true, true, "Alpha");
        }
    }

    ASSERT_EQ(buffer.getEntries().size(), 2u);
    EXPECT_EQ(buffer.getEntries()[0].logCounter, originalFirstSequence);
    EXPECT_EQ(buffer.getEntries()[0].message, "Alpha");
    EXPECT_EQ(buffer.getEntries()[1].message, "Beta");
}

TEST(ConsoleRegressionTest, AllRecognizedCommandsRejectExtraArguments) {
    for (const auto& command : {"help extra", "clear extra", "pause extra", "resume extra"}) {
        SCOPED_TRACE(command);
        LogBuffer buffer;
        EditorState state;
        buffer.add(logger_level::Info, "Keep me");
        executeConsoleCommand(command, buffer, state);

        ASSERT_EQ(buffer.getEntries().size(), 2u);
        EXPECT_EQ(buffer.getEntries()[0].message, "Keep me");
        EXPECT_EQ(buffer.getEntries()[1].severity, logger_level::Error);
        EXPECT_TRUE(state.getPause());
        EXPECT_FALSE(state.isDirty());
    }
}

TEST(ConsoleRegressionTest, EmptySearchMatchesAllEnabledSeverities) {
    LogBuffer buffer;
    buffer.add(logger_level::Info, "One");
    buffer.add(logger_level::Error, "Two");
    const auto& entries = buffer.getEntries();
    ASSERT_EQ(entries.size(), 2u);

    EXPECT_TRUE(consoleLogMatches(entries[0], true, false, false, ""));
    EXPECT_TRUE(consoleLogMatches(entries[1], false, false, true, ""));
    EXPECT_FALSE(consoleLogMatches(entries[0], false, false, false, ""));
}
