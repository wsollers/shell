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

static void BM_ShellCore_ValidateCommand_Invalid(benchmark::State& state) {
    const std::string input = "invalid_command_that_does_not_exist";
    
    for (auto _ : state) {
        auto result = wshell::ShellCore::validateCommand(input);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_ShellCore_ValidateCommand_Empty(benchmark::State& state) {
    const std::string input = "";
    
    for (auto _ : state) {
        auto result = wshell::ShellCore::validateCommand(input);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_ShellCore_ValidateCommand_Complex(benchmark::State& state) {
    const std::string input = "ls -la /usr/bin | grep python | head -5";
    
    for (auto _ : state) {
        auto result = wshell::ShellCore::validateCommand(input);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_ShellCore_Execute_Exit(benchmark::State& state) {
    const std::string input = "exit 42";
    wshell::ShellCore shell;
    
    for (auto _ : state) {
        auto result = shell.execute(input);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_ShellCore_Execute_Help(benchmark::State& state) {
    const std::string input = "help";
    wshell::ShellCore shell;
    
    for (auto _ : state) {
        auto result = shell.execute(input);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_ShellCore_Execute_Version(benchmark::State& state) {
    const std::string input = "version";
    wshell::ShellCore shell;
    
    for (auto _ : state) {
        auto result = shell.execute(input);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_ShellCore_Execute_LargeInput(benchmark::State& state) {
    std::string large_input = "echo";
    for (int i = 0; i < state.range(0); ++i) {
        large_input += " word" + std::to_string(i);
    }
    wshell::ShellCore shell;
    
    for (auto _ : state) {
        auto result = shell.execute(large_input);
        benchmark::DoNotOptimize(result);
    }
}

// Register benchmarks
BENCHMARK(BM_ShellCore_ValidateCommand_Valid);
BENCHMARK(BM_ShellCore_ValidateCommand_Invalid);
BENCHMARK(BM_ShellCore_ValidateCommand_Empty);
BENCHMARK(BM_ShellCore_ValidateCommand_Complex);
BENCHMARK(BM_ShellCore_Execute_Exit);
BENCHMARK(BM_ShellCore_Execute_Help);
BENCHMARK(BM_ShellCore_Execute_Version);
BENCHMARK(BM_ShellCore_Execute_LargeInput)->Range(8, 8<<10);

BENCHMARK_MAIN();