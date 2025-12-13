#!/bin/bash
# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

set -e

sudo apt-get install libc++-dev libc++abi-dev

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

BUILD_TYPE="Release"
BUILD_DIR="build"
ENABLE_TESTING=ON
ENABLE_FUZZING=OFF
ENABLE_BENCHMARKS=OFF
ENABLE_COVERAGE=OFF
ENABLE_SANITIZERS=OFF

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug) BUILD_TYPE="Debug"; shift ;;
        --release) BUILD_TYPE="Release"; shift ;;
        --test) BUILD_TYPE="RelWithDebInfo"; ENABLE_SANITIZERS=ON; shift ;;
        --coverage) BUILD_TYPE="Debug"; ENABLE_COVERAGE=ON; shift ;;
        --fuzz) ENABLE_FUZZING=ON; shift ;;
        --benchmark) ENABLE_BENCHMARKS=ON; shift ;;
        --build-dir) BUILD_DIR="$2"; shift 2 ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --debug          Configure for Debug build"
            echo "  --release        Configure for Release build (default)"
            echo "  --test           Configure for testing with sanitizers"
            echo "  --coverage       Configure for code coverage"
            echo "  --fuzz           Enable fuzz testing"
            echo "  --benchmark      Enable benchmarks"
            echo "  --build-dir DIR  Set build directory (default: build)"
            echo "  --help           Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

echo -e "${GREEN}Configuring wshell...${NC}"
echo "Build type: $BUILD_TYPE"
echo "Build directory: $BUILD_DIR"
echo "Testing: $ENABLE_TESTING"
echo "Fuzzing: $ENABLE_FUZZING"
echo "Benchmarks: $ENABLE_BENCHMARKS"
echo "Coverage: $ENABLE_COVERAGE"
echo "Sanitizers: $ENABLE_SANITIZERS"

mkdir -p "$BUILD_DIR"

cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_FLAGS="-stdlib=libc++" \
    -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -lc++abi" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DENABLE_TESTING="$ENABLE_TESTING" \
    -DENABLE_FUZZING="$ENABLE_FUZZING" \
    -DENABLE_BENCHMARKS="$ENABLE_BENCHMARKS" \
    -DENABLE_COVERAGE="$ENABLE_COVERAGE" \
    -DENABLE_SANITIZERS="$ENABLE_SANITIZERS"

echo -e "${GREEN}Configuration complete!${NC}"
echo "To build, run: ./scripts/build.sh"
