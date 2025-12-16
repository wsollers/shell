// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace shell {

/// @brief Redirection kind for I/O operations
enum class RedirKind : std::uint8_t {
  IN,         ///< Input redirection (<)
  OUT_TRUNC,  ///< Output redirection, truncate (>)
  OUT_APPEND, ///< Output redirection, append (>>)
  HEREDOC     ///< Here document (<<)
};

/// @brief Represents a single redirection
struct Redir {
  RedirKind kind{};
  std::string target;
};

/// @brief Represents a simple command with arguments and redirections
struct Command {
  std::vector<std::string> argv;
  std::vector<Redir> redirs;
};

/// @brief Represents a pipeline of commands
struct Pipeline {
  std::vector<Command> cmds;
};

/// @brief Logical operators for combining commands
enum class LogicalOp : std::uint8_t { AND_IF, OR_IF };

/// @brief Represents a logical operation between two nodes
struct Logical {
  LogicalOp op{};
  std::size_t lhs{};
  std::size_t rhs{};
};

/// @brief Variant node type for AST
struct Node {
  std::variant<Command, Pipeline, Logical> v;
};

/// @brief List item with background flag
struct ListItem {
  std::size_t node{};
  bool background{};
};

/// @brief Sequence of list items
struct Sequence {
  std::vector<ListItem> items;
};

/// @brief Arena allocator for AST nodes
struct Arena {
  std::vector<Node> nodes;

  template<class T>
  std::size_t add(T&& t) {
    nodes.push_back(Node{std::variant<Command, Pipeline, Logical>(std::forward<T>(t))});
    return nodes.size() - 1;
  }

  Node&       at(std::size_t i)       { return nodes.at(i); }
  Node const& at(std::size_t i) const { return nodes.at(i); }
};

} // namespace shell
