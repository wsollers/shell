cmake_minimum_required(VERSION 3.25)

project(wshell 
    VERSION 0.1.0
    DESCRIPTION "A secure command shell interpreter"
    LANGUAGES CXX
)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tools like clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include useful CMake modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

# Options
option(ENABLE_TESTING "Enable testing" ON)
option(ENABLE_FUZZING "Enable fuzz testing" OFF)
option(ENABLE_BENCHMARKS "Enable benchmarks" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers (ASan, UBSan, etc.)" OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Security flags for MSVC
if(MSVC)
    add_compile_options(
        /W4                 # Warning level 4
        /WX                 # Treat warnings as errors
        /permissive-        # Standards conformance
        /GS                 # Buffer security check
        /guard:cf           # Control Flow Guard
        /sdl                # SDL checks
    )
    add_link_options(
        /DYNAMICBASE        # ASLR
        /NXCOMPAT           # DEP
        /GUARD:CF           # Control Flow Guard
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /GL)
        add_link_options(/LTCG /OPT:REF /OPT:ICF)
    endif()
    
# Security flags for GCC/Clang
else()
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
    )
    
    # GCC-specific warnings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Wtrampolines)
    endif()
    
    # Fortify source (requires optimization)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(_FORTIFY_SOURCE=3)
    endif()
    
    add_compile_options(
        -fstack-protector-strong
        -fno-common
    )

    # -fstack-clash-protection is not supported on Apple Clang (macOS)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-fstack-clash-protection)
    endif()

    include(CheckCXXCompilerFlag)

    # Control-Flow Protection (Intel CET) — only where supported
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-fcf-protection=full" HAS_CF_PROTECTION_FULL)

    if (HAS_CF_PROTECTION_FULL AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        add_compile_options(-fcf-protection=full)
    endif()
    
    add_link_options(
        -pie
        -Wl,-z,relro
        -Wl,-z,now
        -Wl,-z,noexecstack
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
        # Enable LTO for release builds
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# Sanitizers
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
    message(STATUS "Sanitizers enabled: ASan, UBSan, LSan")
endif()

# Code coverage
if(ENABLE_COVERAGE AND NOT MSVC)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
    message(STATUS "Code coverage enabled")
endif()

# Find dependencies
if(ENABLE_TESTING)
    enable_testing()
    
    # Try to find GTest, if not found, fetch it
    find_package(GTest QUIET)
    
    if(NOT GTest_FOUND)
        message(STATUS "GTest not found, fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
            GIT_SHALLOW TRUE
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        message(STATUS "Google Test will be built from source")
    else()
        message(STATUS "Using system Google Test")
    endif()
endif()

if(ENABLE_BENCHMARKS)
    find_package(benchmark QUIET)
    
    if(NOT benchmark_FOUND)
        message(STATUS "Google Benchmark not found, fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.3
            GIT_SHALLOW TRUE
        )
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googlebenchmark)
        
        # Disable strict warnings for Google Benchmark targets
        if(TARGET benchmark)
            if(NOT MSVC)
                target_compile_options(benchmark PRIVATE -Wno-error -Wno-format-nonliteral)
            endif()
        endif()
        
        message(STATUS "Google Benchmark will be built from source")
    else()
        message(STATUS "Using system Google Benchmark")
    endif()
endif()

# Add subdirectories
add_subdirectory(src/lib)
add_subdirectory(src/main)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

if(ENABLE_FUZZING)
    add_subdirectory(fuzz)
endif()

# Installation
install(TARGETS wshell wshell_lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Print configuration summary
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