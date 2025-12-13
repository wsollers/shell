cmake_minimum_required(VERSION 3.25)

project(wshell
    VERSION 0.1.0
    DESCRIPTION "A secure command shell interpreter"
    LANGUAGES CXX
)

# ============================
# Language / standard settings
# ============================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================
# Modules
# ============================
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

# ============================
# Options
# ============================
option(ENABLE_TESTING "Enable testing" ON)
option(ENABLE_FUZZING "Enable fuzz testing" OFF)
option(ENABLE_BENCHMARKS "Enable benchmarks" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers (ASan, UBSan, etc.)" OFF)

# ============================
# Build type
# ============================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ============================
# Security / Hardening flags
# ============================
if(MSVC)
    # ---- Compile hardening ----
    add_compile_options(
        /W4
        /WX
        /permissive-
        /GS
        /sdl
        /utf-8
        /Zc:__cplusplus
        /Zc:preprocessor
    )

    # Control Flow Guard
    add_compile_options(/guard:cf)
    add_link_options(/GUARD:CF)

    # ASLR / DEP / high-entropy VA (x64)
    add_link_options(
        /DYNAMICBASE
        /NXCOMPAT
        /HIGHENTROPYVA
    )

    # EH continuation metadata (feature-tested)
    check_cxx_compiler_flag("/guard:ehcont" HAS_GUARD_EHCONT)
    if(HAS_GUARD_EHCONT)
        add_compile_options(/guard:ehcont)
    endif()

    # CET / Shadow Stack compatibility (feature-tested)
    check_linker_flag(CXX "/CETCOMPAT" HAS_CETCOMPAT)
    if(HAS_CETCOMPAT)
        add_link_options(/CETCOMPAT)
    endif()

    # Release-only optimization + hardening
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /GL)
        add_link_options(
            /LTCG
            /OPT:REF
            /OPT:ICF
            /INCREMENTAL:NO
        )
    endif()

else()
    # ---- GCC / Clang compile hardening ----
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wformat=2
        -Wformat-security
        -Wnull-dereference
        -Wstack-protector
        -Wno-unused-parameter
        -fstack-protector-strong
        -fno-common
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Wtrampolines)
    endif()

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(_FORTIFY_SOURCE=3)
    endif()

    # Linux-only stack clash protection
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-fstack-clash-protection)
    endif()

    # Linux x86_64 CET (feature-tested)
    check_cxx_compiler_flag("-fcf-protection=full" HAS_CF_PROTECTION_FULL)
    if(HAS_CF_PROTECTION_FULL
       AND CMAKE_SYSTEM_NAME STREQUAL "Linux"
       AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        add_compile_options(-fcf-protection=full)
    endif()

    # ---- Linker hardening ----
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_link_options(
            -pie
            -Wl,-z,relro
            -Wl,-z,now
            -Wl,-z,noexecstack
        )
    elseif(APPLE)
        add_link_options(
            -Wl,-dead_strip
            -Wl,-bind_at_load
        )
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# ============================
# Sanitizers (non-MSVC)
# ============================
if(ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=leak
        -fno-omit-frame-pointer
    )
    add_link_options(
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=leak
    )
endif()

# ============================
# Coverage (non-MSVC)
# ============================
# Code coverage (non-MSVC)
if(ENABLE_COVERAGE AND NOT MSVC)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # LLVM source-based coverage (recommended with clang/llvm-profdata/llvm-cov)
        add_compile_options(-O0 -g -fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate)
        message(STATUS "Code coverage enabled: LLVM/Clang source-based coverage")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC/gcov coverage (use gcovr/lcov)
        add_compile_options(-O0 -g --coverage)
        add_link_options(--coverage)
        message(STATUS "Code coverage enabled: GCC/gcov coverage")
    else()
        message(WARNING "ENABLE_COVERAGE is ON, but compiler '${CMAKE_CXX_COMPILER_ID}' is not explicitly supported for coverage flags.")
    endif()
endif()


# ============================
# Dependencies
# ============================
if(ENABLE_TESTING)
    enable_testing()
    find_package(GTest QUIET)

    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
            GIT_SHALLOW TRUE
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

if(ENABLE_BENCHMARKS)
    find_package(benchmark QUIET)
    if(NOT benchmark_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.3
            GIT_SHALLOW TRUE
        )
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googlebenchmark)
    endif()
endif()

# ============================
# Subdirectories
# ============================
add_subdirectory(src/lib)
add_subdirectory(src/main)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

if(ENABLE_FUZZING)
    add_subdirectory(fuzz)
endif()

# ============================
# Installation
# ============================
install(TARGETS wshell wshell_lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# ============================
# Summary
# ============================
message(STATUS "=== Configuration Summary ===")
message(STATUS "  Version:       ${PROJECT_VERSION}")
message(STATUS "  Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:  ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:      ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Testing:       ${ENABLE_TESTING}")
message(STATUS "  Fuzzing:       ${ENABLE_FUZZING}")
message(STATUS "  Benchmarks:    ${ENABLE_BENCHMARKS}")
message(STATUS "  Coverage:      ${ENABLE_COVERAGE}")
message(STATUS "  Sanitizers:    ${ENABLE_SANITIZERS}")
message(STATUS "============================")
