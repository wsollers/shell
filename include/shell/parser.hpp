// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <expected>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "ast.hpp"
#include "ast_printer.hpp"
#include "lexer.hpp"

namespace wshell {

enum class ParseErrorKind {
    SyntaxError,
    IncompleteInput
};

/// Parse error with location information
struct ParseError {
    ParseErrorKind kind_;
    std::string message_;
    std::size_t line_{0};
    std::size_t column_{0};

    ParseError(ParseErrorKind theKind = ParseErrorKind::SyntaxError,
               std::string msg = "Unknown Error.",
               std::size_t ln = 0,
               std::size_t col = 0)
        : kind_{theKind}, message_(std::move(msg)), line_(ln), column_(col) {}

    [[nodiscard]] std::string to_string() const {
        return "Parse error at line " + std::to_string(line_)
             + ", column " + std::to_string(column_)
             + ": " + message_;
    }
};

/// Grammar:
///   Program    := Statement*
///   Statement  := Comment | Assignment | Command | Pipeline | Sequence
///   Comment    := '#' .*
///   Assignment := 'let' Identifier '=' Value
///   Command    := Identifier Identifier*
class Parser {
public:
    /// Construct parser with input source
    explicit Parser(std::string_view source, bool repl_mode = true)
        : lexer_(source), repl_mode_{repl_mode} {}

    /// Parse the entire program
    [[nodiscard]] std::expected<std::unique_ptr<ProgramNode>, ParseError>
    parse_program();

    /// Parse a single line (for REPL mode)
    [[nodiscard]] std::expected<std::unique_ptr<ProgramNode>, ParseError>
    parse_line();

private:
    Lexer lexer_;
    bool repl_mode_;

    // Parser methods (all updated to match the new AST)
    [[nodiscard]] std::expected<StatementNode, ParseError> parse_statement();
    [[nodiscard]] std::expected<CommentNode, ParseError> parse_comment();
    [[nodiscard]] std::expected<AssignmentNode, ParseError> parse_assignment();
    [[nodiscard]] std::expected<Redirection, ParseError> parse_redirection();
    [[nodiscard]] std::expected<CommandNode, ParseError> parse_simple_command();
    [[nodiscard]] std::expected<CommandNode, ParseError> parse_command();
    [[nodiscard]] std::expected<StatementNode, ParseError> parse_pipeline();
    [[nodiscard]] std::expected<StatementNode, ParseError> parse_list();

    // Token helpers
    [[nodiscard]] Token peek_token();
    [[nodiscard]] Token current_token();
    bool check(TokenType type);
    void advance();
    [[nodiscard]] bool match(TokenType t);
    void skip_newlines();

    [[nodiscard]] ParseError make_error(ParseErrorKind kind,
                                        const std::string& message);
};

// ============================================================================
// Convenience Functions
// ============================================================================

/// Parse a line of shell code
[[nodiscard]] inline std::expected<std::unique_ptr<ProgramNode>, ParseError>
parse_line(std::string_view source) {
    Parser parser(source, true);
    auto result = parser.parse_line();
    if (result.has_value()) {
        print_program(*result.value(), std::cout);
    }
    return result;
}

/// Parse a complete program (script)
[[nodiscard]] inline std::expected<std::unique_ptr<ProgramNode>, ParseError>
parse_program(std::string_view source) {
    Parser parser(source, false);
    auto result = parser.parse_program();
    if (result.has_value()) {
        print_program(*result.value(), std::cout);
    }
    return result;
}

} // namespace wshell