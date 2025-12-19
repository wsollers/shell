// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/parser.hpp"
#include "shell/lexer.hpp"
#include "shell/ast.hpp"

#include <utility>
#include <variant>

namespace wshell {

namespace {

// Internal helper variants for parser stages:
// - A pipeline is either a single Command or a PipelineNode.
// - A list (sequence) is either Command, Pipeline, or SequenceNode.

using PipelineVariant = std::variant<
    std::unique_ptr<CommandNode>,
    std::unique_ptr<PipelineNode>
>;

using ListVariant = std::variant<
    std::unique_ptr<CommandNode>,
    std::unique_ptr<PipelineNode>,
    std::unique_ptr<SequenceNode>
>;

RedirectKind redirect_kind_from_lexeme(const std::string& s) {
    if (s == ">")  return RedirectKind::Output;
    if (s == ">>") return RedirectKind::Append;
    return RedirectKind::Input; // "<"
}

} // namespace

// -----------------------------------------------------------------------------
// Token helpers
// -----------------------------------------------------------------------------

Token Parser::current_token() {
    return lexer_.next_token();
}

Token Parser::peek_token() {
    return lexer_.peek_token();
}

bool Parser::check(TokenType type) {
    return peek_token().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        (void)current_token();
        return true;
    }
    return false;
}

void Parser::skip_newlines() {
    while (match(TokenType::Newline)) {
        // consume all consecutive newlines
    }
}

ParseError Parser::make_error(const std::string& msg) {
    Token t = peek_token();
    return ParseError(msg, t.line, t.column);
}

// -----------------------------------------------------------------------------
// Comments
// -----------------------------------------------------------------------------

std::expected<std::unique_ptr<CommentNode>, ParseError>
Parser::parse_comment() {
    Token tok = current_token(); // consume
    if (tok.type != TokenType::Comment) {
        return std::unexpected(make_error("Expected comment"));
    }
    return make_comment(tok.value);
}

// -----------------------------------------------------------------------------
// Assignments: let x = value
// -----------------------------------------------------------------------------

std::expected<std::unique_ptr<AssignmentNode>, ParseError>
Parser::parse_assignment() {
    Token let_tok = current_token();
    if (let_tok.type != TokenType::Let) {
        return std::unexpected(make_error("Expected 'let' keyword"));
    }

    Token var_tok = current_token();
    if (var_tok.type != TokenType::Identifier) {
        return std::unexpected(make_error("Expected variable name after 'let'"));
    }
    std::string variable = var_tok.value;

    Token eq_tok = current_token();
    if (eq_tok.type != TokenType::Equals) {
        return std::unexpected(make_error("Expected '=' after variable name"));
    }

    std::string value;
    while (!check(TokenType::Newline) &&
           !check(TokenType::EndOfFile) &&
           !check(TokenType::Semicolon)) {
        Token t = current_token();
        if (!value.empty()) {
            value.push_back(' ');
        }
        value += t.value;
    }

    if (value.empty()) {
        return std::unexpected(make_error("Expected value after '='"));
    }

    return make_assignment(std::move(variable), std::move(value));
}

// -----------------------------------------------------------------------------
// Redirections: <, >, >>  (attach to a command)
// -----------------------------------------------------------------------------

std::expected<Redirection, ParseError>
Parser::parse_redirection() {
    Token op = current_token();
    if (op.type != TokenType::Redirect) {
        return std::unexpected(make_error("Expected redirection operator"));
    }

    Token target = current_token();
    if (target.type != TokenType::Identifier) {
        return std::unexpected(make_error("Expected redirection target"));
    }

    return Redirection{ redirect_kind_from_lexeme(op.value), target.value };
}

// -----------------------------------------------------------------------------
// Simple command: name + args (no redirects, no &, no |, no ;)
// -----------------------------------------------------------------------------

std::expected<std::unique_ptr<CommandNode>, ParseError>
Parser::parse_simple_command() {
    Token cmd_tok = current_token();
    if (cmd_tok.type != TokenType::Identifier) {
        return std::unexpected(make_error("Expected command name"));
    }

    std::string name = cmd_tok.value;
    std::vector<std::string> args;

    while (!check(TokenType::Newline) &&
           !check(TokenType::EndOfFile) &&
           !check(TokenType::Pipe) &&
           !check(TokenType::Semicolon) &&
           !check(TokenType::Background) &&
           !check(TokenType::Redirect)) {
        Token t = current_token();
        if (t.type == TokenType::Identifier || t.type == TokenType::Equals) {
            args.push_back(t.value);
        } else {
            break;
        }
    }

    return make_command(std::move(name), std::move(args));
}

// -----------------------------------------------------------------------------
// Full command: simple + redirections + optional background
// -----------------------------------------------------------------------------

std::expected<std::unique_ptr<CommandNode>, ParseError>
Parser::parse_command() {
    auto simple = parse_simple_command();
    if (!simple) {
        return std::unexpected(simple.error());
    }

    auto cmd = std::move(*simple);

    // Attach redirections
    while (check(TokenType::Redirect)) {
        auto redir = parse_redirection();
        if (!redir) {
            return std::unexpected(redir.error());
        }
        cmd->redirections.push_back(std::move(*redir));
    }

    // Optional background flag
    if (match(TokenType::Background)) {
        cmd->background = true;
    }

    return cmd;
}

