// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace wshell {

// ============================================================================
// AST Node Base
// ============================================================================

/// Base class for all AST nodes
struct ASTNode {
    virtual ~ASTNode() = default;
};

// ============================================================================
// Phase 1 AST Nodes
// ============================================================================

/// Comment node (# comment text)
struct CommentNode : ASTNode {
    std::string text;
    
    explicit CommentNode(std::string t) : text(std::move(t)) {}
};

/// Variable assignment node (let var = value)
struct AssignmentNode : ASTNode {
    std::string variable;
    std::string value;
    
    AssignmentNode(std::string var, std::string val) 
        : variable(std::move(var)), value(std::move(val)) {}
};

/// Simple command node (command arg1 arg2 ...)
struct CommandNode : ASTNode {
    std::string command_name;
    std::vector<std::string> arguments;
    
    CommandNode(std::string name, std::vector<std::string> args = {})
        : command_name(std::move(name)), arguments(std::move(args)) {}
};

// ============================================================================
// Statement - Any top-level construct
// ============================================================================

/// Statement variant - can be Comment, Assignment, or Command
using StatementNode = std::variant<
    std::unique_ptr<CommentNode>,
    std::unique_ptr<AssignmentNode>,
    std::unique_ptr<CommandNode>
>;

// ============================================================================
// Program - Collection of statements
// ============================================================================

/// Program node - root of AST
struct ProgramNode : ASTNode {
    std::vector<StatementNode> statements;
    
    ProgramNode() = default;
    
    void add_statement(StatementNode stmt) {
        statements.push_back(std::move(stmt));
    }
    
    [[nodiscard]] bool empty() const noexcept {
        return statements.empty();
    }
    
    [[nodiscard]] std::size_t size() const noexcept {
        return statements.size();
    }
};

// ============================================================================
// Helper Functions
// ============================================================================

/// Create a comment node
inline std::unique_ptr<CommentNode> make_comment(std::string text) {
    return std::make_unique<CommentNode>(std::move(text));
}

/// Create an assignment node
inline std::unique_ptr<AssignmentNode> make_assignment(std::string var, std::string value) {
    return std::make_unique<AssignmentNode>(std::move(var), std::move(value));
}

/// Create a command node
inline std::unique_ptr<CommandNode> make_command(std::string name, std::vector<std::string> args = {}) {
    return std::make_unique<CommandNode>(std::move(name), std::move(args));
}

} // namespace wshell
