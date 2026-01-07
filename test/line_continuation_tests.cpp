#include "shell/ast.hpp"
#include "shell/ast_printer.hpp"
#include "shell/parser.hpp"
#include <gtest/gtest.h>

using namespace wshell;

std::string parse_error_toString(ParseErrorKind err) {
    if (err == ParseErrorKind::IncompleteInput) {
        return "Incomplete Input\n";
    }
    return "Syntax Error\n";
}


static void expect_syntax_error(const std::string& input) {
    auto result = wshell::parse_line(input);
    ASSERT_FALSE(result.has_value()) << "Expected parse error, got success";
    std::cerr << "Parse error: " << result.error().to_string() << std::endl;
    ASSERT_EQ(result.error().kind_, wshell::ParseErrorKind::SyntaxError);
}

static void expect_ok(const std::string& input) {
    auto result = wshell::parse_line(input);
    ASSERT_TRUE(result.has_value());
}
// -----------------------------------------------------------------------------
// Focused investigation: bracket the error for 'let x = 42 ; echo hi |'
// -----------------------------------------------------------------------------
// (Moved after helper functions and includes)
// -----------------------------------------------------------------------------
// Expanded diagnostic tests for assignment, semicolon, and pipe edge cases
// -----------------------------------------------------------------------------

// Assignment with multiple semicolons and pipes
TEST(ParserContinuation_Diagnostics, AssignmentSemicolonPipeCommand) {
    expect_syntax_error("FOO=bar; | grep foo");
}

TEST(ParserContinuation_Diagnostics, AssignmentSemicolonPipeSemicolon) {
    expect_syntax_error("FOO=bar; |;");
}

TEST(ParserContinuation_Diagnostics, AssignmentSemicolonPipePipe) {
    expect_syntax_error("FOO=bar; ||");
}

TEST(ParserContinuation_Diagnostics, AssignmentSemicolonCommandPipe) {
    expect_syntax_error("FOO=bar; echo hi |");
}

TEST(ParserContinuation_Diagnostics, AssignmentSemicolonCommandPipeCommand) {
    expect_ok("FOO=bar; echo hi | grep foo");
}

TEST(ParserContinuation_Diagnostics, AssignmentSemicolonCommandPipeSemicolon) {
    expect_syntax_error("FOO=bar; echo hi |;");
}

TEST(ParserContinuation_Diagnostics, AssignmentSemicolonCommandPipePipe) {
    expect_syntax_error("FOO=bar; echo hi ||");
}

// Chained assignments and pipes
TEST(ParserContinuation_Diagnostics, MultipleAssignmentsSemicolonPipe) {
    expect_syntax_error("FOO=bar; BAZ=qux |");
}

TEST(ParserContinuation_Diagnostics, MultipleAssignmentsSemicolonPipeCommand) {
    expect_ok("FOO=bar; BAZ=qux | grep foo");
}

TEST(ParserContinuation_Diagnostics, MultipleAssignmentsSemicolonCommandPipe) {
    expect_syntax_error("FOO=bar; BAZ=qux; echo hi |");
}

TEST(ParserContinuation_Diagnostics, MultipleAssignmentsSemicolonCommandPipeCommand) {
    expect_ok("FOO=bar; BAZ=qux; echo hi | grep foo");
}

// Nested semicolons and pipes
TEST(ParserContinuation_Diagnostics, SemicolonPipeSemicolon) {
    expect_syntax_error("; | ;");
}

TEST(ParserContinuation_Diagnostics, PipeSemicolonPipe) {
    expect_syntax_error("| ; |");
}

TEST(ParserContinuation_Diagnostics, SemicolonPipeCommand) {
    expect_syntax_error("; | grep foo");
}

TEST(ParserContinuation_Diagnostics, PipeSemicolonCommand) {
    expect_syntax_error("| ; grep foo");
}

// Assignment with whitespace and comments
TEST(ParserContinuation_Diagnostics, AssignmentSemicolonPipeComment) {
    expect_syntax_error("FOO=bar; | # comment");
}

TEST(ParserContinuation_Diagnostics, AssignmentSemicolonCommandPipeComment) {
    expect_syntax_error("FOO=bar; echo hi | # comment");
}
// End of file

// -----------------------------------------------------------------------------
// Focused investigation: bracket the error for 'let x = 42 ; echo hi |'
// -----------------------------------------------------------------------------
// Control: just a pipeline after let assignment (should be ok)
TEST(ParserContinuation_Focused, LetAssignmentThenCommand) {
    expect_ok("let x = 42 ; echo hi");
}

// Control: just a pipeline after let assignment, no semicolon (should be incomplete)
TEST(ParserContinuation_Focused, LetAssignmentCommandPipe) {
    expect_ok("let x = 42 echo hi |");
}

// Control: just a pipeline after command (should be incomplete)
TEST(ParserContinuation_Focused, CommandPipe) {
    expect_syntax_error("echo hi |");
}

// Control: let assignment only (should be ok)
TEST(ParserContinuation_Focused, LetAssignmentOnly) {
    expect_ok("let x = 42");
}

