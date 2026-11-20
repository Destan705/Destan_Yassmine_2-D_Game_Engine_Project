#include "console_commands.h"
#include <gtest/gtest.h>

TEST(ConsoleCommandsTest, HelpListsExactlyFourCommands) {
    LogBuffer buffer;
    EditorState state;
    executeConsoleCommand("help", buffer, state);
    ASSERT_EQ(buffer.getEntries().size(), 1u);
    EXPECT_EQ(buffer.getEntries()[0].severity, logger_level::Info);
    EXPECT_EQ(buffer.getEntries()[0].message, "Commands: help, clear, pause, resume");
}

TEST(ConsoleCommandsTest, PauseAndResumeChangeOnlySimulationState) {
    LogBuffer buffer;
    EditorState state;
    EXPECT_TRUE(state.getPause());
    executeConsoleCommand("resume", buffer, state);
    EXPECT_FALSE(state.getPause());
    EXPECT_FALSE(state.isDirty());
    executeConsoleCommand("pause", buffer, state);
    EXPECT_TRUE(state.getPause());
    EXPECT_TRUE(buffer.getEntries().empty());
}

TEST(ConsoleCommandsTest, ClearEmptiesMessages) {
    LogBuffer buffer;
    EditorState state;
    buffer.add(logger_level::Error, "Original error");
    executeConsoleCommand("clear", buffer, state);
    EXPECT_TRUE(buffer.getEntries().empty());
}

TEST(ConsoleCommandsTest, UnknownCommandsAreNotExecuted) {
    LogBuffer buffer;
    EditorState state;
    executeConsoleCommand("rm -rf /", buffer, state);
    ASSERT_EQ(buffer.getEntries().size(), 1u);
    EXPECT_EQ(buffer.getEntries()[0].severity, logger_level::Error);
    EXPECT_EQ(buffer.getEntries()[0].message, "Unknown command: rm");
    EXPECT_TRUE(state.getPause());
}

TEST(ConsoleCommandsTest, KnownCommandsRejectExtraArguments) {
    LogBuffer buffer;
    EditorState state;
    executeConsoleCommand("resume now", buffer, state);
    EXPECT_TRUE(state.getPause());
    ASSERT_EQ(buffer.getEntries().size(), 1u);
    EXPECT_EQ(buffer.getEntries()[0].message, "resume takes no arguments");
}

TEST(ConsoleCommandsTest, WhitespaceOnlyIsIgnoredAndTrailingWhitespaceAllowed) {
    LogBuffer buffer;
    EditorState state;
    executeConsoleCommand("  \t  ", buffer, state);
    EXPECT_TRUE(buffer.getEntries().empty());
    executeConsoleCommand("  resume  \t", buffer, state);
    EXPECT_FALSE(state.getPause());
    EXPECT_TRUE(buffer.getEntries().empty());
}

TEST(ConsoleCommandsTest, UnknownCaseIsRejected) {
    LogBuffer buffer;
    EditorState state;
    executeConsoleCommand("Help", buffer, state);
    ASSERT_EQ(buffer.getEntries().size(), 1u);
    EXPECT_EQ(buffer.getEntries()[0].severity, logger_level::Error);
}
