#include "shell/lexer.hpp"
#include "shell/parser.hpp"
#include <benchmark/benchmark.h>
#include <string>
#include <vector>

using namespace shell;

static void BM_LexParse(benchmark::State& state) {
  Lexer lx;
  Parser p;

  std::vector<std::string> samples = {
    "echo hello world",
    R"(echo "a b" 'c d' e\ f)",
    "cat < in.txt | grep foo >> out.txt",
    "a | b | c && d || e ; f & g && h",
    "cmd1 > out ; cmd2 < in && cmd3 | cmd4 >> log &"
  };

  for (auto _ : state) {
    for (auto const& s : samples) {
      auto lr = lx.lex(s + "\n");
      auto pr = p.parse(lr.toks);
      benchmark::DoNotOptimize(pr.seq.items.size());
      benchmark::DoNotOptimize(pr.arena.nodes.size());
    }
  }
}
BENCHMARK(BM_LexParse);

BENCHMARK_MAIN();