// Control: let assignment with semicolon only (should be ok)
TEST(ParserContinuation_Focused, LetAssignmentSemicolon) {
    expect_ok("let x = 42 ;");
}

// Variant: let assignment, semicolon, command, no pipe (should be ok)
TEST(ParserContinuation_Focused, LetAssignmentSemicolonCommand) {
    expect_ok("let x = 42 ; echo hi");
}

// Variant: let assignment, semicolon, command, pipe (should be incomplete)
TEST(ParserContinuation_Focused, LetAssignmentSemicolonCommandPipe) {
    expect_syntax_error("let x = 42 ; echo hi |");
}

// Variant: let assignment, semicolon, pipe only (should be incomplete)
TEST(ParserContinuation_Focused, LetAssignmentSemicolonPipe) {
    expect_syntax_error("let x = 42 ; |");
}

// Variant: let assignment, command, pipe (should be incomplete)
TEST(ParserContinuation_Focused, LetAssignmentCommandPipeNoSemicolon) {
    expect_ok("let x = 42 echo hi |");
}

// Variant: assignment (no let), semicolon, command, pipe (should be incomplete)
TEST(ParserContinuation_Focused, AssignmentSemicolonCommandPipe) {
    expect_syntax_error("FOO=bar ; echo hi |");
}

// Variant: assignment (no let), semicolon, pipe (should be incomplete)
TEST(ParserContinuation_Focused, AssignmentSemicolonPipe) {
    expect_syntax_error("FOO=bar ; |");
}

// Variant: assignment (no let), command, pipe (should be incomplete)
TEST(ParserContinuation_Focused, AssignmentCommandPipeNoSemicolon) {
    expect_syntax_error("FOO=bar echo hi |");
}

// -----------------------------------------------------------------------------
// PipelineNeedsMoreInput
// -----------------------------------------------------------------------------

TEST(ParserContinuation_PipelineNeedsMoreInput, EchoFooPipe) {
    expect_syntax_error("echo foo |");
}

TEST(ParserContinuation_PipelineNeedsMoreInput, LsDashLPipeSpaces) {
    expect_syntax_error("ls -l |   ");
}

TEST(ParserContinuation_PipelineNeedsMoreInput, CatFilePipeComment) {
    expect_syntax_error("cat file | # comment");
}

// -----------------------------------------------------------------------------
// PipelineSyntaxErrors
// -----------------------------------------------------------------------------

TEST(ParserContinuation_PipelineSyntaxErrors, StartsWithPipe) {
    expect_syntax_error("| echo foo");
}

TEST(ParserContinuation_PipelineSyntaxErrors, DoublePipeUnsupported) {
    expect_syntax_error("echo foo || bar");
}

// -----------------------------------------------------------------------------
// RedirectNeedsMoreInput
// -----------------------------------------------------------------------------

TEST(ParserContinuation_RedirectNeedsMoreInput, LsGreater) {
    expect_syntax_error("ls >");
}

TEST(ParserContinuation_RedirectNeedsMoreInput, CatLessSpaces) {
    expect_syntax_error("cat <   ");
}

TEST(ParserContinuation_RedirectNeedsMoreInput, EchoHiAppend) {
    expect_syntax_error("echo hi >>");
}

// -----------------------------------------------------------------------------
// RedirectSyntaxErrors
// -----------------------------------------------------------------------------

TEST(ParserContinuation_RedirectSyntaxErrors, RedirectPipe) {
    expect_syntax_error("ls > |");
}

TEST(ParserContinuation_RedirectSyntaxErrors, RedirectSemicolon) {
    expect_syntax_error("ls > ;");
}

TEST(ParserContinuation_RedirectSyntaxErrors, RedirectHash) {
    expect_syntax_error("ls > #");
}

// -----------------------------------------------------------------------------
// AssignmentNeedsMoreInput
// -----------------------------------------------------------------------------

TEST(ParserContinuation_AssignmentNeedsMoreInput, LetXEquals) {
    expect_ok("let x =");
}

TEST(ParserContinuation_AssignmentNeedsMoreInput, LetVarEqualsSpaces) {
    expect_ok("let var =   ");
}

// -----------------------------------------------------------------------------
// AssignmentSyntaxErrors
// -----------------------------------------------------------------------------

TEST(ParserContinuation_AssignmentSyntaxErrors, MissingIdentifier) {
    expect_syntax_error("let = 5");
}

TEST(ParserContinuation_AssignmentSyntaxErrors, MissingEquals) {
    expect_syntax_error("let x y");
}

// -----------------------------------------------------------------------------
// SequenceNeedsMoreInput
// -----------------------------------------------------------------------------

TEST(ParserContinuation_SequenceNeedsMoreInput, EchoHiSemicolon) {
    expect_ok("echo hi;");
}

TEST(ParserContinuation_SequenceNeedsMoreInput, LsDashLSemicolonSpaces) {
    expect_ok("ls -l ;   ");
}

