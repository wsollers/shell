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

TEST(Parser, Redirections) {
  auto pr = parse_line("cat < in.txt > out.txt");
  ASSERT_FALSE(pr.err.has_value());
  auto* c = std::get_if<Command>(&pr.arena.at(pr.seq.items[0].node).v);
  ASSERT_NE(c, nullptr);
  ASSERT_EQ(c->redirs.size(), 2u);
  EXPECT_EQ(c->redirs[0].kind, RedirKind::In);
  EXPECT_EQ(c->redirs[0].target, "in.txt");
  EXPECT_EQ(c->redirs[1].kind, RedirKind::OutTrunc);
  EXPECT_EQ(c->redirs[1].target, "out.txt");
}

TEST(Parser, PipelinePrecedence) {
  auto pr = parse_line("a | b && c");
  ASSERT_FALSE(pr.err.has_value());
  auto& root = pr.arena.at(pr.seq.items[0].node).v;
  auto* log = std::get_if<Logical>(&root);
  ASSERT_NE(log, nullptr);
  EXPECT_EQ(log->op, LogicalOp::AndIf);

  auto* lhsPipe = std::get_if<Pipeline>(&pr.arena.at(log->lhs).v);
  ASSERT_NE(lhsPipe, nullptr);
  ASSERT_EQ(lhsPipe->cmds.size(), 2u);

  auto* rhsCmd = std::get_if<Command>(&pr.arena.at(log->rhs).v);
  ASSERT_NE(rhsCmd, nullptr);
}

TEST(Parser, BackgroundListItems) {
  auto pr = parse_line("a & b ; c &");
  ASSERT_FALSE(pr.err.has_value());
  ASSERT_EQ(pr.seq.items.size(), 3u);
  EXPECT_TRUE(pr.seq.items[0].background);
  EXPECT_FALSE(pr.seq.items[1].background);
  EXPECT_TRUE(pr.seq.items[2].background);
}

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