// -----------------------------------------------------------------------------
// Pipeline: cmd ('|' cmd)*
// Returns either:
//   - std::unique_ptr<CommandNode> (no pipe)
//   - std::unique_ptr<PipelineNode> (one or more pipes)
// via PipelineVariant.
// -----------------------------------------------------------------------------

std::expected<PipelineVariant, ParseError>
Parser::parse_pipeline_variant() {
    auto first = parse_command();
    if (!first) {
        return std::unexpected(first.error());
    }

    std::vector<std::unique_ptr<CommandNode>> cmds;
    cmds.push_back(std::move(*first));

    while (match(TokenType::Pipe)) {
        auto next = parse_command();
        if (!next) {
            return std::unexpected(next.error());
        }
        cmds.push_back(std::move(*next));
    }

    if (cmds.size() == 1) {
        // Just a single command
        return PipelineVariant{ std::move(cmds.front()) };
    }

    // Multiple commands: build a PipelineNode
    auto pipeline = make_pipeline(std::move(cmds));
    return PipelineVariant{ std::move(pipeline) };
}

// -----------------------------------------------------------------------------
// List / sequence: pipeline (';' pipeline)*
// Returns either:
//   - CommandNode
//   - PipelineNode
//   - SequenceNode
// via ListVariant.
// -----------------------------------------------------------------------------

std::expected<ListVariant, ParseError>
Parser::parse_list_variant() {
    auto first_pipe = parse_pipeline_variant();
    if (!first_pipe) {
        return std::unexpected(first_pipe.error());
    }

    // If there is no ';', we just return the pipeline result directly.
    if (!check(TokenType::Semicolon)) {
        // No sequence, just one pipeline/command.
        // Promote PipelineVariant to ListVariant.
        return std::visit(
            [](auto ptr) -> ListVariant {
                return ListVariant{ std::move(ptr) };
            },
            std::move(*first_pipe)
        );
    }

    // There is at least one ';' → we are building a SequenceNode.
    std::vector<std::unique_ptr<ASTNode>> stmts;

    // Move first pipeline into ASTNode
    std::visit(
        [&](auto& ptr) {
            // upcast unique_ptr<Derived> to unique_ptr<ASTNode>
            std::unique_ptr<ASTNode> base(ptr.release());
            stmts.push_back(std::move(base));
        },
        *first_pipe
    );

    while (match(TokenType::Semicolon)) {
        // Allow trailing semicolon before newline/EOF
        if (check(TokenType::Newline) || check(TokenType::EndOfFile)) {
            break;
        }

        auto next_pipe = parse_pipeline_variant();
        if (!next_pipe) {
            return std::unexpected(next_pipe.error());
        }

        std::visit(
            [&](auto& ptr) {
                std::unique_ptr<ASTNode> base(ptr.release());
                stmts.push_back(std::move(base));
            },
            *next_pipe
        );
    }

    auto seq = make_sequence(std::move(stmts));
    return ListVariant{ std::move(seq) };
}

// -----------------------------------------------------------------------------
// Statement: comment | assignment | list(pipeline/sequence/command)
// -----------------------------------------------------------------------------

std::expected<StatementNode, ParseError>
Parser::parse_statement() {
    skip_newlines();

    if (check(TokenType::EndOfFile)) {
        return std::unexpected(make_error("Unexpected end of input"));
    }

    // Comment
    if (check(TokenType::Comment)) {
        auto c = parse_comment();
        if (!c) {
            return std::unexpected(c.error());
        }
        return StatementNode{ std::move(*c) };
    }

    // Assignment
    if (check(TokenType::Let)) {
        auto a = parse_assignment();
        if (!a) {
            return std::unexpected(a.error());
        }
        return StatementNode{ std::move(*a) };
    }

    // Command / Pipeline / Sequence
    auto list = parse_list_variant();
    if (!list) {
        return std::unexpected(list.error());
    }

    // Convert ListVariant into StatementNode variant (same underlying node types)
    return std::visit(
        [](auto ptr) -> StatementNode {
            return StatementNode{ std::move(ptr) };
        },
        std::move(*list)
    );
}

// -----------------------------------------------------------------------------
// Program: many statements (for full scripts)
// -----------------------------------------------------------------------------

std::expected<std::unique_ptr<ProgramNode>, ParseError>
Parser::parse_program() {
    auto program = std::make_unique<ProgramNode>();

    skip_newlines();

    while (!check(TokenType::EndOfFile)) {
        auto stmt = parse_statement();
        if (!stmt) {
            return std::unexpected(stmt.error());
        }
        program->add_statement(std::move(*stmt));
        skip_newlines();
    }

    return program;
}

// -----------------------------------------------------------------------------
// parse_line: a single logical line (for REPL)
// -----------------------------------------------------------------------------

std::expected<std::unique_ptr<ProgramNode>, ParseError>
Parser::parse_line() {
    auto program = std::make_unique<ProgramNode>();

    // Treat empty input as empty program
    if (check(TokenType::EndOfFile) || check(TokenType::Newline)) {
        return program;
    }

    auto stmt = parse_statement();
    if (!stmt) {
        return std::unexpected(stmt.error());
    }

    program->add_statement(std::move(*stmt));

    // Allow a single trailing newline.
    if (check(TokenType::Newline)) {
        (void)current_token();
    }

    // No extra tokens allowed after the statement in parse_line.
    if (!check(TokenType::EndOfFile)) {
        return std::unexpected(ParseError{"Unexpected tokens after statement"});
    }

    return program;
}

} // namespace wshell