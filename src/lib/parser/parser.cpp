// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/parser.hpp"

#include "shell/ast.hpp"
#include "shell/ast_printer.hpp"
#include "shell/lexer.hpp"

#include <variant>

#include <utility>

namespace wshell {

namespace {

RedirectKind redirect_kind_from_lexeme(const std::string& s) {
    if (s == ">")
        return RedirectKind::OutputTruncate;
    if (s == ">>")
        return RedirectKind::OutputAppend;
    return RedirectKind::Input;  // "<"
}

}  // namespace

// -----------------------------------------------------------------------------
// Token helpers
// -----------------------------------------------------------------------------

Token Parser::peek_token() {
    return lexer_.peek_token();
}

Token Parser::current_token() {
    return lexer_.peek_token();
}

bool Parser::check(TokenType type) {
    return lexer_.peek_token().type == type;
}

bool Parser::match(TokenType type) {
    Token t = lexer_.peek_token();
    if (t.type == type) {
        (void)lexer_.next_token();  // consume
        return true;
    }
    return false;
}

void Parser::advance() {
    (void)lexer_.next_token();
}

void Parser::skip_newlines() {
    while (match(TokenType::Newline)) {
        // consume all consecutive newlines
    }
}

ParseError Parser::make_error(ParseErrorKind theKind, const std::string& msg) {
    Token t = peek_token();
    return ParseError(ParseErrorKind{theKind}, msg, t.line, t.column);
}

// -----------------------------------------------------------------------------
// Comments
// -----------------------------------------------------------------------------

std::expected<CommentNode, ParseError> Parser::parse_comment() {
    if (!check(TokenType::Comment)) {
        return std::unexpected(make_error(ParseErrorKind::SyntaxError, "Expected comment"));
    }

    Token tok = lexer_.next_token();  // consume the comment token
    return make_comment(tok.value);
}

// -----------------------------------------------------------------------------
// Assignments: let x = value
// -----------------------------------------------------------------------------

std::expected<AssignmentNode, ParseError> Parser::parse_assignment() {
    // 'let'
    if (!match(TokenType::Let)) {
        return std::unexpected(make_error(ParseErrorKind::SyntaxError, "Expected 'let' keyword"));
    }

    // identifier
    if (!check(TokenType::Identifier)) {
        return std::unexpected(
            make_error(ParseErrorKind::SyntaxError, "Expected variable name after 'let'"));
    }
    Token var_tok = current_token();
    std::string variable = var_tok.value;
    (void)lexer_.next_token();  // consume identifier

    // '='
    if (!match(TokenType::Equals)) {
        return std::unexpected(
            make_error(ParseErrorKind::SyntaxError, "Expected '=' after variable name"));
    }

    // skip comments immediately after '='
    while (match(TokenType::Comment)) {
        // do nothing
    }

    // Allow empty value: let VAR = <newline|EOF|;>
    if (check(TokenType::Semicolon) || check(TokenType::EndOfFile) || check(TokenType::Newline)) {
        return make_assignment(std::move(variable), "");
    }

    // collect value tokens until newline/EOF/semicolon
    std::string value;
    bool first = true;
    while (!check(TokenType::Newline) && !check(TokenType::EndOfFile) &&
           !check(TokenType::Semicolon)) {
        Token t = current_token();
        if (!first) value.push_back(' ');
        first = false;
        value += t.value;
        (void)lexer_.next_token();  // consume
    }
    return make_assignment(std::move(variable), std::move(value));
}

// -----------------------------------------------------------------------------
// Redirections: <, >, >>  (attach to a command)
// -----------------------------------------------------------------------------

std::expected<Redirection, ParseError> Parser::parse_redirection() {
    if (!check(TokenType::Redirect)) {
        return std::unexpected(
            make_error(ParseErrorKind::SyntaxError, "Expected redirection operator"));
    }

    Token op = lexer_.next_token();  // consume operator

    // Detect incomplete input: "cmd >" or "cmd <"
    if (check(TokenType::EndOfFile) || check(TokenType::Newline)) {
        return std::unexpected(ParseError{ParseErrorKind::SyntaxError,
                                          "Expected redirection target", op.line, op.column});
    }

    Token target = current_token();

    // Comment after redirect is a syntax error, not incomplete input
    if (target.type == TokenType::Comment) {
        return std::unexpected(
            make_error(ParseErrorKind::SyntaxError, "Expected redirection target"));
    }

    if (target.type != TokenType::Identifier) {
        return std::unexpected(
            make_error(ParseErrorKind::SyntaxError, "Expected redirection target"));
    }

    // For now, treat all redirection targets as unquoted, needs_expansion=true
    Word target_word{target.value, false, true};
    (void)lexer_.next_token();  // consume target

    return Redirection{redirect_kind_from_lexeme(op.value), std::move(target_word)};
}

// -----------------------------------------------------------------------------
// Simple command: name + args (no redirects, no &, no |, no ;)
// -----------------------------------------------------------------------------

std::expected<CommandNode, ParseError> Parser::parse_simple_command() {
    Token cmd_tok = current_token();
    if (cmd_tok.type != TokenType::Identifier) {
        return std::unexpected(make_error(ParseErrorKind::SyntaxError, "Expected command name"));
    }

    // Command name as Word (unquoted, needs_expansion=true)
    Word name_word{cmd_tok.value, false, true};
    std::vector<Word> args;

    (void)lexer_.next_token();  // consume the command name

    // collect arguments until a control token
    while (!check(TokenType::Newline) && !check(TokenType::EndOfFile) && !check(TokenType::Pipe) &&
           !check(TokenType::Semicolon) && !check(TokenType::Background) &&
           !check(TokenType::Redirect)) {
        Token t = current_token();
        if (t.type == TokenType::Identifier || t.type == TokenType::Equals) {
            std::string val = t.value;
            bool is_quoted = false;
            // Handle quoted arguments (may contain spaces or nested quotes)
            if (!val.empty() && val.front() == '"') {
                is_quoted = true;
                val.erase(0, 1); // remove leading quote
                // Collect until closing quote (may span multiple tokens)
                while (true) {
                    // If ends with quote, remove and break
                    if (!val.empty() && val.back() == '"') {
                        val.pop_back();
                        break;
                    }
                    // Otherwise, add a space and next token
                    (void)lexer_.next_token();
                    if (check(TokenType::Newline) || check(TokenType::EndOfFile) || check(TokenType::Pipe) ||
                        check(TokenType::Semicolon) || check(TokenType::Background) || check(TokenType::Redirect)) {
                        // Unterminated quote, treat as is
                        break;
                    }
                    Token next = current_token();
                    val += ' ';
                    val += next.value;
                }
            }
            args.emplace_back(val, is_quoted, true);
            (void)lexer_.next_token();  // consume the argument (or last part of quoted)
        } else {
            break;
        }
    }

    return make_command(std::move(name_word), std::move(args));
}

// -----------------------------------------------------------------------------
// Full command: simple + redirections + optional background
// -----------------------------------------------------------------------------

std::expected<CommandNode, ParseError> Parser::parse_command() {
    auto simple = parse_simple_command();
    if (!simple) {
        return std::unexpected(simple.error());
    }

    CommandNode cmd = std::move(*simple);

    // Attach redirections
    while (check(TokenType::Redirect)) {
        auto redir = parse_redirection();
        if (!redir) {
            return std::unexpected(redir.error());
        }
        cmd.redirections.push_back(std::move(*redir));
    }

    // Optional background flag
    if (match(TokenType::Background)) {
        cmd.background = true;
    }

    return cmd;
}

// -----------------------------------------------------------------------------
// Pipeline: cmd ('|' cmd)*
// -----------------------------------------------------------------------------

std::expected<StatementNode, ParseError> Parser::parse_pipeline() {
    auto first = parse_command();
    if (!first) {
        return std::unexpected(first.error());
    }

    std::vector<CommandNode> cmds;
    cmds.push_back(std::move(*first));

    while (check(TokenType::Pipe)) {
        Token pipe_tok = peek_token();
        // Save lexer state
        auto lexer_state = lexer_;
        (void)match(TokenType::Pipe);  // consume '|'

        // Skip comments after a pipe
        while (match(TokenType::Comment)) {
            // nothing
        }

        // Double pipe: SyntaxError
        if (check(TokenType::Pipe)) {
            return std::unexpected(ParseError{ParseErrorKind::SyntaxError,
                                              "Unexpected '|' after '|'", peek_token().line,
                                              peek_token().column});
        }
        // Pipe followed by semicolon: SyntaxError
        if (check(TokenType::Semicolon)) {
            return std::unexpected(ParseError{ParseErrorKind::SyntaxError,
                                              "Unexpected ';' after '|'", peek_token().line,
                                              peek_token().column});
        }
        // Detect incomplete input: "cmd |", "cmd | # comment"
        if (check(TokenType::EndOfFile) || check(TokenType::Newline)) {
            return std::unexpected(ParseError{ParseErrorKind::IncompleteInput,
                                              "Expected command after '|'", pipe_tok.line,
                                              pipe_tok.column});
        }

        auto next = parse_command();
        if (!next) {
            // If the error is IncompleteInput, propagate it upward (dangling pipe)
            if (next.error().kind_ == ParseErrorKind::IncompleteInput) {
                return std::unexpected(next.error());
            }
            // Restore lexer state so parse_list can see the pipe
            lexer_ = lexer_state;
            break;
        }

        cmds.push_back(std::move(*next));
    }

    if (cmds.size() == 1) {
        return StatementNode{std::move(cmds.front())};
    }

    PipelineNode pipeline = make_pipeline(std::move(cmds));
    return StatementNode{std::move(pipeline)};
}

// -----------------------------------------------------------------------------
// List / sequence: pipeline (';' pipeline)*
// -----------------------------------------------------------------------------

std::expected<StatementNode, ParseError> Parser::parse_list() {
    auto first_pipe = parse_pipeline();
    if (!first_pipe.has_value()) {
        return std::unexpected(first_pipe.error());
    }

    // If there is no semicolon, return the pipeline normally
    if (!check(TokenType::Semicolon)) {
        return *first_pipe;
    }

    // There is at least one ';' → build a SequenceNode.
    std::vector<StatementNode> stmts;
    stmts.push_back(std::move(*first_pipe));

    while (match(TokenType::Semicolon)) {
        // Accept trailing semicolon at end of input (REPL or script)
        if (check(TokenType::EndOfFile) || check(TokenType::Newline)) {
            break;
        }

        // If the next token is a pipe or redirect, treat as incomplete input (continuation)
        if (check(TokenType::Pipe) || check(TokenType::Redirect)) {
            Token op_tok = peek_token();
            return std::unexpected(ParseError{ParseErrorKind::IncompleteInput,
                                              "Incomplete command at end of line",
                                              op_tok.line, op_tok.column});
        }

        auto next_pipe = parse_pipeline();
        if (!next_pipe.has_value()) {
            // Always propagate IncompleteInput upward for continuation
            if (next_pipe.error().kind_ == ParseErrorKind::IncompleteInput) {
                return std::unexpected(next_pipe.error());
            }
            // Otherwise propagate any error
            return std::unexpected(next_pipe.error());
        }

        // After parsing a pipeline inside a list, check for dangling operators (pipe/redirect)
        if (check(TokenType::Pipe) || check(TokenType::Redirect)) {
            Token op_tok = peek_token();
            return std::unexpected(ParseError{ParseErrorKind::IncompleteInput,
                                              "Incomplete command at end of line",
                                              op_tok.line, op_tok.column});
        }

        stmts.push_back(std::move(*next_pipe));
    }

    SequenceNode seq = make_sequence(std::move(stmts));
    return StatementNode{std::move(seq)};
}

// -----------------------------------------------------------------------------
// Statement: comment | assignment | list(pipeline/sequence/command)
// -----------------------------------------------------------------------------

std::expected<StatementNode, ParseError> Parser::parse_statement() {
    skip_newlines();

    if (check(TokenType::EndOfFile)) {
        return std::unexpected(make_error(ParseErrorKind::SyntaxError, "Unexpected end of input"));
    }

    // Comment
    if (check(TokenType::Comment)) {
        auto c = parse_comment();
        if (!c) {
            return std::unexpected(c.error());
        }
        return StatementNode{std::move(*c)};
    }

    // Assignment
    if (check(TokenType::Let)) {
        auto a = parse_assignment();
        if (!a) {
            return std::unexpected(a.error());
        }
        return StatementNode{std::move(*a)};
    }

    // Command / Pipeline / Sequence
    auto list = parse_list();
    if (!list) {
        return std::unexpected(list.error());
    }

    return *list;
}

// -----------------------------------------------------------------------------
// Program: many statements (for full scripts)
// -----------------------------------------------------------------------------

std::expected<std::unique_ptr<ProgramNode>, ParseError> Parser::parse_program() {
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
std::expected<std::unique_ptr<ProgramNode>, ParseError> Parser::parse_line() {
    auto program = std::make_unique<ProgramNode>();

    // Empty input is fine
    if (check(TokenType::EndOfFile) || check(TokenType::Newline)) {
        return program;
    }

    // Parse the single statement allowed in REPL mode
    auto stmt = parse_statement();
    if (!stmt.has_value()) {
        // If the statement itself is incomplete, propagate that upward
        if (stmt.error().kind_ == ParseErrorKind::IncompleteInput) {
            return std::unexpected(stmt.error());
        }
        // Otherwise it's a real syntax error
        return std::unexpected(stmt.error());
    }

    program->add_statement(std::move(*stmt));


    // Allow a single trailing newline or semicolon
    if (check(TokenType::Newline)) {
        (void)lexer_.next_token();
    }
    if (check(TokenType::Semicolon)) {
        (void)lexer_.next_token();
    }

    // If leftover tokens exist, check if they indicate continuation
    if (!check(TokenType::EndOfFile)) {
        Token last = peek_token();

        // Dangling operator at end of line → continuation
        if (last.type == TokenType::Pipe || last.type == TokenType::Redirect) {
            return std::unexpected(ParseError{ParseErrorKind::IncompleteInput,
                                              "Incomplete command at end of line", last.line,
                                              last.column});
        }
        // If a semicolon is present, allow a single trailing semicolon, but check for pipe after
        if (last.type == TokenType::Semicolon) {
            (void)lexer_.next_token();
            if (!check(TokenType::EndOfFile)) {
                Token next = peek_token();
                if (next.type == TokenType::Pipe || next.type == TokenType::Redirect) {
                    // Always treat as IncompleteInput
                    return std::unexpected(ParseError{ParseErrorKind::IncompleteInput,
                                                      "Incomplete command at end of line", next.line,
                                                      next.column});
                }
                // Any other leftover tokens are a syntax error
                return std::unexpected(ParseError{ParseErrorKind::SyntaxError,
                                                  "Unexpected tokens after statement", next.line,
                                                  next.column});
            }
            // If only a trailing semicolon, accept
            return program;
        }

        // Otherwise it's a real syntax error
        return std::unexpected(ParseError{ParseErrorKind::SyntaxError,
                                          "Unexpected tokens after statement", last.line,
                                          last.column});
    }

    return program;
}

}  // namespace wshell