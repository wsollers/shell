// shell_substitution_tests.cpp
// Integration tests for shell variable substitution and expansion

#include "gtest/gtest.h"
#include "shell/ast.hpp"
#include "shell/parser.hpp"
#include "shell/ast_to_command_model.hpp"
#include "shell/shell_interpreter.hpp"
#include "execution_policy.hpp"
#include <string>
#include <map>

using namespace wshell;


TEST(ShellSubstitution, VariableExpansionInCommand) {
    // Setup: let VAR=world; echo Hello $VAR
    std::string input = "let VAR = world\necho Hello $VAR\n";
    auto parse_result = parse_program(input);
    ASSERT_TRUE(parse_result.has_value());
    ProgramNode& program = *parse_result.value();
    // Setup interpreter and assign variable
    wshell::StreamOutputDestination stdout_dest(std::cout, "stdout");
    wshell::StreamOutputDestination stderr_dest(std::cerr, "stderr");
    ShellInterpreter<> interp(stdout_dest, stderr_dest );
    interp.set_variable("VAR", "world");
    // Simulate expansion (replace $VAR with value)
    // For this test, we just check that the variable is present in the AST and can be expanded
    const auto& stmt = program.statements[1];
    ASSERT_TRUE(std::holds_alternative<CommandNode>(stmt));
    const CommandNode& cmd = std::get<CommandNode>(stmt);
    ASSERT_EQ(cmd.command_name.text, "echo");
    ASSERT_EQ(cmd.arguments.size(), 2);
    EXPECT_EQ(cmd.arguments[0].text, "Hello");
    EXPECT_EQ(cmd.arguments[1].text, "$VAR");
    // Now expand using interpreter logic
    std::string expanded = interp.expand_variables(cmd.arguments[1].text);
    EXPECT_EQ(expanded, "world");
}

TEST(ShellSubstitution, VariableExpansionWithQuotes) {
    std::string input = "let X = 42\necho \"$X\"\n";
    auto parse_result = parse_program(input);
    ASSERT_TRUE(parse_result.has_value());
    ProgramNode& program = *parse_result.value();
    wshell::StreamOutputDestination stdout_dest(std::cout, "stdout");
    wshell::StreamOutputDestination stderr_dest(std::cerr, "stderr");
    ShellInterpreter<> interp(stdout_dest, stderr_dest );
    interp.set_variable("X", "42");
    const auto& stmt = program.statements[1];
    ASSERT_TRUE(std::holds_alternative<CommandNode>(stmt));
    const CommandNode& cmd = std::get<CommandNode>(stmt);
    ASSERT_EQ(cmd.arguments.size(), 1);
    EXPECT_EQ(cmd.arguments[0].text, "$X");
    EXPECT_TRUE(cmd.arguments[0].quoted);
    std::string expanded = interp.expand_variables(cmd.arguments[0].text);
    EXPECT_EQ(expanded, "42");
}


TEST(ShellSubstitution, VariableExpansion_EmptyVariable) {
    std::string input = "let EMPTY = \necho $EMPTY\n";
    auto parse_result = parse_program(input);
    ASSERT_TRUE(parse_result.has_value());
    ProgramNode& program = *parse_result.value();
    wshell::StreamOutputDestination stdout_dest(std::cout, "stdout");
    wshell::StreamOutputDestination stderr_dest(std::cerr, "stderr");
    ShellInterpreter<> interp(stdout_dest, stderr_dest );
    interp.set_variable("EMPTY", "");
    const auto& stmt = program.statements[1];
    ASSERT_TRUE(std::holds_alternative<CommandNode>(stmt));
    const CommandNode& cmd = std::get<CommandNode>(stmt);
    ASSERT_EQ(cmd.arguments.size(), 1);
    EXPECT_EQ(cmd.arguments[0].text, "$EMPTY");
    std::string expanded = interp.expand_variables(cmd.arguments[0].text);
    EXPECT_EQ(expanded, "");
}

TEST(ShellSubstitution, VariableExpansion_MixedQuotes) {
    std::string input = "let A = foo\nlet B = bar\necho \"$A $B\" $A\n";
    auto parse_result = parse_program(input);
    ASSERT_TRUE(parse_result.has_value());
    ProgramNode& program = *parse_result.value();
    wshell::StreamOutputDestination stdout_dest(std::cout, "stdout");
    wshell::StreamOutputDestination stderr_dest(std::cerr, "stderr");
    ShellInterpreter<> interp(stdout_dest, stderr_dest );
    interp.set_variable("A", "foo");
    interp.set_variable("B", "bar");
    const auto& stmt = program.statements[2];
    ASSERT_TRUE(std::holds_alternative<CommandNode>(stmt));
    const CommandNode& cmd = std::get<CommandNode>(stmt);
    ASSERT_EQ(cmd.arguments.size(), 2);
    // First arg is quoted, should expand both vars, no splitting
    EXPECT_EQ(cmd.arguments[0].text, "$A $B");
    EXPECT_TRUE(cmd.arguments[0].quoted);
    std::string expanded0 = interp.expand_variables(cmd.arguments[0].text);
    EXPECT_EQ(expanded0, "foo bar");
    // Second arg is unquoted, should expand $A
    EXPECT_EQ(cmd.arguments[1].text, "$A");
    EXPECT_FALSE(cmd.arguments[1].quoted);
    std::string expanded1 = interp.expand_variables(cmd.arguments[1].text);
    EXPECT_EQ(expanded1, "foo");
}

TEST(ShellSubstitution, VariableExpansion_NestedQuotesLiteral) {
    std::string input = "let X = 42\necho \"'Value: $X'\"\n";
    auto parse_result = parse_program(input);
    ASSERT_TRUE(parse_result.has_value());
    ProgramNode& program = *parse_result.value();
    wshell::StreamOutputDestination stdout_dest(std::cout, "stdout");
    wshell::StreamOutputDestination stderr_dest(std::cerr, "stderr");
    ShellInterpreter<> interp(stdout_dest, stderr_dest );
    interp.set_variable("X", "42");
    const auto& stmt = program.statements[1];
    ASSERT_TRUE(std::holds_alternative<CommandNode>(stmt));
    const CommandNode& cmd = std::get<CommandNode>(stmt);
    ASSERT_EQ(cmd.arguments.size(), 1);
    // The argument is quoted, but contains a literal single quote
    EXPECT_EQ(cmd.arguments[0].text, "'Value: $X'");
    EXPECT_TRUE(cmd.arguments[0].quoted);
    std::string expanded = interp.expand_variables(cmd.arguments[0].text);
    EXPECT_EQ(expanded, "'Value: 42'");
}