// -----------------------------------------------------------------------------
// SequenceSyntaxErrors
// -----------------------------------------------------------------------------

TEST(ParserContinuation_SequenceSyntaxErrors, StartsWithSemicolon) {
    expect_syntax_error("; echo hi");
}

TEST(ParserContinuation_SequenceSyntaxErrors, DoubleSemicolon) {
    expect_syntax_error("echo hi;;");
}

// -----------------------------------------------------------------------------
// MixedContinuationCases
// -----------------------------------------------------------------------------

TEST(ParserContinuation_MixedContinuationCases, PipeThenSemicolon) {
    expect_ok("echo hi | grep h ;");
}

TEST(ParserContinuation_MixedContinuationCases, AssignmentThenPipe) {
    expect_syntax_error("let x = 42 ; echo hi |");
}

TEST(ParserContinuation_MixedContinuationCases, RedirectThenPipe) {
    expect_syntax_error("ls > out.txt ; echo hi >");
}

// -----------------------------------------------------------------------------
// MixedSyntaxErrors
// -----------------------------------------------------------------------------

TEST(ParserContinuation_MixedSyntaxErrors, PipeSemicolon) {
    expect_syntax_error("echo hi | ;");
}

TEST(ParserContinuation_MixedSyntaxErrors, LetXEqualsSemicolon) {
    expect_ok("let x = ;");
}

TEST(ParserContinuation_MixedSyntaxErrors, RedirectPipeGrep) {
    expect_syntax_error("ls > | grep");
}

// -----------------------------------------------------------------------------
// CompleteStatements
// -----------------------------------------------------------------------------

TEST(ParserContinuation_CompleteStatements, EchoHi) {
    expect_ok("echo hi");
}

TEST(ParserContinuation_CompleteStatements, EchoPipeGrep) {
    expect_ok("echo hi | grep h");
}

TEST(ParserContinuation_CompleteStatements, RedirectOut) {
    expect_ok("ls > out.txt");
}

TEST(ParserContinuation_CompleteStatements, LetX42) {
    expect_ok("let x = 42");
}

TEST(ParserContinuation_CompleteStatements, TwoCommandsSemicolon) {
    expect_ok("echo hi; echo bye");
}

TEST(ParserContinuation_CompleteStatements, TwoCommandsSpacedSemicolon) {
    expect_ok("echo hi ; echo bye");
}

// -----------------------------------------------------------------------------
// EdgeCases
// -----------------------------------------------------------------------------

TEST(ParserContinuation_EdgeCases, PipeComment) {
    expect_syntax_error("echo hi | # comment");
}

TEST(ParserContinuation_EdgeCases, LetComment) {
    expect_ok("let x = # comment");
}

TEST(ParserContinuation_EdgeCases, RedirectComment) {
    expect_syntax_error("ls > # comment");
}

TEST(ParserContinuation_EdgeCases, JustACommentSyntaxError) {
    expect_ok("# just a comment");
}

TEST(ParserContinuation_EdgeCases, CommentOnlyOk) {
    expect_ok("# comment only");
}

TEST(ParserContinuation_EdgeCases, RedirectMissingTarget) {
    expect_syntax_error("ls >");
}

// -----------------------------------------------------------------------------
// Diagnostic tests for assignment and pipe edge cases
// -----------------------------------------------------------------------------

TEST(ParserContinuation_Diagnostics, MinimalAssignmentThenPipe) {
    expect_syntax_error("FOO=bar |");
}

TEST(ParserContinuation_Diagnostics, AssignmentThenPipeThenCommand) {
    expect_ok("FOO=bar | echo hi");
}

TEST(ParserContinuation_Diagnostics, AssignmentSemicolonPipe) {
    expect_syntax_error("FOO=bar; |");
}

TEST(ParserContinuation_Diagnostics, PipeAtStart) {
    expect_syntax_error("| echo hi");
}

TEST(ParserContinuation_Diagnostics, PipeAtEnd) {
    expect_syntax_error("echo hi |");
}

TEST(ParserContinuation_Diagnostics, MultipleAssignmentsThenPipe) {
    expect_syntax_error("FOO=bar BAZ=qux |");
}

TEST(ParserContinuation_Diagnostics, AssignmentSemicolonCommand) {
    // Should parse as complete, not error
    auto result = wshell::parse_line("FOO=bar; echo hi");
    ASSERT_TRUE(result.has_value()) << "Expected success, got error: " << result.error().to_string();
}

TEST(ParserContinuation_Diagnostics, AssignmentOnly) {
    auto result = wshell::parse_line("FOO=bar");
    ASSERT_TRUE(result.has_value()) << "Expected success, got error: " << result.error().to_string();
}

TEST(ParserContinuation_Diagnostics, SemicolonOnly) {
    expect_syntax_error(";");
}

TEST(ParserContinuation_Diagnostics, AssignmentWhitespaceSemicolonPipe) {
    expect_syntax_error("FOO=bar ; |");
    expect_syntax_error("FOO=bar;|");
}
