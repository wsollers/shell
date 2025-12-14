cmake_minimum_required(VERSION 3.25)

# bench/CMakeLists.txt
# Google Benchmark integration for performance testing

message(STATUS "Benchmarks: enabled (Google Benchmark configured)")

# Benchmark: command parser
add_executable(bench_command_parser bench_command_parser.cpp)
target_link_libraries(bench_command_parser PRIVATE 
    wshell_lib 
    benchmark::benchmark
)
target_compile_features(bench_command_parser PRIVATE cxx_std_23)

# Benchmark: shell core
add_executable(bench_shell_core bench_shell_core.cpp)
target_link_libraries(bench_shell_core PRIVATE 
    wshell_lib 
    benchmark::benchmark
)
target_compile_features(bench_shell_core PRIVATE cxx_std_23)