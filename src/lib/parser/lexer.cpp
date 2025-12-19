// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/lexer.hpp"
#include <cctype>

namespace wshell {

char Lexer::current() const noexcept {
    if (is_at_end()) return '\0';
    return source_[position_];
}

char Lexer::peek(std::size_t offset) const noexcept {
    if (position_ + offset >= source_.size()) return '\0';
    return source_[position_ + offset];
}

void Lexer::advance() noexcept {
    if (is_at_end()) return;

    if (current() == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    position_++;
}

void Lexer::skip_whitespace() {
    while (!is_at_end() && (current() == ' ' || current() == '\t')) {
        advance();
    }
}

Token Lexer::make_token(TokenType type, std::string value) {
    return Token(type, std::move(value), line_, column_);
}

Token Lexer::lex_comment() {
    std::size_t start = position_;
    advance(); // Skip '#'

    while (!is_at_end() && current() != '\n') {
        advance();
    }

    std::string comment_text(source_.substr(start + 1, position_ - start - 1));

    // Trim exactly one leading space
    if (!comment_text.empty() && comment_text[0] == ' ') {
        comment_text.erase(0, 1);
    }

    return make_token(TokenType::Comment, comment_text);
}

// -----------------------------------------------------------------------------
// ✅ Unified shell-style WORD rule
// -----------------------------------------------------------------------------

Token Lexer::lex_word() {
    std::size_t start = position_;

    auto is_operator = [&](char ch) {
        return ch == '=' || ch == '#' || ch == '|' || ch == '&' ||
               ch == ';' || ch == '<' || ch == '>';
    };

    while (!is_at_end() &&
           !std::isspace(static_cast<unsigned char>(current())) &&
           !is_operator(current())) {
        advance();
    }

    std::string text(source_.substr(start, position_ - start));

    // Keyword check
    if (text == "let") {
        return make_token(TokenType::Let, text);
    }

    return make_token(TokenType::Identifier, text);
}

// -----------------------------------------------------------------------------
// next_token()
// -----------------------------------------------------------------------------

Token Lexer::next_token() {
    // Return peeked token if available
    if (peeked_token_.has_value()) {
        Token token = *peeked_token_;
        peeked_token_.reset();
        return token;
    }

    skip_whitespace();

    if (is_at_end()) {
        return make_token(TokenType::EndOfFile);
    }

    char c = current();

    // Newline
    if (c == '\n') {
        Token token = make_token(TokenType::Newline, "\n");
        advance();
        return token;
    }

    // Comment
    if (c == '#') {
        return lex_comment();
    }

    // Operators ------------------------------------------------------

    if (c == '|') {
        advance();
        return make_token(TokenType::Pipe, "|");
    }

    if (c == '&') {
        advance();
        return make_token(TokenType::Background, "&");
    }

    if (c == ';') {
        advance();
        return make_token(TokenType::Semicolon, ";");
    }

    if (c == '>') {
        if (peek() == '>') {
            advance();
            advance();
            return make_token(TokenType::Redirect, ">>");
        }
        advance();
        return make_token(TokenType::Redirect, ">");
    }

    if (c == '<') {
        advance();
        return make_token(TokenType::Redirect, "<");
    }

    if (c == '=') {
        Token token = make_token(TokenType::Equals, "=");
        advance();
        return token;
    }

    // ✅ Unified WORD rule (fixes out.txt, 42, $HOME, -R, ./foo, etc.)
    return lex_word();
}

Token Lexer::peek_token() {
    if (!peeked_token_.has_value()) {
        peeked_token_ = next_token();
    }
    return *peeked_token_;
}

std::vector<Token> Lexer::tokenize(std::string_view source) {
    Lexer lexer(source);
    std::vector<Token> tokens;

    while (true) {
        Token token = lexer.next_token();
        tokens.push_back(token);
        if (token.type == TokenType::EndOfFile) {
            break;
        }
    }

    return tokens;
}

} // namespace wshell