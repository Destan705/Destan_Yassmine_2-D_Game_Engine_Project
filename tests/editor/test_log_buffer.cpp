#include "log_buffer.h"
#include "console_commands.h"
#include <gtest/gtest.h>
#include <string>

TEST(LogBufferTest, StartsEmptyAndPreservesSeverityAndSequence) {
    LogBuffer buffer;
    EXPECT_TRUE(buffer.getEntries().empty());
    buffer.add(logger_level::Warning, "Warning one");
    buffer.add(logger_level::Error, "Error two");
    const auto& entries = buffer.getEntries();
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].severity, logger_level::Warning);
    EXPECT_EQ(entries[0].message, "Warning one");
    EXPECT_EQ(entries[1].severity, logger_level::Error);
    EXPECT_LT(entries[0].logCounter, entries[1].logCounter);
}

TEST(LogBufferTest, EvictsOldestAt1000AndKeepsOrdering) {
    LogBuffer buffer;
    for (int i = 0; i < 1010; ++i) {
        buffer.add(logger_level::Info, std::to_string(i));
    }
    const auto& entries = buffer.getEntries();
    ASSERT_EQ(entries.size(), 1000u);
    EXPECT_EQ(entries.front().message, "10");
    EXPECT_EQ(entries.back().message, "1009");
    EXPECT_LT(entries.front().logCounter, entries.back().logCounter);
}

TEST(LogBufferTest, TruncatesMessagesAt4096BytesWithVisibleMarker) {
    LogBuffer buffer;
    buffer.add(logger_level::Error, std::string(5000, 'A'));
    ASSERT_EQ(buffer.getEntries().size(), 1u);
    const auto& text = buffer.getEntries().front().message;
    EXPECT_EQ(text.size(), 4096u);
    EXPECT_EQ(text.substr(text.size() - 12), " [truncated]");
}

TEST(LogBufferTest, Exactly4096BytesAreUnchanged) {
    LogBuffer buffer;
    const std::string original(4096, 'B');
    buffer.add(logger_level::Info, original);
    EXPECT_EQ(buffer.getEntries().front().message, original);
}

TEST(LogBufferTest, ClearEmptiesViewerButDoesNotResetMonotonicSequence) {
    LogBuffer buffer;
    buffer.add(logger_level::Info, "Old");
    const auto first = buffer.getEntries().back().logCounter;
    buffer.clear();
    EXPECT_TRUE(buffer.getEntries().empty());
    buffer.add(logger_level::Info, "New");
    EXPECT_GT(buffer.getEntries().back().logCounter, first);
}

TEST(LogBufferTest, SeverityFiltersAndCaseSensitiveTextSearch) {
    LogBuffer buffer;
    buffer.add(logger_level::Info, "Loaded texture");
    buffer.add(logger_level::Warning, "Texture missing");
    buffer.add(logger_level::Error, "Unable to load");
    auto entries = buffer.getEntries();
    EXPECT_TRUE(consoleLogMatches(entries[0], true, false, false, "texture"));
    EXPECT_FALSE(consoleLogMatches(entries[1], true, false, false, "Texture"));
    EXPECT_FALSE(consoleLogMatches(entries[2], true, false, false, "load"));
    EXPECT_TRUE(consoleLogMatches(entries[2], false, false, true, "load"));
    EXPECT_FALSE(consoleLogMatches(entries[0], true, true, true, "not-found"));
}
