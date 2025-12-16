#include "shell/lexer.hpp"
#include "shell/parser.hpp"
#include <benchmark/benchmark.h>
#include <string>
#include <vector>

using namespace shell;

static void BM_LexParse(benchmark::State& state) {

}
BENCHMARK(BM_LexParse);

BENCHMARK_MAIN();
