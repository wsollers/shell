#include "shell/lexer.hpp"
#include "shell/parser.hpp"
#include <gtest/gtest.h>

using namespace shell;

static ParseResult parse_line(std::string const& s) {
  Lexer lx;
  auto lr = lx.lex(s + "\n");
  Parser p;
  return p.parse(lr.toks);
}

// Test parsing a basic command with command name and single argument
// Expected: Successfully parse "echo hello" into a Command with argv[0]="echo", argv[1]="hello"
// The command should not be marked as background and should be the only item in the sequence
/*
TEST(Parser, SimpleCommand) {
  auto pr = parse_line("echo hello");
  ASSERT_FALSE(pr.err.has_value());
  ASSERT_EQ(pr.seq.items.size(), 1u);
  EXPECT_FALSE(pr.seq.items[0].background);

  auto& n = pr.arena.at(pr.seq.items[0].node).v;
  auto* c = std::get_if<Command>(&n);
  ASSERT_NE(c, nullptr);
  ASSERT_EQ(c->argv.size(), 2u);
  EXPECT_EQ(c->argv[0], "echo");
  EXPECT_EQ(c->argv[1], "hello");
}
*/

// Test parsing commands with various quoting mechanisms and escape sequences
// Expected: Parse double quotes, single quotes, and backslash escapes correctly
// "a b" should become "a b", 'c d' should become "c d", e\ f should become "e f"
/*
TEST(Parser, QuotesAndEscapes) {
  auto pr = parse_line(R"(echo "a b" 'c d' e\ f)");
  ASSERT_FALSE(pr.err.has_value());
  auto* c = std::get_if<Command>(&pr.arena.at(pr.seq.items[0].node).v);
  ASSERT_NE(c, nullptr);
  ASSERT_EQ(c->argv.size(), 4u);
  EXPECT_EQ(c->argv[1], "a b");
  EXPECT_EQ(c->argv[2], "c d");
  EXPECT_EQ(c->argv[3], "e f");
}
*/

// Test parsing input and output redirections
// Expected: Parse "cat < in.txt > out.txt" with two redirections:
// - Input redirection from "in.txt" (RedirKind::IN)
// - Output redirection to "out.txt" (RedirKind::OUT_TRUNC)
/*
TEST(Parser, Redirections) {
  auto pr = parse_line("cat < in.txt > out.txt");
  ASSERT_FALSE(pr.err.has_value());
  auto* c = std::get_if<Command>(&pr.arena.at(pr.seq.items[0].node).v);
  ASSERT_NE(c, nullptr);
  ASSERT_EQ(c->redirs.size(), 2u);
  EXPECT_EQ(c->redirs[0].kind, RedirKind::IN);
  EXPECT_EQ(c->redirs[0].target, "in.txt");
  EXPECT_EQ(c->redirs[1].kind, RedirKind::OUT_TRUNC);
  EXPECT_EQ(c->redirs[1].target, "out.txt");
}
*/

// Test operator precedence between pipes and logical operators
// Expected: Parse "a | b && c" as ((a | b) && c), not (a | (b && c))
// The pipeline (a | b) should be the left operand of the logical AND
/*
TEST(Parser, PipelinePrecedence) {
  auto pr = parse_line("a | b && c");
  ASSERT_FALSE(pr.err.has_value());
  auto& root = pr.arena.at(pr.seq.items[0].node).v;
  auto* log = std::get_if<Logical>(&root);
  ASSERT_NE(log, nullptr);
  EXPECT_EQ(log->op, LogicalOp::AND_IF);

  auto* lhsPipe = std::get_if<Pipeline>(&pr.arena.at(log->lhs).v);
  ASSERT_NE(lhsPipe, nullptr);
  ASSERT_EQ(lhsPipe->cmds.size(), 2u);

  auto* rhsCmd = std::get_if<Command>(&pr.arena.at(log->rhs).v);
  ASSERT_NE(rhsCmd, nullptr);
}
*/

// Test parsing background processes and sequential commands
// Expected: Parse "a & b ; c &" as three separate list items:
// - "a" marked as background (true)
// - "b" not marked as background (false) 
// - "c" marked as background (true)
/*
TEST(Parser, BackgroundListItems) {
  auto pr = parse_line("a & b ; c &");
  ASSERT_FALSE(pr.err.has_value());
  ASSERT_EQ(pr.seq.items.size(), 3u);
  EXPECT_TRUE(pr.seq.items[0].background);
  EXPECT_FALSE(pr.seq.items[1].background);
  EXPECT_TRUE(pr.seq.items[2].background);
}
*/

// Test that various syntax errors are properly detected and reported
// Expected: All of these malformed inputs should result in parse errors:
// - "| a" (pipe at start of command)
// - "a &&" (logical operator with missing right operand)
// - "a >" (redirection with missing target)
// - "a & & b" (invalid token sequence with double ampersand)
TEST(Parser, SyntaxErrors) {
  auto pr = parse_line("| a");
  ASSERT_TRUE(pr.err.has_value());

  pr = parse_line("a &&");
  ASSERT_TRUE(pr.err.has_value());

  pr = parse_line("a >");
  ASSERT_TRUE(pr.err.has_value());

  pr = parse_line("a & & b");
  ASSERT_TRUE(pr.err.has_value());
}
