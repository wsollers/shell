#include <gtest/gtest.h>

#include "shell/parser.hpp"
#include "shell/ast.hpp"
#include "shell/ast_printer.hpp"

using namespace wshell;

// -----------------------------------------------------------------------------
// Helpers (NO macros)
// -----------------------------------------------------------------------------

static void assert_parse_ok_and_ast_equals(
    std::string_view input,
    const std::string& expected_ast)
{
    auto result = parse_line(input);
    if (!result.has_value()) {
        const auto& err = result.error();
        FAIL() << "Parse failed for input:\n"
               << input << "\n"
               << "Error: " << err.message << "\n"
               << "At line " << err.line << ", column " << err.column;
    }

    const ProgramNode& program = *result.value();
    std::string actual = to_string(program);

    if (actual != expected_ast) {
        FAIL() << "AST mismatch\n"
               << "Input:\n" << input << "\n"
               << "Expected:\n" << expected_ast
               << "Actual:\n" << actual;
    }
}

static void assert_parse_program_ok_and_ast_equals(
    std::string_view input,
    const std::string& expected_ast)
{
    auto result = parse_program(input);
    if (!result.has_value()) {
        const auto& err = result.error();
        FAIL() << "Parse failed for input:\n"
               << input << "\n"
               << "Error: " << err.message << "\n"
               << "At line " << err.line << ", column " << err.column;
    }

    const ProgramNode& program = *result.value();
    std::string actual = to_string(program);

    if (actual != expected_ast) {
        FAIL() << "AST mismatch\n"
               << "Input:\n" << input << "\n"
               << "Expected:\n" << expected_ast
               << "Actual:\n" << actual;
    }
}

static void assert_parse_fails(std::string_view input)
{
    auto result = parse_line(input);
    if (result.has_value()) {
        FAIL() << "Expected parse failure but succeeded.\n"
               << "Input:\n" << input << "\n"
               << "AST:\n" << to_string(*result.value());
    }
}

// -----------------------------------------------------------------------------
// Basic command tests
// -----------------------------------------------------------------------------

TEST(ParserTests, SimpleCommand_NoArgs) {
    assert_parse_ok_and_ast_equals(
        "ls\n",
        "Command: ls\n"
    );
}

TEST(ParserTests, SimpleCommand_WithArgs) {
    assert_parse_ok_and_ast_equals(
        "echo hello world\n",
        "Command: echo\n"
        "  Args: hello world\n"
    );
}

TEST(ParserTests, Command_WithFlagsAndPaths) {
    assert_parse_ok_and_ast_equals(
        "grep -R foo ./src\n",
        "Command: grep\n"
        "  Args: -R foo ./src\n"
    );
}

TEST(ParserTests, Command_WhitespaceVariations) {
    assert_parse_ok_and_ast_equals(
        "   echo    hi   there   \n",
        "Command: echo\n"
        "  Args: hi there\n"
    );
}

// -----------------------------------------------------------------------------
// Assignment tests
// -----------------------------------------------------------------------------

TEST(ParserTests, Assignment_Simple) {
    assert_parse_ok_and_ast_equals(
        "let x = 42\n",
        "Assignment: x = 42\n"
    );
}

TEST(ParserTests, Assignment_Expression) {
    assert_parse_ok_and_ast_equals(
        "let y = 1 + 2 * 3\n",
        "Assignment: y = 1 + 2 * 3\n"
    );
}

TEST(ParserTests, Assignment_Whitespace) {
    assert_parse_ok_and_ast_equals(
        "let   var    =    value here\n",
        "Assignment: var = value here\n"
    );
}

// -----------------------------------------------------------------------------
// Comment tests
// -----------------------------------------------------------------------------

TEST(ParserTests, Comment_Only) {
    assert_parse_ok_and_ast_equals(
        "# hello world\n",
        "Comment: hello world\n"
    );
}

TEST(ParserTests, Comment_WithLeadingWhitespace) {
    assert_parse_ok_and_ast_equals(
        "   # spaced\n",
        "Comment: spaced\n"
    );
}

// -----------------------------------------------------------------------------
// Redirection tests
// -----------------------------------------------------------------------------

TEST(ParserTests, Redirect_Output) {
    assert_parse_program_ok_and_ast_equals(
        "ls > out.txt\n",
        "Command: ls\n"
        "  Redirections:\n"
        "    > out.txt\n"
    );
}

TEST(ParserTests, Redirect_Output2) {
    assert_parse_program_ok_and_ast_equals(
        "ls > out.txt",
        "Command: ls\n"
        "  Redirections:\n"
        "    > out.txt\n"
    );
}
TEST(ParserTests, Redirect_Append) {
    assert_parse_ok_and_ast_equals(
        "echo log >> app.log\n",
        "Command: echo\n"
        "  Args: log\n"
        "  Redirections:\n"
        "    >> app.log\n"
    );
}

TEST(ParserTests, Redirect_Input) {
    assert_parse_ok_and_ast_equals(
        "cat < input.txt\n",
        "Command: cat\n"
        "  Redirections:\n"
        "    < input.txt\n"
    );
}

TEST(ParserTests, Redirect_Multiple) {
    assert_parse_ok_and_ast_equals(
        "cmd < in > out\n",
        "Command: cmd\n"
        "  Redirections:\n"
        "    < in\n"
        "    > out\n"
    );
}

