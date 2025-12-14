#include "shell/lexer.hpp"
#include <string>

namespace shell {

static inline bool is_space(char c) {
  return c == ' ' || c == '\t' || c == '\r';
}

LexResult Lexer::lex(std::string_view in) const {
  LexResult out;
  out.toks.reserve(in.size() / 2 + 8);

  enum class St : std::uint8_t { Normal, InSq, InDq, EscNormal, EscDq };
  St st = St::Normal;

  std::string cur;
  auto flush_word = [&](std::size_t pos_end) {
    if (!cur.empty()) {
      out.toks.push_back(Token{TokKind::Word, pos_end - cur.size(), std::move(cur), {}});
      cur.clear();
    }
  };

  auto err = [&](std::size_t pos, std::string msg) {
    flush_word(pos);
    out.ok = false;
    out.toks.push_back(Token{TokKind::Error, pos, {}, std::move(msg)});
  };

  const std::size_t n = in.size();
  for (std::size_t i = 0; i < n; ++i) {
    char c = in[i];

    switch (st) {
      case St::Normal: {
        if (c == '\n') {
          flush_word(i);
          out.toks.push_back(Token{TokKind::Eol, i, {}, {}});
          break;
        }
        if (is_space(c)) {
          flush_word(i);
          break;
        }
        // quotes
        if (c == '\'') { st = St::InSq; break; }
        if (c == '"')  { st = St::InDq; break; }
        if (c == '\\') { st = St::EscNormal; break; }

        // operators (max munch)
        if (c == '&') {
          flush_word(i);
          if (i + 1 < n && in[i + 1] == '&') {
            out.toks.push_back(Token{TokKind::AndIf, i, {}, {}});
            ++i;
          } else {
            out.toks.push_back(Token{TokKind::Amp, i, {}, {}});
          }
          break;
        }
        if (c == '|') {
          flush_word(i);
          if (i + 1 < n && in[i + 1] == '|') {
            out.toks.push_back(Token{TokKind::OrIf, i, {}, {}});
            ++i;
          } else {
            out.toks.push_back(Token{TokKind::Pipe, i, {}, {}});
          }
          break;
        }
        if (c == ';') {
          flush_word(i);
          out.toks.push_back(Token{TokKind::Semi, i, {}, {}});
          break;
        }
        if (c == '>') {
          flush_word(i);
          if (i + 1 < n && in[i + 1] == '>') {
            out.toks.push_back(Token{TokKind::ROutApp, i, {}, {}});
            ++i;
          } else {
            out.toks.push_back(Token{TokKind::ROut, i, {}, {}});
          }
          break;
        }
        if (c == '<') {
          flush_word(i);
          if (i + 1 < n && in[i + 1] == '<') {
            out.toks.push_back(Token{TokKind::Heredoc, i, {}, {}});
            ++i;
          } else {
            out.toks.push_back(Token{TokKind::RIn, i, {}, {}});
          }
          break;
        }

        // ordinary byte (UTF-8 safe: treat as opaque)
        cur.push_back(c);
        break;
      }

      case St::InSq: {
        if (c == '\'') { st = St::Normal; }
        else { cur.push_back(c); }
        break;
      }

      case St::InDq: {
        if (c == '"') { st = St::Normal; break; }
        if (c == '\\') { st = St::EscDq; break; }
        cur.push_back(c);
        break;
      }

      case St::EscNormal: {
        cur.push_back(c);
        st = St::Normal;
        break;
      }

      case St::EscDq: {
        cur.push_back(c);
        st = St::InDq;
        break;
      }
    }
  }

  if (st == St::InSq) err(n, "unterminated single quote");
  else if (st == St::InDq) err(n, "unterminated double quote");
  else if (st == St::EscNormal || st == St::EscDq) err(n, "dangling escape");
  else flush_word(n);

  out.toks.push_back(Token{TokKind::End, n, {}, {}});
  return out;
}

} // namespace shell
