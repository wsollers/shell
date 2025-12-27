#include "shell/lexer.hpp"
#include "shell/parser.hpp"

#include <string>
#include <vector>

#include <benchmark/benchmark.h>

using namespace wshell;

static void BM_LexParse(benchmark::State& state) {}
BENCHMARK(BM_LexParse);

BENCHMARK_MAIN();
