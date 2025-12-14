// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../src/lib/command_parser.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <string>

static void BM_CommandParser_Tokenize_Simple(benchmark::State& state) {
    const std::string input = "ls -la /home/user";
    
    for (auto _ : state) {
        auto tokens = wshell::CommandParser::tokenize(input);
        benchmark::DoNotOptimize(tokens);
    }
}

static void BM_CommandParser_Tokenize_Complex(benchmark::State& state) {
    const std::string input = R"(grep -r "hello world" /path/to/files --include="*.txt" | sort | uniq)";
    
    for (auto _ : state) {
        auto tokens = wshell::CommandParser::tokenize(input);
        benchmark::DoNotOptimize(tokens);
    }
}

static void BM_CommandParser_Tokenize_Quoted(benchmark::State& state) {
    const std::string input = R"(echo "This is a quoted string with spaces" 'and single quotes' normal_word)";
    
    for (auto _ : state) {
        auto tokens = wshell::CommandParser::tokenize(input);
        benchmark::DoNotOptimize(tokens);
    }
}

static void BM_CommandParser_Tokenize_LargeInput(benchmark::State& state) {
    std::string large_input = "command";
    for (int i = 0; i < state.range(0); ++i) {
        large_input += " arg" + std::to_string(i);
    }
    
    for (auto _ : state) {
        auto tokens = wshell::CommandParser::tokenize(large_input);
        benchmark::DoNotOptimize(tokens);
    }
}

static void BM_CommandParser_Trim_NoWhitespace(benchmark::State& state) {
    const std::string input = "no_whitespace_here";
    
    for (auto _ : state) {
        auto result = wshell::CommandParser::trim(input);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_CommandParser_Trim_WithWhitespace(benchmark::State& state) {
    const std::string input = "   \t  some text with whitespace  \n  ";
    
    for (auto _ : state) {
        auto result = wshell::CommandParser::trim(input);
        benchmark::DoNotOptimize(result);
    }
}

// Register benchmarks
BENCHMARK(BM_CommandParser_Tokenize_Simple);
BENCHMARK(BM_CommandParser_Tokenize_Complex);
BENCHMARK(BM_CommandParser_Tokenize_Quoted);
BENCHMARK(BM_CommandParser_Tokenize_LargeInput)->Range(8, 8<<10);
BENCHMARK(BM_CommandParser_Trim_NoWhitespace);
BENCHMARK(BM_CommandParser_Trim_WithWhitespace);

BENCHMARK_MAIN();