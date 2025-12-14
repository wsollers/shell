#include "shell/lexer.hpp"
#include "shell/parser.hpp"
#include <cstddef>
#include <cstdint>
#include <string_view>

using namespace shell;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::string s(reinterpret_cast<const char*>(data), size);
  Lexer lx;
  auto lr = lx.lex(std::string_view{s});
  Parser p;
  auto pr = p.parse(lr.toks);

  if (!pr.err.has_value()) {
    for (auto const& it : pr.seq.items) {
      if (it.node >= pr.arena.nodes.size()) __builtin_trap();
    }
  }
  return 0;
}
