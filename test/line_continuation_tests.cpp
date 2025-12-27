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

static void expect_incomplete(const std::string& input) {
    auto result = wshell::parse_line(input);
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().kind_, wshell::ParseErrorKind::IncompleteInput);
}

static void expect_syntax_error(const std::string& input) {
    auto result = wshell::parse_line(input);
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().kind_, wshell::ParseErrorKind::SyntaxError);
}

static void expect_ok(const std::string& input) {
    auto result = wshell::parse_line(input);
    ASSERT_TRUE(result.has_value());
}

// -----------------------------------------------------------------------------
// PipelineNeedsMoreInput
// -----------------------------------------------------------------------------

TEST(ParserContinuation_PipelineNeedsMoreInput, EchoFooPipe) {
    expect_incomplete("echo foo |");
}

TEST(ParserContinuation_PipelineNeedsMoreInput, LsDashLPipeSpaces) {
    expect_incomplete("ls -l |   ");
}

TEST(ParserContinuation_PipelineNeedsMoreInput, CatFilePipeComment) {
    expect_incomplete("cat file | # comment");
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
    expect_incomplete("let x = 42 ; echo hi |");
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
    expect_incomplete("echo hi | # comment");
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