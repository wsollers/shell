#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-2-Clause

set -euo pipefail

BUILD_DIR="build"
CONFIG=""  # for multi-config generators
OUT_FILE="coverage.info"

usage() {
  cat <<'EOF'
Usage: ./scripts/coverage.sh [OPTIONS]

Options:
  --build-dir DIR     Build directory (default: build)
  --config CFG        Debug|Release|RelWithDebInfo|MinSizeRel (for multi-config)
  --out FILE          Output LCOV file (default: coverage.info)

Notes:
- This script generates LLVM source-based coverage (Clang):
  - runs tests (via scripts/test.sh for consistency)
  - merges .profraw -> .profdata
  - exports LCOV via llvm-cov
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir) BUILD_DIR="${2:?Missing value for --build-dir}"; shift 2 ;;
    --config)    CONFIG="${2:?Missing value for --config}"; shift 2 ;;
    --out)       OUT_FILE="${2:?Missing value for --out}"; shift 2 ;;
    --help|-h)   usage; exit 0 ;;
    *)
      echo "Unknown option: $1"
      usage
      exit 1
      ;;
  esac
done

# Robust guard: do NOT use `[ ... ] && ...` with `set -e`
[[ -d "$BUILD_DIR" ]] || { echo "Build directory not found: $BUILD_DIR"; exit 1; }

cd "$BUILD_DIR"

[[ -f CMakeCache.txt ]] || { echo "CMakeCache.txt not found in $BUILD_DIR; run configure first."; exit 1; }

grep -q '^ENABLE_COVERAGE:BOOL=ON$' CMakeCache.txt || {
  echo "ENABLE_COVERAGE is OFF. Reconfigure with:"
  echo "  ./scripts/configure.sh --coverage --build-dir $BUILD_DIR"
  exit 1
}

COMPILER_ID="$(grep -E '^CMAKE_CXX_COMPILER_ID:STRING=' CMakeCache.txt | sed 's/^CMAKE_CXX_COMPILER_ID:STRING=//')"
[[ "$COMPILER_ID" == "Clang" ]] || {
  echo "This script expects Clang (LLVM coverage). Detected compiler: $COMPILER_ID"
  exit 1
}

command -v llvm-profdata >/dev/null 2>&1 || { echo "llvm-profdata not found in PATH"; exit 1; }
command -v llvm-cov >/dev/null 2>&1 || { echo "llvm-cov not found in PATH"; exit 1; }

# Ensure deterministic profile output location (ABSOLUTE path)
mkdir -p coverage
export LLVM_PROFILE_FILE="$PWD/coverage/wshell-%p-%m.profraw"
echo "LLVM_PROFILE_FILE=$LLVM_PROFILE_FILE"

echo "Running tests to generate .profraw..."
# Use the repository script so behavior is consistent locally + CI.
# We are currently in $BUILD_DIR; jump back to repo root via .. to call scripts/test.sh.
if [[ -n "$CONFIG" ]]; then
  (cd .. && ./scripts/test.sh --build-dir "$BUILD_DIR" --config "$CONFIG")
else
  (cd .. && ./scripts/test.sh --build-dir "$BUILD_DIR")
fi

# Collect profraws robustly (in case some ended up elsewhere)
echo "Collecting .profraw files..."
mapfile -t PROFRAWS < <(find . -type f -name '*.profraw' -print | sort)

if [[ ${#PROFRAWS[@]} -eq 0 ]]; then
  echo "No .profraw files generated."
  echo "Sanity hints:"
  echo "  - Was the build configured with Clang coverage flags?"
  echo "  - Is ENABLE_COVERAGE=ON?"
  echo "  - Does the test binary actually run instrumented code?"
  exit 1
fi

echo "Found ${#PROFRAWS[@]} profraw files."

echo "Merging profiles..."
llvm-profdata merge -sparse "${PROFRAWS[@]}" -o coverage/wshell.profdata

# Find test binary (portable)
WSHELL_TESTS="$(find . -type f -name wshell_tests -perm -111 | head -n 1 || true)"
[[ -n "$WSHELL_TESTS" ]] || { echo "wshell_tests not found under $BUILD_DIR"; exit 1; }

# Include shared library object if present (better coverage completeness)
WSHELL_LIB="$(find . -type f \( -name 'libwshell*.so' -o -name 'libwshell*.dylib' \) | head -n 1 || true)"

echo "Exporting LCOV to $OUT_FILE..."
if [[ -n "$WSHELL_LIB" ]]; then
  llvm-cov export "$WSHELL_TESTS" \
    -object "$WSHELL_LIB" \
    -instr-profile=coverage/wshell.profdata \
    -format=lcov \
    -ignore-filename-regex='.*/(test|_deps)/.*' \
    > "$OUT_FILE"
else
  llvm-cov export "$WSHELL_TESTS" \
    -instr-profile=coverage/wshell.profdata \
    -format=lcov \
    -ignore-filename-regex='.*/(test|_deps)/.*' \
    > "$OUT_FILE"
fi

echo "Done: $BUILD_DIR/$OUT_FILE"
