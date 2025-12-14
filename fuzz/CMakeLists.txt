cmake_minimum_required(VERSION 3.25)

# Default: fuzzing not enabled unless we pass all checks below
set(WSHELL_FUZZ_ENABLED OFF CACHE INTERNAL "Whether fuzz targets are enabled" FORCE)

# fuzz/CMakeLists.txt
# Ubuntu/Clang-only fuzz targets using libFuzzer + libc++.

include(CheckCXXCompilerFlag)

# Only build fuzzers with Clang
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(STATUS "Fuzzing: skipped (requires Clang, detected: ${CMAKE_CXX_COMPILER_ID})")
  return()
endif()

# Require Clang >= 18
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "18.0.0")
  message(STATUS "Fuzzing: skipped (requires Clang >= 18, detected: ${CMAKE_CXX_COMPILER_VERSION})")
  return()
endif()

# We only support fuzzing on Linux (Ubuntu runner)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(STATUS "Fuzzing: skipped (supported only on Linux/Ubuntu, detected: ${CMAKE_SYSTEM_NAME})")
  return()
endif()

# Check that libFuzzer flags are supported.
# IMPORTANT: Make try_compile compile-only to avoid link false-negatives.
set(_old_try_compile_target_type "${CMAKE_TRY_COMPILE_TARGET_TYPE}")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
check_cxx_compiler_flag("-fsanitize=fuzzer" HAS_LIBFUZZER)
unset(CMAKE_TRY_COMPILE_TARGET_TYPE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "${_old_try_compile_target_type}")

if(NOT HAS_LIBFUZZER)
  message(STATUS "Fuzzing: skipped (Clang ${CMAKE_CXX_COMPILER_VERSION} but libFuzzer not available: -fsanitize=fuzzer unsupported)")
  return()
endif()

message(STATUS "Fuzzing: enabled (Clang ${CMAKE_CXX_COMPILER_VERSION}, libc++ enforced for fuzz targets)")

# Common fuzz flags (force libc++ for fuzz targets only)
set(WSHELL_FUZZ_COMPILE_OPTS
  -g
  -fno-omit-frame-pointer
  -fsanitize=fuzzer,address,undefined
  -stdlib=libc++
)

set(WSHELL_FUZZ_LINK_OPTS
  -fsanitize=fuzzer,address,undefined
  -stdlib=libc++
  -lc++abi
)

set(WSHELL_FUZZ_ENABLED ON CACHE INTERNAL "Whether fuzz targets are enabled" FORCE)

# Fuzz: command parser
add_executable(fuzz_command_parser fuzz_command_parser.cpp)
target_link_libraries(fuzz_command_parser PRIVATE wshell_lib)
target_compile_options(fuzz_command_parser PRIVATE ${WSHELL_FUZZ_COMPILE_OPTS})
target_link_options(fuzz_command_parser PRIVATE ${WSHELL_FUZZ_LINK_OPTS})

# Fuzz: shell core
add_executable(fuzz_shell_core fuzz_shell_core.cpp)
target_link_libraries(fuzz_shell_core PRIVATE wshell_lib)
target_compile_options(fuzz_shell_core PRIVATE ${WSHELL_FUZZ_COMPILE_OPTS})
target_link_options(fuzz_shell_core PRIVATE ${WSHELL_FUZZ_LINK_OPTS})
