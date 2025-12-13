// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../src/lib/command_parser.h"
#include <gtest/gtest.h>

namespace wshell::test {

TEST(CommandParserTest, TokenizeEmptyString) {
    auto tokens = CommandParser::tokenize("");
    EXPECT_TRUE(tokens.empty());
}

TEST(CommandParserTest, TokenizeSingleWord) {
    auto tokens = CommandParser::tokenize("hello");
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0], "hello");
}

TEST(CommandParserTest, TokenizeMultipleWords) {
    auto tokens = CommandParser::tokenize("echo hello world");
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "hello");
    EXPECT_EQ(tokens[2], "world");
}

TEST(CommandParserTest, TokenizeWithQuotes) {
    auto tokens = CommandParser::tokenize("echo \"hello world\"");
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "hello world");
}

TEST(CommandParserTest, TokenizeWithExtraSpaces) {
    auto tokens = CommandParser::tokenize("  echo   test  ");
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "test");
}

TEST(CommandParserTest, TrimEmptyString) {
    auto result = CommandParser::trim("");
    EXPECT_TRUE(result.empty());
}

TEST(CommandParserTest, TrimNoWhitespace) {
    auto result = CommandParser::trim("hello");
    EXPECT_EQ(result, "hello");
}

TEST(CommandParserTest, TrimLeadingWhitespace) {
    auto result = CommandParser::trim("  hello");
    EXPECT_EQ(result, "hello");
}

TEST(CommandParserTest, TrimTrailingWhitespace) {
    auto result = CommandParser::trim("hello  ");
    EXPECT_EQ(result, "hello");
}

TEST(CommandParserTest, TrimBothSides) {
    auto result = CommandParser::trim("  hello  ");
    EXPECT_EQ(result, "hello");
}

TEST(CommandParserTest, TrimWhitespaceOnly) {
    auto result = CommandParser::trim("   ");
    EXPECT_TRUE(result.empty());
}

} // namespace wshell::test
