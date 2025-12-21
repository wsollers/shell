// Copyright (c) 2024
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace wshell {

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
    std::string target;

    Redirection(RedirectKind k, std::string t)
        : kind(k), target(std::move(t)) {}
};

// ============================================================================
// Forward declarations for variant
// ============================================================================

struct CommentNode;
struct AssignmentNode;
struct CommandNode;
struct PipelineNode;
struct SequenceNode;

// ============================================================================
// AST Node Types (NO inheritance, NO base class)
// ============================================================================

struct CommentNode {
    std::string text;
};

struct AssignmentNode {
    std::string variable;
    std::string value;
};

struct CommandNode {
    std::string command_name;
    std::vector<std::string> arguments;
    std::vector<Redirection> redirections;
    bool background = false;
};

struct PipelineNode {
    std::vector<CommandNode> commands;   // by value
};

struct SequenceNode {
    std::vector<std::variant<
        CommentNode,
        AssignmentNode,
        CommandNode,
        PipelineNode,
        SequenceNode
    >> statements;
};

// ============================================================================
// Statement Variant (NO pointers)
// ============================================================================

using StatementNode = std::variant<
    CommentNode,
    AssignmentNode,
    CommandNode,
    PipelineNode,
    SequenceNode
>;

// ============================================================================
// Program Node
// ============================================================================

struct ProgramNode {
    std::vector<StatementNode> statements;

    void add_statement(StatementNode stmt) {
        statements.push_back(std::move(stmt));
    }

    [[nodiscard]] bool empty() const noexcept { return statements.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return statements.size(); }
};

// ============================================================================
// Factory Helpers (now return value types)
// ============================================================================

inline CommentNode make_comment(std::string text) {
    return CommentNode{ std::move(text) };
}

inline AssignmentNode make_assignment(std::string var, std::string value) {
    return AssignmentNode{ std::move(var), std::move(value) };
}

inline CommandNode make_command(
    std::string name,
    std::vector<std::string> args = {},
    std::vector<Redirection> redirs = {},
    bool background = false)
{
    return CommandNode{
        std::move(name),
        std::move(args),
        std::move(redirs),
        background
    };
}

inline PipelineNode make_pipeline(std::vector<CommandNode> cmds) {
    return PipelineNode{ std::move(cmds) };
}

inline SequenceNode make_sequence(std::vector<StatementNode> stmts) {
    return SequenceNode{ std::move(stmts) };
}

} // namespace wshell