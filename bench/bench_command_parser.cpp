// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../src/lib/command_parser.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <string>

static void BM_CommandParser_Tokenize_Simple(benchmark::State& state) {
    const std::string input = "ls -la /home/user";
    
    for (auto _ : state) {
        //auto tokens = wshell::CommandParser::tokenize(input);
        //benchmark::DoNotOptimize(tokens);
    }
}


// Register benchmarks
BENCHMARK(BM_CommandParser_Tokenize_Simple);

BENCHMARK_MAIN();