// -----------------------------------------------------------------------------
// Background tests
// -----------------------------------------------------------------------------

TEST(ParserTests, Background_Simple) {
    assert_parse_ok_and_ast_equals(
        "sleep 10 &\n",
        "Command: sleep &\n"
        "  Args: 10\n"
    );
}

TEST(ParserTests, Background_WithRedirect) {
    assert_parse_ok_and_ast_equals(
        "cmd > out &\n",
        "Command: cmd &\n"
        "  Redirections:\n"
        "    > out\n"
    );
}

// -----------------------------------------------------------------------------
// Pipeline tests
// -----------------------------------------------------------------------------

TEST(ParserTests, Pipeline_TwoCommands) {
    assert_parse_ok_and_ast_equals(
        "ls | grep foo\n",
        "Pipeline:\n"
        "  Command: ls\n"
        "  Command: grep\n"
        "    Args: foo\n"
    );
}

TEST(ParserTests, Pipeline_ThreeCommands) {
    assert_parse_ok_and_ast_equals(
        "cat file | grep foo | sort\n",
        "Pipeline:\n"
        "  Command: cat\n"
        "    Args: file\n"
        "  Command: grep\n"
        "    Args: foo\n"
        "  Command: sort\n"
    );
}

TEST(ParserTests, Pipeline_RedirectionOnLast) {
    assert_parse_ok_and_ast_equals(
        "cat file | grep foo > out\n",
        "Pipeline:\n"
        "  Command: cat\n"
        "    Args: file\n"
        "  Command: grep\n"
        "    Args: foo\n"
        "    Redirections:\n"
        "      > out\n"
    );
}

TEST(ParserTests, Pipeline_BackgroundOnLast) {
    assert_parse_ok_and_ast_equals(
        "cat file | grep foo &\n",
        "Pipeline:\n"
        "  Command: cat\n"
        "    Args: file\n"
        "  Command: grep &\n"
        "    Args: foo\n"
    );
}

// -----------------------------------------------------------------------------
// Sequence tests
// -----------------------------------------------------------------------------

TEST(ParserTests, Sequence_TwoCommands) {
    assert_parse_program_ok_and_ast_equals(
        "echo one; echo two\n",
        "Sequence:\n"
        "  Command: echo\n"
        "    Args: one\n"
        "  Command: echo\n"
        "    Args: two\n"
    );
}

TEST(ParserTests, Sequence_WithPipeline) {
    assert_parse_program_ok_and_ast_equals(
        "echo start; ls | grep txt; echo end\n",
        "Sequence:\n"
        "  Command: echo\n"
        "    Args: start\n"
        "  Pipeline:\n"
        "    Command: ls\n"
        "    Command: grep\n"
        "      Args: txt\n"
        "  Command: echo\n"
        "    Args: end\n"
    );
}

TEST(ParserTests, Sequence_TrailingSemicolon) {
    assert_parse_program_ok_and_ast_equals(
        "echo one; echo two;\n",
        "Sequence:\n"
        "  Command: echo\n"
        "    Args: one\n"
        "  Command: echo\n"
        "    Args: two\n"
    );
}

// -----------------------------------------------------------------------------
// Mixed programs
// -----------------------------------------------------------------------------

TEST(ParserTests, Mixed_CommentsAssignmentsCommands) {
    assert_parse_program_ok_and_ast_equals(
        "# header\nlet x = 5\necho x\n",
        "Comment: header\n"
        "Assignment: x = 5\n"
        "Command: echo\n"
        "  Args: x\n"
    );
}

TEST(ParserTests, Mixed_PipelineBackgroundAssignment) {
    assert_parse_program_ok_and_ast_equals(
        "let f = data.txt\ncat $f | grep foo > out &\n",
        "Assignment: f = data.txt\n"
        "Pipeline:\n"
        "  Command: cat\n"
        "    Args: $f\n"
        "  Command: grep &\n"
        "    Args: foo\n"
        "    Redirections:\n"
        "      > out\n"
    );
}

// -----------------------------------------------------------------------------
// Tricky edge cases
// -----------------------------------------------------------------------------

TEST(ParserTests, Tricky_LeadingSemicolon) {
    assert_parse_fails("; echo hi\n");
}

TEST(ParserTests, Tricky_PipeAtStart) {
    assert_parse_fails("| grep foo\n");
}

TEST(ParserTests, Tricky_PipeAtEnd) {
    assert_parse_fails("ls |\n");
}

TEST(ParserTests, Tricky_RedirectionWithoutTarget) {
    assert_parse_fails("ls > \n");
}

TEST(ParserTests, Tricky_BackgroundWithoutCommand) {
    assert_parse_fails("&\n");
}

TEST(ParserTests, Tricky_SequenceWithMissingCommand) {
    assert_parse_fails("echo hi; ; echo bye\n");
}

TEST(ParserTests, Tricky_DoublePipe) {
    assert_parse_fails("ls || grep foo\n"); // not supported yet
}

TEST(ParserTests, Tricky_DoubleAmpersand) {
    assert_parse_fails("ls && echo hi\n"); // not supported yet
}

TEST(ParserTests, Tricky_WhitespaceOnly) {
    auto result = parse_line("     \n");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result.value()->empty());
}

TEST(ParserTests, Tricky_EmptyProgram) {
    auto result = parse_program("");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result.value()->empty());
}

