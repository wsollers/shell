

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(WARNING "Fuzzing requires Clang compiler, skipping fuzz tests")
    return()
endif()

# Fuzz test for command parser
add_executable(fuzz_command_parser fuzz_command_parser.cpp)
target_link_libraries(fuzz_command_parser PRIVATE wshell_lib stdc++)
target_compile_options(fuzz_command_parser PRIVATE 
    -fsanitize=fuzzer,address,undefined
    -stdlib=libstdc++
    -g
)
target_link_options(fuzz_command_parser PRIVATE 
    -fsanitize=fuzzer,address,undefined
    -stdlib=libstdc++
)

# Fuzz test for shell core
add_executable(fuzz_shell_core fuzz_shell_core.cpp)
target_link_libraries(fuzz_shell_core PRIVATE wshell_lib stdc++)
target_compile_options(fuzz_shell_core PRIVATE 
    -fsanitize=fuzzer,address,undefined
    -stdlib=libstdc++
    -g
)
target_link_options(fuzz_shell_core PRIVATE 
    -fsanitize=fuzzer,address,undefined
    -stdlib=libstdc++
)