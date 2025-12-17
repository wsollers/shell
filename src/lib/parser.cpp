// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/parser.hpp"

namespace wshell {

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
        (void)current_token(); // Consume token
        return true;
    }
    return false;
}

void Parser::skip_newlines() {
    while (match(TokenType::Newline)) {
        // Skip all newlines
    }
}

ParseError Parser::make_error(const std::string& message) {
    Token token = peek_token();
    return ParseError(message, token.line, token.column);
}

std::expected<std::unique_ptr<CommentNode>, ParseError> Parser::parse_comment() {
    Token token = current_token(); // Consume comment token
    if (token.type != TokenType::Comment) {
        return std::unexpected(make_error("Expected comment"));
    }
    return make_comment(token.value);
}

std::expected<std::unique_ptr<AssignmentNode>, ParseError> Parser::parse_assignment() {
    // Consume 'let' keyword
    Token let_token = current_token();
    if (let_token.type != TokenType::Let) {
        return std::unexpected(make_error("Expected 'let' keyword"));
    }
    
    // Get variable name
    Token var_token = current_token();
    if (var_token.type != TokenType::Identifier) {
        return std::unexpected(make_error("Expected variable name after 'let'"));
    }
    std::string variable = var_token.value;
    
    // Expect '='
    Token eq_token = current_token();
    if (eq_token.type != TokenType::Equals) {
        return std::unexpected(make_error("Expected '=' after variable name"));
    }
    
    // Get value (everything until newline or EOF)
    std::string value;
    while (!check(TokenType::Newline) && !check(TokenType::EndOfFile)) {
        Token val_token = current_token();
        if (!value.empty()) {
            value += " ";
        }
        value += val_token.value;
    }
    
    if (value.empty()) {
        return std::unexpected(make_error("Expected value after '='"));
    }
    
    return make_assignment(variable, value);
}

std::expected<std::unique_ptr<CommandNode>, ParseError> Parser::parse_command() {
    // Get command name
    Token cmd_token = current_token();
    if (cmd_token.type != TokenType::Identifier) {
        return std::unexpected(make_error("Expected command name"));
    }
    std::string command_name = cmd_token.value;
    
    // Get arguments (everything until newline or EOF)
    std::vector<std::string> arguments;
    while (!check(TokenType::Newline) && !check(TokenType::EndOfFile)) {
        Token arg_token = current_token();
        if (arg_token.type == TokenType::Identifier || arg_token.type == TokenType::Equals) {
            arguments.push_back(arg_token.value);
        } else {
            break;
        }
    }
    
    return make_command(command_name, arguments);
}

std::expected<StatementNode, ParseError> Parser::parse_statement() {
    // Skip leading newlines
    skip_newlines();
    
    // Check for EOF
    if (check(TokenType::EndOfFile)) {
        return std::unexpected(make_error("Unexpected end of file"));
    }
    
    // Check for comment
    if (check(TokenType::Comment)) {
        auto comment = parse_comment();
        if (!comment) {
            return std::unexpected(comment.error());
        }
        return StatementNode(std::move(*comment));
    }
    
    // Check for assignment
    if (check(TokenType::Let)) {
        auto assignment = parse_assignment();
        if (!assignment) {
            return std::unexpected(assignment.error());
        }
        return StatementNode(std::move(*assignment));
    }
    
    // Otherwise, parse as command
    if (check(TokenType::Identifier)) {
        auto command = parse_command();
        if (!command) {
            return std::unexpected(command.error());
        }
        return StatementNode(std::move(*command));
    }
    
    return std::unexpected(make_error("Unexpected token"));
}

std::expected<std::unique_ptr<ProgramNode>, ParseError> Parser::parse_program() {
    auto program = std::make_unique<ProgramNode>();
    
    skip_newlines();
    
    while (!check(TokenType::EndOfFile)) {
        auto statement = parse_statement();
        if (!statement) {
            return std::unexpected(statement.error());
        }
        
        program->add_statement(std::move(*statement));
        
        // Consume trailing newlines
        skip_newlines();
    }
    
    return program;
}

std::expected<std::unique_ptr<ProgramNode>, ParseError> Parser::parse_line() {
    // Parse a single line (for REPL mode)
    auto program = std::make_unique<ProgramNode>();
    
    skip_newlines();
    
    if (check(TokenType::EndOfFile)) {
        // Empty line
        return program;
    }
    
    auto statement = parse_statement();
    if (!statement) {
        return std::unexpected(statement.error());
    }
    
    program->add_statement(std::move(*statement));
    
    return program;
}

} // namespace wshell
