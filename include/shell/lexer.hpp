#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace shell {

enum class TokKind : std::uint8_t {
  Word,
  Pipe,       // |
  AndIf,      // &&
  OrIf,       // ||
  Semi,       // ;
  Amp,        // &
  ROut,       // >
  ROutApp,    // >>
  RIn,        // <
  Heredoc,    // <<
  Eol,
  End,
  Error
};

struct Token {
  TokKind kind{};
  std::size_t pos{};         // byte offset
  std::string text{};        // for Word only (already unescaped/unquoted in v0)
  std::string error_msg{};   // for Error
};

struct LexResult {
  std::vector<Token> toks;
  bool ok{true};
};

struct Lexer {
  using char_t = char; // your future char_t can replace this

  LexResult lex(std::string_view input) const;
};

} // namespace shell
