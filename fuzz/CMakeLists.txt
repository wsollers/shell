if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(WARNING "Fuzzing requires Clang compiler, skipping fuzz tests")
    return()
endif()

add_executable(fuzz_command_parser fuzz_command_parser.cpp)
target_link_libraries(fuzz_command_parser PRIVATE wshell_lib)
target_compile_options(fuzz_command_parser PRIVATE -fsanitize=fuzzer,address,undefined)
target_link_options(fuzz_command_parser PRIVATE -fsanitize=fuzzer,address,undefined)

add_executable(fuzz_shell_core fuzz_shell_core.cpp)
target_link_libraries(fuzz_shell_core PRIVATE wshell_lib)
target_compile_options(fuzz_shell_core PRIVATE -fsanitize=fuzzer,address,undefined)
target_link_options(fuzz_shell_core PRIVATE -fsanitize=fuzzer,address,undefined)
