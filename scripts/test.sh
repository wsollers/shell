#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause

set -euo pipefail

BUILD_DIR="build"
CONFIG=""

usage() {
  cat <<'EOF'
Usage: ./scripts/test.sh [OPTIONS]

Options:
  --build-dir DIR     Build directory (default: build)
  --config CFG        Debug|Release|RelWithDebInfo|MinSizeRel (for multi-config)
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir) BUILD_DIR="${2:?Missing value for --build-dir}"; shift 2 ;;
    --config)    CONFIG="${2:?Missing value for --config}"; shift 2 ;;
    --help|-h)   usage; exit 0 ;;
    *) echo "Unknown option: $1"; usage; exit 1 ;;
  esac
done

[ ! -d "$BUILD_DIR" ] && echo "Build directory not found: $BUILD_DIR" && exit 1

echo "Running tests..."
cd "$BUILD_DIR"

# Print test count up front (so you always know it saw tests)
echo "Discovered tests:"
ctest -N || true

# Coverage/Clang profile placement (deterministic)
COVERAGE_ENABLED="OFF"
COMPILER_ID=""
if [[ -f "CMakeCache.txt" ]]; then
  grep -q '^ENABLE_COVERAGE:BOOL=ON$' CMakeCache.txt && COVERAGE_ENABLED="ON" || true
  COMPILER_ID="$(grep -E '^CMAKE_CXX_COMPILER_ID:STRING=' CMakeCache.txt | sed 's/^CMAKE_CXX_COMPILER_ID:STRING=//')"
fi

if [[ "${COVERAGE_ENABLED}" == "ON" && "${COMPILER_ID}" == "Clang" ]]; then
  mkdir -p coverage
  export LLVM_PROFILE_FILE="${LLVM_PROFILE_FILE:-$PWD/coverage/wshell-%p-%m.profraw}"
  case "$LLVM_PROFILE_FILE" in
    /*) : ;;
    *) export LLVM_PROFILE_FILE="$PWD/coverage/$LLVM_PROFILE_FILE" ;;
  esac
  echo "Coverage enabled (Clang). LLVM_PROFILE_FILE=$LLVM_PROFILE_FILE"
fi

# Actually run tests, always show progress
if [[ -n "$CONFIG" ]]; then
  ctest --output-on-failure --progress -C "$CONFIG"
else
  ctest --output-on-failure --progress
fi

echo "All tests passed."
