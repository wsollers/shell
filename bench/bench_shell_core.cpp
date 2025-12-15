// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../src/lib/shell_core.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <string>

static void BM_ShellCore_ValidateCommand_Valid(benchmark::State& state) {
    const std::string input = "exit 0";
    
    for (auto _ : state) {
        auto result = wshell::ShellCore::validateCommand(input);
        benchmark::DoNotOptimize(result);
    }
}


// Register benchmarks
BENCHMARK(BM_ShellCore_ValidateCommand_Valid);

BENCHMARK_MAIN();