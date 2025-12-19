// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace wshell {

// ============================================================================
// Base AST Node
// ============================================================================

struct ASTNode {
    virtual ~ASTNode() = default;
};

// ============================================================================
// Redirection
// ============================================================================

enum class RedirectKind {
    Input,      // <
    Output,     // >
    Append      // >>
};

struct Redirection {
    RedirectKind kind;
    std::string target;   // filename

    Redirection(RedirectKind k, std::string t)
        : kind(k), target(std::move(t)) {}
};

// ============================================================================
// Command Node: simple command + IO + background
// ============================================================================

struct CommandNode : ASTNode {
    std::string command_name;
    std::vector<std::string> arguments;
    std::vector<Redirection> redirections;
    bool background = false;

    CommandNode(std::string name,
                std::vector<std::string> args = {},
                std::vector<Redirection> redirs = {},
                bool bg = false)
        : command_name(std::move(name)),
          arguments(std::move(args)),
          redirections(std::move(redirs)),
          background(bg) {}
};

// ============================================================================
// Pipeline: cmd1 | cmd2 | cmd3
// ============================================================================

struct PipelineNode : ASTNode {
    std::vector<std::unique_ptr<CommandNode>> commands;

    explicit PipelineNode(std::vector<std::unique_ptr<CommandNode>> cmds)
        : commands(std::move(cmds)) {}
};

// ============================================================================
// Sequence: A ; B ; C
// ============================================================================

struct SequenceNode : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;

    explicit SequenceNode(std::vector<std::unique_ptr<ASTNode>> stmts)
        : statements(std::move(stmts)) {}
};

// ============================================================================
// Assignment and Comment
// ============================================================================

struct AssignmentNode : ASTNode {
    std::string variable;
    std::string value;

    AssignmentNode(std::string var, std::string val)
        : variable(std::move(var)), value(std::move(val)) {}
};

struct CommentNode : ASTNode {
    std::string text;

    explicit CommentNode(std::string t)
        : text(std::move(t)) {}
};

// ============================================================================
// Statement Variant
// ============================================================================
//
// A "top-level statement" in a Program can be:
//  - a comment
//  - an assignment
//  - a single command
//  - a pipeline of commands
//  - a sequence of statements
//

using StatementNode = std::variant<
    std::unique_ptr<CommentNode>,
    std::unique_ptr<AssignmentNode>,
    std::unique_ptr<CommandNode>,
    std::unique_ptr<PipelineNode>,
    std::unique_ptr<SequenceNode>
>;

// ============================================================================
// Program Node
// ============================================================================

struct ProgramNode : ASTNode {
    std::vector<StatementNode> statements;

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
// Factory Helpers
// ============================================================================

inline std::unique_ptr<CommentNode> make_comment(std::string text) {
    return std::make_unique<CommentNode>(std::move(text));
}

inline std::unique_ptr<AssignmentNode> make_assignment(
    std::string var,
    std::string value)
{
    return std::make_unique<AssignmentNode>(
        std::move(var),
        std::move(value));
}

inline std::unique_ptr<CommandNode> make_command(
    std::string name,
    std::vector<std::string> args = {},
    std::vector<Redirection> redirs = {},
    bool background = false)
{
    return std::make_unique<CommandNode>(
        std::move(name),
        std::move(args),
        std::move(redirs),
        background);
}

inline std::unique_ptr<PipelineNode> make_pipeline(
    std::vector<std::unique_ptr<CommandNode>> cmds)
{
    return std::make_unique<PipelineNode>(std::move(cmds));
}

inline std::unique_ptr<SequenceNode> make_sequence(
    std::vector<std::unique_ptr<ASTNode>> stmts)
{
    return std::make_unique<SequenceNode>(std::move(stmts));
}

} // namespace wshell