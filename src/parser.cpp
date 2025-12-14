#include "shell/parser.hpp"
#include <utility>

namespace shell {

struct Cursor {
  std::vector<Token> const* t{};
  std::size_t i{0};

  Token const& peek() const { return (*t)[i]; }
  Token const& next() { return (*t)[i++]; }
  bool match(TokKind k) {
    if (peek().kind == k) { ++i; return true; }
    return false;
  }
};

static inline bool is_list_stop(TokKind k) {
  return k == TokKind::Semi || k == TokKind::Amp || k == TokKind::Eol || k == TokKind::End || k == TokKind::Error;
}

static inline std::pair<int,int> bp(TokKind k) {
  switch (k) {
    case TokKind::Pipe:  return {70, 71}; // left assoc
    case TokKind::AndIf: return {50, 51};
    case TokKind::OrIf:  return {50, 51};
    default: return {-1, -1};
  }
}

static std::optional<ParseError> make_err(Token const& tok, std::string msg) {
  return ParseError{tok.pos, std::move(msg)};
}

static std::optional<ParseError> parse_redir(Cursor& c, Command& cmd) {
  auto k = c.peek().kind;
  RedirKind rk{};
  if (k == TokKind::ROut) rk = RedirKind::OutTrunc;
  else if (k == TokKind::ROutApp) rk = RedirKind::OutAppend;
  else if (k == TokKind::RIn) rk = RedirKind::In;
  else if (k == TokKind::Heredoc) rk = RedirKind::Heredoc;
  else return std::nullopt;

  c.next(); // op
  if (c.peek().kind != TokKind::Word) {
    return make_err(c.peek(), "redirection missing target word");
  }
  Token const& w = c.next();
  cmd.redirs.push_back(Redir{rk, w.text});
  return std::nullopt;
}

static std::optional<ParseError> parse_simple_command(Cursor& c, Arena& a, std::size_t& out_node) {
  Command cmd;
  bool saw_word = false;

  while (true) {
    if (c.peek().kind == TokKind::Word) {
      saw_word = true;
      cmd.argv.push_back(c.next().text);
      continue;
    }

    if (auto e = parse_redir(c, cmd)) return e;

    auto k = c.peek().kind;
    if (k == TokKind::Pipe || k == TokKind::AndIf || k == TokKind::OrIf || is_list_stop(k)) break;

    if (k == TokKind::Error) return make_err(c.peek(), c.peek().error_msg);
    return make_err(c.peek(), "unexpected token in command");
  }

  if (!saw_word) return make_err(c.peek(), "expected command word");
  out_node = a.add(std::move(cmd));
  return std::nullopt;
}

static std::optional<ParseError> parse_expr(Cursor& c, Arena& a, int min_bp, std::size_t& out_node) {
  std::size_t lhs{};
  if (auto e = parse_simple_command(c, a, lhs)) return e;

  while (true) {
    TokKind opk = c.peek().kind;
    if (is_list_stop(opk)) break;

    auto [lbp, rbp_] = bp(opk);
    if (lbp < min_bp) break;

    if (opk != TokKind::Pipe && opk != TokKind::AndIf && opk != TokKind::OrIf) break;
    Token const& op = c.next();

    std::size_t rhs{};
    if (auto e = parse_expr(c, a, rbp_, rhs)) return e;

    if (opk == TokKind::Pipe) {
      Pipeline pip;

      auto& ln = a.at(lhs).v;
      if (auto* lp = std::get_if<Pipeline>(&ln)) pip.cmds = std::move(lp->cmds);
      else if (auto* lc = std::get_if<Command>(&ln)) pip.cmds.push_back(std::move(*lc));
      else return make_err(op, "cannot pipeline a logical expression (grouping not implemented)");

      auto& rn = a.at(rhs).v;
      if (auto* rp = std::get_if<Pipeline>(&rn)) {
        for (auto& ccmd : rp->cmds) pip.cmds.push_back(std::move(ccmd));
      } else if (auto* rc = std::get_if<Command>(&rn)) {
        pip.cmds.push_back(std::move(*rc));
      } else {
        return make_err(op, "cannot pipeline a logical expression (grouping not implemented)");
      }

      lhs = a.add(std::move(pip));
    } else {
      LogicalOp lop = (opk == TokKind::AndIf) ? LogicalOp::AndIf : LogicalOp::OrIf;
      lhs = a.add(Logical{lop, lhs, rhs});
    }
  }

  out_node = lhs;
  return std::nullopt;
}

ParseResult Parser::parse(std::vector<Token> const& toks) const {
  ParseResult r;
  Cursor c{&toks, 0};

  while (true) {
    TokKind k = c.peek().kind;
    if (k == TokKind::End) break;
    if (k == TokKind::Error) { r.err = make_err(c.peek(), c.peek().error_msg); return r; }

    if (k == TokKind::Eol || k == TokKind::Semi) { c.next(); continue; }
    if (k == TokKind::Amp) { r.err = make_err(c.peek(), "background operator requires a command before '&'"); return r; }

    std::size_t expr{};
    if (auto e = parse_expr(c, r.arena, 0, expr)) { r.err = std::move(e); return r; }

    bool bg = false;
    if (c.match(TokKind::Amp)) bg = true;
    r.seq.items.push_back(ListItem{expr, bg});

    if (c.match(TokKind::Semi)) continue;
    if (c.match(TokKind::Eol)) continue;
    if (c.peek().kind == TokKind::End) break;

    r.err = make_err(c.peek(), "expected ';', '&', newline, or end");
    return r;
  }

  return r;
}

} // namespace shell
