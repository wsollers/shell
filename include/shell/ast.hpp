#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace shell {

enum class RedirKind : std::uint8_t {
  In,         // <
  OutTrunc,   // >
  OutAppend,  // >>
  Heredoc     // <<
};

struct Redir {
  RedirKind kind{};
  std::string target; // already unquoted/unescaped in v0
};

struct Command {
  std::vector<std::string> argv; // argv[0] required
  std::vector<Redir> redirs;
};

struct Pipeline {
  std::vector<Command> cmds; // >= 2 for pipeline; allow 1 if you want
};

enum class LogicalOp : std::uint8_t { AndIf, OrIf };

struct Logical {
  LogicalOp op{};
  std::size_t lhs{};
  std::size_t rhs{};
};

struct Node {
  std::variant<Command, Pipeline, Logical> v;
};

struct ListItem {
  std::size_t node{};
  bool background{};
};

struct Sequence {
  std::vector<ListItem> items;
};

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
