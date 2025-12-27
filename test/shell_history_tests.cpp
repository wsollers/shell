// shell_history_tests.cpp
// Unit tests for shell history functionality

#include "gtest/gtest.h"
#include "shell/history.hpp"
#include <string>

using wshell::History;

TEST(HistoryTests, PushAndSize) {
    History history;
    EXPECT_TRUE(history.empty());
    history.push("ls -l");
    history.push("echo hello");
    EXPECT_EQ(history.size(), 2);
    const auto& items = history.items();
    ASSERT_EQ(items[0], "ls -l");
    ASSERT_EQ(items[1], "echo hello");
}

TEST(HistoryTests, SetMaxTrimsHistory) {
    History history;
    history.push("one");
    history.push("two");
    history.push("three");
    history.set_max(2);
    EXPECT_EQ(history.size(), 2);
    const auto& items = history.items();
    EXPECT_EQ(items[0], "two");
    EXPECT_EQ(items[1], "three");
}

TEST(HistoryTests, EmptyHistory) {
    History history;
    EXPECT_TRUE(history.empty());
    history.push("foo");
    EXPECT_FALSE(history.empty());
}
