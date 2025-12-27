// Copyright (c) 2024
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <string_view>
#include <variant>

#include <string>
#include <vector>
#include <ostream>

namespace wshell {

// ============================================================================
// Word struct for shell words (quoted, needs expansion, etc.)
// ============================================================================

struct Word {
    std::string text;
    bool quoted = false;
    bool needs_expansion = false;
    Word(std::string t, bool q = false, bool n = false)
        : text(std::move(t)), quoted(q), needs_expansion(n) {}
    Word() = default;
};

// Stream output for Word (for AST printing)
inline std::ostream& operator<<(std::ostream& os, const Word& w) {
    if (w.quoted) {
        os.put('"');
        os << w.text;
        os.put('"');
    } else {
        os << w.text;
    }
    return os;
}

enum class RedirectKind {
    Input,           // <
    OutputTruncate,  // >
    OutputAppend     // >>
};

struct Redirection {
    RedirectKind kind;
    Word target;

    Redirection(RedirectKind k, Word t) : kind(k), target(std::move(t)) {}
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
    Word command_name;
    std::vector<Word> arguments;
    std::vector<Redirection> redirections;
    bool background = false;
};

struct PipelineNode {
    std::vector<CommandNode> commands;  // by value
};

struct SequenceNode {
    std::vector<std::variant<CommentNode, AssignmentNode, CommandNode, PipelineNode, SequenceNode>>
        statements;
};

// ============================================================================
// Statement Variant (NO pointers)
// ============================================================================

using StatementNode =
    std::variant<CommentNode, AssignmentNode, CommandNode, PipelineNode, SequenceNode>;

// ============================================================================
// Program Node
// ============================================================================

struct ProgramNode {
    std::vector<StatementNode> statements;

    void add_statement(StatementNode stmt) { statements.push_back(std::move(stmt)); }

    [[nodiscard]] bool empty() const noexcept { return statements.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return statements.size(); }
};

// ============================================================================
// Factory Helpers (now return value types)
// ============================================================================

inline CommentNode make_comment(std::string text) {
    return CommentNode{std::move(text)};
}

inline AssignmentNode make_assignment(std::string var, std::string value) {
    return AssignmentNode{std::move(var), std::move(value)};
}

inline CommandNode make_command(Word name, std::vector<Word> args = {},
                                std::vector<Redirection> redirs = {}, bool background = false) {
    return CommandNode{std::move(name), std::move(args), std::move(redirs), background};
}

inline PipelineNode make_pipeline(std::vector<CommandNode> cmds) {
    return PipelineNode{std::move(cmds)};
}

inline SequenceNode make_sequence(std::vector<StatementNode> stmts) {
    return SequenceNode{std::move(stmts)};
}

}  // namespace wshell