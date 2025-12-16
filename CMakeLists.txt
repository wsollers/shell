cmake_minimum_required(VERSION 3.20)

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
option(WSHELL_FORCE_LIBCXX "Force Clang libc++/libc++abi on non-MSVC builds" OFF)

if(NOT MSVC AND WSHELL_FORCE_LIBCXX AND NOT ENABLE_FUZZING)
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++ -lc++abi)
endif()

# Note: Fuzzing requires compatible standard library with libFuzzer runtime
# Comment out the libc++ forcing for fuzzing to avoid linking conflicts
# if(ENABLE_FUZZING AND NOT MSVC)
#   set(WSHELL_FORCE_LIBCXX ON CACHE BOOL "" FORCE)
# endif()

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
    add_compile_options($<$<CONFIG:Release>:/O2>)
    add_compile_options($<$<CONFIG:Release>:/GL>)
    add_link_options(
        $<$<CONFIG:Release>:/LTCG>
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
        $<$<CONFIG:Release>:/INCREMENTAL:NO>
    )
    
    # Debug-only runtime checks
    add_compile_options($<$<CONFIG:Debug>:/RTC1>)

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
# New ChatGPT files library
# ============================
add_library(shell_core
  src/main/parser/lexer.cpp
  src/main/parser/parser.cpp
  src/main/parser/config.cpp
  src/main/exec/exec_common.cpp
  src/main/exec/exec_linux.cpp
  src/main/exec/exec_macos.cpp
  src/main/exec/exec_win32.cpp
)
target_include_directories(shell_core PUBLIC include)

