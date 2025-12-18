// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause
#include <gtest/gtest.h>

#include <iostream>
#include <span>
#include <string>

#include "shell/config.hpp"
#include "shell/parser.hpp"
#include "shell/shell_interpreter.hpp"

#include <cstdio>

#include <shell/output_destination.hpp>

namespace wshell::test {

TEST(CommandParserTest, TestMIdentifierCannotStartWithDigit) {

    auto parse_result = wshell::parse_line("let 123 = 6\n");

    ASSERT_FALSE(parse_result.has_value());

}

TEST(CommandParserTest, TestMissingVariableIdentifier) {

    auto parse_result = wshell::parse_line("let = 6\n");

    ASSERT_FALSE(parse_result.has_value());

}

TEST(CommandParserTest, TestMissingVariableValue) {

    auto parse_result = wshell::parse_line("let x = \n");

    ASSERT_FALSE(parse_result.has_value());

}

TEST(CommandParserTest, TestVariableValueInternalSpaces) {

    auto parse_result = wshell::parse_line("let x = hello world\n");

    ASSERT_TRUE(parse_result.has_value());

}

TEST(CommandParserTest, TokenizeSpace) {

    auto parse_result = wshell::parse_line(" ");

    //ASSERT_FALSE(parse_result.has_value());

}

TEST(CommandParserTest, TokenizeEmptyString) {

    auto parse_result = wshell::parse_line("");

    ASSERT_TRUE(parse_result.has_value());

 }

TEST(CommandParserTest, TokenizeEmptyLine) {

    auto parse_result = wshell::parse_line("\n");

    ASSERT_TRUE(parse_result.has_value());

}

TEST(CommandParserTest, TokenizeMultipleEmptyLines) {

    auto parse_result = wshell::parse_line("\n\n\n\n");

    ASSERT_TRUE(parse_result.has_value());

}

/*
TEST(CommandParserTest, TokenizeSingleWord) {

    auto parse_result = wshell::parse_line("hello");

    ASSERT_FALSE(parse_result.has_value());
}
*/


/*
TEST(CommandParserTest, TokenizeMultipleWords) {
    auto result = CommandParser::tokenize("echo hello world");
    ASSERT_TRUE(result.has_value());
    auto& tokens = *result;
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "hello");
    EXPECT_EQ(tokens[2], "world");
}
*/

/*
TEST(CommandParserTest, TokenizeWithQuotes) {
    auto result = CommandParser::tokenize("echo \"hello world\"");
    ASSERT_TRUE(result.has_value());
    auto& tokens = *result;
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "hello world");
}
*/

/*
TEST(CommandParserTest, TokenizeWithExtraSpaces) {
    auto result = CommandParser::tokenize("  echo   test  ");
    ASSERT_TRUE(result.has_value());
    auto& tokens = *result;
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "test");
}
*/

/*
TEST(CommandParserTest, TokenizeUnclosedQuote) {
    auto result = CommandParser::tokenize("echo \"hello");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Unclosed quote in input");
}
*/

} // namespace wshell::test
