if(ENABLE_BENCHMARKS)
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

    # Flame graph profiling support
    if(ENABLE_FLAME_GRAPHS)
        message(STATUS "Flame graphs: enabled - adding profiling support")

        # Add profiling flags for flame graph generation
        if(NOT MSVC)
            # Enable frame pointers and debug symbols for accurate profiling
            target_compile_options(bench_command_parser PRIVATE -g -fno-omit-frame-pointer)
            target_compile_options(bench_shell_core PRIVATE -g -fno-omit-frame-pointer)

            # Disable optimization that might interfere with profiling accuracy
            target_compile_options(bench_command_parser PRIVATE -O2 -fno-inline-functions)
            target_compile_options(bench_shell_core PRIVATE -O2 -fno-inline-functions)
        endif()
    endif()
endif()