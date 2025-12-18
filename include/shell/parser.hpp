// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include "lexer.hpp"
#include "ast.hpp"
#include <expected>
#include <memory>
#include <stdexcept>
#include <string>

namespace wshell {

// ============================================================================
// Parser Error
// ============================================================================

/// Parse error with location information
struct ParseError {
    std::string message;
    std::size_t line{0};
    std::size_t column{0};
    
    ParseError(std::string msg, std::size_t ln = 0, std::size_t col = 0)
        : message(std::move(msg)), line(ln), column(col) {}
    
    [[nodiscard]] std::string to_string() const {
        return "Parse error at line " + std::to_string(line) 
               + ", column " + std::to_string(column) 
               + ": " + message;
    }
};

// ============================================================================
// Parser - Phase 1 Implementation
// ============================================================================

/// Parser for Phase 1 shell language
/// Grammar:
///   Program    := Statement*
///   Statement  := Comment | Assignment | Command | Newline
///   Comment    := '#' .*
///   Assignment := 'let' Identifier '=' Value
///   Command    := Identifier Identifier*
class Parser {
public:
    /// Construct parser with input source
    explicit Parser(std::string_view source) : lexer_(source) {}
    
    /// Parse the entire program
    [[nodiscard]] std::expected<std::unique_ptr<ProgramNode>, ParseError> parse_program();

    /// Parse a single line (for REPL mode)
    [[nodiscard]] std::expected<std::unique_ptr<ProgramNode>, ParseError> parse_line();
    
private:
    Lexer lexer_;
    
    // Parser methods
    [[nodiscard]] std::expected<StatementNode, ParseError> parse_statement();
    [[nodiscard]] std::expected<std::unique_ptr<CommentNode>, ParseError> parse_comment();
    [[nodiscard]] std::expected<std::unique_ptr<AssignmentNode>, ParseError> parse_assignment();
    [[nodiscard]] std::expected<std::unique_ptr<CommandNode>, ParseError> parse_command();
    
    // Token helpers
    [[nodiscard]] Token current_token();
    [[nodiscard]] Token peek_token();
    [[nodiscard]] bool check(TokenType type);
    [[nodiscard]] bool match(TokenType type);
    void skip_newlines();
    
    [[nodiscard]] ParseError make_error(const std::string& message);
};




// ============================================================================
// Convenience Functions
// ============================================================================

/// Parse a line of shell code
[[nodiscard]] inline std::expected<std::unique_ptr<ProgramNode>, ParseError> 
parse_line(std::string_view source) {
    Parser parser(source);
    return parser.parse_line();
}

/// Parse a complete program (script)
[[nodiscard]] inline std::expected<std::unique_ptr<ProgramNode>, ParseError> 
parse_program(std::string_view source) {
    Parser parser(source);
    return parser.parse_program();
}

} // namespace wshell
