// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <vector>

namespace wshell {

// ============================================================================
// Token Types for Phase 1
// ============================================================================

/// Token type enumeration
enum class TokenType {
    Identifier,      // command names, variable names
    Let,             // 'let' keyword
    Equals,          // '='
    Comment,         // '# ...' (rest of line)
    Newline,         // '\n'
    Whitespace,      // ' ', '\t' (usually skipped)
    EndOfFile,       // End of input

    // Variable and substitution tokens
    Dollar,          // $
    Variable,        // $VAR or ${VAR}
    LBrace,          // {
    RBrace,          // }

    //added
    Pipe,         // '|'
    Redirect,     // '>', '<', '>>'
    Semicolon,    // ';'
    Background,   // '&'
};

/// Token with type, value, and location
struct Token {
    TokenType type;
    std::string value;
    std::size_t line{1};
    std::size_t column{1};
    
    Token(TokenType t, std::string v = "", std::size_t ln = 1, std::size_t col = 1)
        : type(t), value(std::move(v)), line(ln), column(col) {}
    
    [[nodiscard]] bool is(TokenType t) const noexcept { return type == t; }
    [[nodiscard]] bool is_not(TokenType t) const noexcept { return type != t; }
};

// ============================================================================
// Lexer - Tokenizes input into tokens
// ============================================================================

/// Lexer for Phase 1 shell language
class Lexer {
public:
    /// Construct lexer with input source
    explicit Lexer(std::string_view source) 
        : source_(source), position_(0), line_(1), column_(1) {}
    
    /// Get next token
    [[nodiscard]] Token next_token();
    
    /// Peek at next token without consuming it
    [[nodiscard]] Token peek_token();
    
    /// Check if at end of input
    [[nodiscard]] bool is_at_end() const noexcept {
        return position_ >= source_.size();
    }
    
    /// Reset lexer to beginning
    void reset() noexcept {
        position_ = 0;
        line_ = 1;
        column_ = 1;
    }
    
    /// Tokenize entire input into vector of tokens
    [[nodiscard]] static std::vector<Token> tokenize(std::string_view source);
    
private:
    std::string_view source_;
    std::size_t position_;
    std::size_t line_;
    std::size_t column_;
    std::optional<Token> peeked_token_;
    
    // Lexer helpers
    [[nodiscard]] char current() const noexcept;
    [[nodiscard]] char peek(std::size_t offset = 1) const noexcept;
    void advance() noexcept;
    void skip_whitespace();
    
    [[nodiscard]] Token lex_comment();
    Token lex_word();
    [[nodiscard]] Token lex_identifier_or_keyword();
    [[nodiscard]] Token make_token(TokenType type, std::string value = "");
};

} // namespace wshell