# Apply security/hardening flags to the new library
target_compile_options(shell_core PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

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
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
    
    # Add new ChatGPT test files
    add_executable(shell_tests test/test_parser.cpp)
    target_link_libraries(shell_tests PRIVATE shell_core gtest_main)
    include(GoogleTest)
    gtest_discover_tests(shell_tests)
endif()

if(ENABLE_BENCHMARKS)
    # Always build Google Benchmark from source for libc++ compatibility
    include(FetchContent)
    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
        GIT_SHALLOW TRUE
    )
    
    # Configure Google Benchmark options
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_LTO OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(googlebenchmark)
    
    # Apply libc++ to benchmark target post-build, and remove strict warning flags
    if(TARGET benchmark AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Apply libc++ 
        target_compile_options(benchmark PRIVATE -stdlib=libc++)
        target_link_options(benchmark INTERFACE -stdlib=libc++)
        
        # Remove strict warning flags from benchmark target
        get_target_property(BENCHMARK_COMPILE_OPTIONS benchmark COMPILE_OPTIONS)
        if(BENCHMARK_COMPILE_OPTIONS)
            # Remove problematic warning flags that cause build failures
            list(REMOVE_ITEM BENCHMARK_COMPILE_OPTIONS "-Werror" "-Wformat-nonliteral" "-pedantic-errors")
            set_target_properties(benchmark PROPERTIES COMPILE_OPTIONS "${BENCHMARK_COMPILE_OPTIONS}")
        endif()
        
        # Add specific flags to suppress problematic warnings
        target_compile_options(benchmark PRIVATE -Wno-format-nonliteral -Wno-error)
    endif()
    
    # Add new ChatGPT benchmark files
    add_executable(shell_bench bench/bench_parser.cpp)
    target_link_libraries(shell_bench PRIVATE shell_core benchmark::benchmark)
endif()

# ============================
# Subdirectories (original enterprise structure)
# ============================
add_subdirectory(src/lib)
add_subdirectory(src/main)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

if(ENABLE_BENCHMARKS)
    add_subdirectory(bench)
endif()

# Track whether fuzz targets were actually enabled/built by fuzz/CMakeLists.txt
set(WSHELL_FUZZ_ENABLED OFF CACHE INTERNAL "Whether fuzz targets are enabled")

if(ENABLE_FUZZING)
    message(STATUS "Fuzzing: requested (ENABLE_FUZZING=ON) - configuring fuzz targets...")
    add_subdirectory(fuzz)
    # Also add new ChatGPT fuzz files directly
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_executable(fuzz_parse fuzz/fuzz_parse.cpp)
        target_link_libraries(fuzz_parse PRIVATE shell_core)
        target_compile_options(fuzz_parse PRIVATE
            -g
            -fno-omit-frame-pointer
            -stdlib=libc++
            -fsanitize=address,undefined
        )
        target_link_options(fuzz_parse PRIVATE
            -stdlib=libstdc++
            -lc++
            -fsanitize=fuzzer,address,undefined
        )
        set(WSHELL_FUZZ_ENABLED ON CACHE INTERNAL "Whether fuzz targets are enabled")
    endif()
endif()

# ============================
# Installation
# ============================
install(TARGETS wshell wshell_lib shell_core
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
message(STATUS "  Fuzzing (requested): ${ENABLE_FUZZING}")
message(STATUS "  Fuzzing (effective): ${WSHELL_FUZZ_ENABLED}")
message(STATUS "  Benchmarks:    ${ENABLE_BENCHMARKS}")
message(STATUS "  Coverage:      ${ENABLE_COVERAGE}")
message(STATUS "  Sanitizers:    ${ENABLE_SANITIZERS}")
message(STATUS "============================")
# ============================
# SBOM Generation (Software Bill of Materials)
# ============================
option(ENABLE_SBOM "Enable SBOM generation" OFF)

if(ENABLE_SBOM)
    include(FetchContent)
    FetchContent_Declare(
        cmake-sbom
        GIT_REPOSITORY https://github.com/DEMCON/cmake-sbom.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(cmake-sbom)
    
    # Add cmake-sbom to module path
    list(APPEND CMAKE_MODULE_PATH "${cmake-sbom_SOURCE_DIR}/cmake")
    
    # Generate SBOM in SPDX tag-value format
    include(sbom)
    sbom_generate(
        OUTPUT "${CMAKE_INSTALL_PREFIX}/wshell-sbom.spdx"
        SUPPLIER "wsollers"
        SUPPLIER_URL "https://github.com/wsollers/shell"
    )
    
    # Add targets to SBOM
    sbom_add(TARGET wshell)
    sbom_add(TARGET wshell_lib)
    sbom_add(TARGET shell_core)
    
    # Finalize SBOM (generates tag-value format, disable verification to avoid issues)
    sbom_finalize(NO_VERIFY)
    
    # Convert SPDX tag-value to JSON using pyspdxtools from venv
    set(VENV_PYTHON "${CMAKE_SOURCE_DIR}/.venv/bin/python3")
    install(CODE "
        message(STATUS \"Converting SBOM to JSON format...\")
        # Remove invalid ExternalRef lines (cmake-sbom bug with empty CPE)
        execute_process(
            COMMAND grep -v \"^ExternalRef: SECURITY cpe23Type \$\" \${CMAKE_INSTALL_PREFIX}/wshell-sbom.spdx
            OUTPUT_FILE \${CMAKE_INSTALL_PREFIX}/wshell-sbom-fixed.spdx
            RESULT_VARIABLE _grep_res
        )
        execute_process(
            COMMAND ${VENV_PYTHON} -m spdx_tools.spdx.clitools.pyspdxtools
                --infile \${CMAKE_INSTALL_PREFIX}/wshell-sbom-fixed.spdx
                --outfile \${CMAKE_INSTALL_PREFIX}/wshell-sbom.spdx.json
                --novalidation
            RESULT_VARIABLE _res
            OUTPUT_VARIABLE _out
            ERROR_VARIABLE _err
        )
        file(REMOVE \${CMAKE_INSTALL_PREFIX}/wshell-sbom-fixed.spdx)
        if(NOT \"\${_res}\" STREQUAL \"0\")
            message(WARNING \"Failed to convert SBOM to JSON: \${_err}\")
        else()
            message(STATUS \"SBOM JSON generated: \${CMAKE_INSTALL_PREFIX}/wshell-sbom.spdx.json\")
        endif()
    ")
    
    message(STATUS "SBOM generation enabled: ${CMAKE_INSTALL_PREFIX}/wshell-sbom.spdx (tag-value) and wshell-sbom.spdx.json (JSON)")
endif()