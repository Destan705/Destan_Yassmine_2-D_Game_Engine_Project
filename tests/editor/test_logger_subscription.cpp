#include "logger.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <vector>

TEST(EditorLoggerSubscriptionTest, SubscribesOnceAndUnsubscribesWithoutAffectingTerminal) {
    std::ostringstream terminal;
    auto* original = std::cout.rdbuf(terminal.rdbuf());
    std::vector<std::string> received;
    const auto subscription = subscribeToLog(
        [&](logger_level level, const std::string& message) {
            EXPECT_EQ(level, logger_level::Info);
            received.push_back(message);
        });
    logMessage(logger_level::Info, "While subscribed");
    unsubscribeFromLog(subscription);
    logMessage(logger_level::Info, "After unsubscribed");
    std::cout.rdbuf(original);
    ASSERT_EQ(received.size(), 1u);
    EXPECT_EQ(received[0], "While subscribed");
    EXPECT_NE(terminal.str().find("[INFO] While subscribed"), std::string::npos);
    EXPECT_NE(terminal.str().find("[INFO] After unsubscribed"), std::string::npos);
}

TEST(EditorLoggerSubscriptionTest, UnsubscribeDuringNotificationIsSafe) {
    int calls = 0;
    std::uint64_t id = 0;
    id = subscribeToLog([&](logger_level, const std::string&) {
        ++calls;
        unsubscribeFromLog(id);
    });
    std::ostringstream terminal;
    auto* original = std::cout.rdbuf(terminal.rdbuf());
    logMessage(logger_level::Info, "first");
    logMessage(logger_level::Info, "second");
    std::cout.rdbuf(original);
    EXPECT_EQ(calls, 1);
}
