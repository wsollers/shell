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

# Toolchain defaults
C_COMPILER="clang"
CXX_COMPILER="clang++"

# New: explicit libc++ toggle
FORCE_LIBCXX=OFF

# Extra flags (avoid baking -stdlib=libc++ into all builds unless requested)
EXTRA_CXX_FLAGS="-fPIC"
EXTRA_EXE_LINKER_FLAGS=""
EXTRA_SHARED_LINKER_FLAGS=""

usage() {
  cat <<'EOF'
Usage: ./scripts/configure.sh [OPTIONS]

Build presets:
  --debug          Configure for Debug build
  --release        Configure for Release build (default)
  --test           Configure for testing (RelWithDebInfo + sanitizers)
  --coverage       Configure for coverage (Debug + coverage)

Feature toggles:
  --fuzz           Enable fuzz testing (Clang 18+ required; will auto-skip if missing)
  --benchmark      Enable benchmarks

Stdlib:
  --libcxx         Force Clang to use libc++ (requires libc++ + libc++abi installed)

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
    --libcxx)    FORCE_LIBCXX=ON; shift ;;
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
EXTRA_DEFS=()
if [[ "$ENABLE_FUZZING" == "ON" ]]; then
  EXTRA_DEFS+=(-DWSHELL_FORCE_LIBCXX=ON)
fi

# If libc++ is requested, set flags.
if [[ "$FORCE_LIBCXX" == "ON" ]]; then
  EXTRA_CXX_FLAGS="${EXTRA_CXX_FLAGS} -stdlib=libc++"
  EXTRA_EXE_LINKER_FLAGS="${EXTRA_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi"
  EXTRA_SHARED_LINKER_FLAGS="${EXTRA_SHARED_LINKER_FLAGS} -stdlib=libc++ -lc++abi"
fi

# If fuzzing is enabled, we REQUIRE:
# - Clang
# - Clang >= 18
# - (if --libcxx) libc++ usable
if [[ "$ENABLE_FUZZING" == "ON" ]]; then
  if ! "$CXX_COMPILER" --version 2>/dev/null | grep -qi clang; then
    echo -e "${YELLOW}Fuzzing: skipped (requires Clang; detected: $("$CXX_COMPILER" --version | head -n 1))${NC}"
    ENABLE_FUZZING=OFF
  else
    CLANG_VER="$("$CXX_COMPILER" --version | head -n 1 | sed -n 's/.*clang version \([0-9]\+\).*/\1/p')"
    CLANG_VER="${CLANG_VER:-0}"
    if [[ "$CLANG_VER" -lt 18 ]]; then
      echo -e "${YELLOW}Fuzzing: skipped (requires Clang >= 18; detected major=$CLANG_VER)${NC}"
      ENABLE_FUZZING=OFF
    fi
  fi

  if [[ "$ENABLE_FUZZING" == "ON" && "$FORCE_LIBCXX" == "ON" ]]; then
    # Quick libc++ sanity test: preprocess should succeed with -stdlib=libc++
    if ! echo | "$CXX_COMPILER" -x c++ -std=c++23 -stdlib=libc++ -E - >/dev/null 2>&1; then
      echo -e "${YELLOW}Fuzzing: skipped (libc++ not usable; install libc++/libc++abi dev packages)${NC}"
      ENABLE_FUZZING=OFF
    fi
  fi
fi

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
echo "Force libc++: $FORCE_LIBCXX"
echo "Extra CXX flags:          ${EXTRA_CXX_FLAGS}"
echo "Extra EXE linker flags:   ${EXTRA_EXE_LINKER_FLAGS:-<none>}"
echo "Extra SHARED linker flags:${EXTRA_SHARED_LINKER_FLAGS:-<none>}"

mkdir -p "$BUILD_DIR"

cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_C_COMPILER="$C_COMPILER" \
  -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
  -DCMAKE_CXX_FLAGS="$EXTRA_CXX_FLAGS" \
  -DCMAKE_EXE_LINKER_FLAGS="$EXTRA_EXE_LINKER_FLAGS" \
  -DCMAKE_SHARED_LINKER_FLAGS="$EXTRA_SHARED_LINKER_FLAGS" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DENABLE_TESTING="$ENABLE_TESTING" \
  -DENABLE_FUZZING="$ENABLE_FUZZING" \
  -DENABLE_BENCHMARKS="$ENABLE_BENCHMARKS" \
  -DENABLE_COVERAGE="$ENABLE_COVERAGE" \
  -DENABLE_SANITIZERS="$ENABLE_SANITIZERS" \
  "${EXTRA_DEFS[@]}"

echo -e "${GREEN}Configuration complete!${NC}"
echo "Next: ./scripts/build.sh --build-dir $BUILD_DIR"
