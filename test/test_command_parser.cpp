// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../src/lib/command_parser.h"
#include <gtest/gtest.h>

namespace wshell::test {

TEST(CommandParserTest, TokenizeEmptyString) {
    auto result = CommandParser::tokenize("");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST(CommandParserTest, TokenizeSingleWord) {
    auto result = CommandParser::tokenize("hello");
    ASSERT_TRUE(result.has_value());
    auto& tokens = *result;
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0], "hello");
}

TEST(CommandParserTest, TokenizeMultipleWords) {
    auto result = CommandParser::tokenize("echo hello world");
    ASSERT_TRUE(result.has_value());
    auto& tokens = *result;
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "hello");
    EXPECT_EQ(tokens[2], "world");
}

TEST(CommandParserTest, TokenizeWithQuotes) {
    auto result = CommandParser::tokenize("echo \"hello world\"");
    ASSERT_TRUE(result.has_value());
    auto& tokens = *result;
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "hello world");
}

TEST(CommandParserTest, TokenizeWithExtraSpaces) {
    auto result = CommandParser::tokenize("  echo   test  ");
    ASSERT_TRUE(result.has_value());
    auto& tokens = *result;
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "test");
}

TEST(CommandParserTest, TokenizeUnclosedQuote) {
    auto result = CommandParser::tokenize("echo \"hello");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Unclosed quote in input");
}

} // namespace wshell::test
