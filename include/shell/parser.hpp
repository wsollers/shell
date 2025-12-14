#pragma once
#include "shell/ast.hpp"
#include "shell/lexer.hpp"
#include <optional>
#include <string>

namespace shell {

struct ParseError {
  std::size_t pos{};
  std::string msg;
};

struct ParseResult {
  Arena arena;
  Sequence seq;
  std::optional<ParseError> err;
};

struct Parser {
  ParseResult parse(std::vector<Token> const& toks) const;
};

} // namespace shell
