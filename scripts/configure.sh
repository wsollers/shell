#!/bin/bash
# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

BUILD_TYPE="Release"
BUILD_DIR="build"

ENABLE_TESTING=ON
ENABLE_FUZZING=OFF
ENABLE_BENCHMARKS=OFF
ENABLE_COVERAGE=OFF
ENABLE_SANITIZERS=OFF

# Unix toolchain defaults (your repo prefers clang + libc++ on Unix)
C_COMPILER="clang"
CXX_COMPILER="clang++"

usage() {
  cat <<'EOF'
Usage: ./scripts/configure.sh [OPTIONS]

Build presets:
  --debug          Configure for Debug build
  --release        Configure for Release build (default)
  --test           Configure for testing (RelWithDebInfo + sanitizers)
  --coverage       Configure for coverage (Debug + coverage)

Feature toggles:
  --fuzz           Enable fuzz testing
  --benchmark      Enable benchmarks

Directories:
  --build-dir DIR  Build directory (default: build)

Toolchain overrides:
  --cc  CC         Set C compiler (default: clang)
  --cxx CXX        Set C++ compiler (default: clang++)

EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --debug)     BUILD_TYPE="Debug"; shift ;;
    --release)   BUILD_TYPE="Release"; shift ;;
    --test)      BUILD_TYPE="RelWithDebInfo"; ENABLE_SANITIZERS=ON; shift ;;
    --coverage)  BUILD_TYPE="Debug"; ENABLE_COVERAGE=ON; shift ;;
    --fuzz)      ENABLE_FUZZING=ON; shift ;;
    --benchmark) ENABLE_BENCHMARKS=ON; shift ;;
    --build-dir) BUILD_DIR="${2:?Missing value for --build-dir}"; shift 2 ;;
    --cc)        C_COMPILER="${2:?Missing value for --cc}"; shift 2 ;;
    --cxx)       CXX_COMPILER="${2:?Missing value for --cxx}"; shift 2 ;;
    --help|-h)   usage; exit 0 ;;
    *)
      echo -e "${RED}Unknown option: $1${NC}"
      usage
      exit 1
      ;;
  esac
done

echo -e "${GREEN}Configuring wshell...${NC}"
echo "Build type:   $BUILD_TYPE"
echo "Build dir:    $BUILD_DIR"
echo "CC:           $C_COMPILER"
echo "CXX:          $CXX_COMPILER"
echo "Testing:      $ENABLE_TESTING"
echo "Fuzzing:      $ENABLE_FUZZING"
echo "Benchmarks:   $ENABLE_BENCHMARKS"
echo "Coverage:     $ENABLE_COVERAGE"
echo "Sanitizers:   $ENABLE_SANITIZERS"

mkdir -p "$BUILD_DIR"

# Best practice: feature switches as -D options; toolchain explicit.
# Your project standardizes on clang + libc++ on Unix; keep that here.
cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_C_COMPILER="$C_COMPILER" \
  -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
  -DCMAKE_CXX_FLAGS="-stdlib=libc++ -fPIC" \
  -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -lc++abi" \
  -DCMAKE_SHARED_LINKER_FLAGS="-stdlib=libc++ -lc++abi" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DENABLE_TESTING="$ENABLE_TESTING" \
  -DENABLE_FUZZING="$ENABLE_FUZZING" \
  -DENABLE_BENCHMARKS="$ENABLE_BENCHMARKS" \
  -DENABLE_COVERAGE="$ENABLE_COVERAGE" \
  -DENABLE_SANITIZERS="$ENABLE_SANITIZERS"

echo -e "${GREEN}Configuration complete!${NC}"
