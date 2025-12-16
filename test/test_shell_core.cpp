// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../src/lib/shell_core.h"
#include <gtest/gtest.h>

namespace wshell::test {

class ShellCoreTest : public ::testing::Test {
protected:
    ShellCore shell;
};

TEST_F(ShellCoreTest, VersionIsValid) {
    auto ver = ShellCore::version();
    EXPECT_FALSE(ver.empty());
    EXPECT_EQ(ver, "0.2.1");
}

TEST_F(ShellCoreTest, ExecuteEmptyCommandFails) {
    auto result = shell.execute("");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ShellError::InvalidCommand);
}

TEST_F(ShellCoreTest, ExecuteValidCommand) {
    auto result = shell.execute("echo hello");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0);
}

TEST_F(ShellCoreTest, ValidateEmptyCommand) {
    EXPECT_FALSE(ShellCore::validateCommand(""));
}

TEST_F(ShellCoreTest, ValidateWhitespaceOnlyCommand) {
    EXPECT_FALSE(ShellCore::validateCommand("   "));
    EXPECT_FALSE(ShellCore::validateCommand("\t\n"));
}

TEST_F(ShellCoreTest, ValidateNormalCommand) {
    EXPECT_TRUE(ShellCore::validateCommand("echo test"));
    EXPECT_TRUE(ShellCore::validateCommand("ls -la"));
}

TEST_F(ShellCoreTest, ValidateCommandWithNullByte) {
    std::string cmd = "echo";
    cmd += '\0';
    cmd += "test";
    EXPECT_FALSE(ShellCore::validateCommand(cmd));
}

TEST_F(ShellCoreTest, ExecuteInvalidCommandFails) {
    auto result = shell.execute("   ");
    EXPECT_FALSE(result.has_value());
}

} // namespace wshell::test
