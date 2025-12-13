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
  --config CFG        Debug|Release|RelWithDebInfo|MinSizeRel (multi-config)
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

[[ -d "$BUILD_DIR" ]] || { echo "Build directory not found: $BUILD_DIR"; exit 1; }

echo "Running tests..."
cd "$BUILD_DIR"

echo "Discovered tests:"
ctest -N || true

# (Optional) make ctest print output on failure automatically
export CTEST_OUTPUT_ON_FAILURE=1

echo "Executing tests (ctest -VV)..."
set +e
if [[ -n "$CONFIG" ]]; then
  ctest -VV --output-on-failure --progress -C "$CONFIG"
else
  ctest -VV --output-on-failure --progress
fi
RC=$?
set -e

echo "ctest exit code: $RC"

if [[ $RC -ne 0 ]]; then
  echo "----- Diagnostics -----"
  echo "PWD: $PWD"

  echo "Looking for test executable(s):"
  find . -maxdepth 5 -type f -name wshell_tests -print || true

  echo "If this is a runtime loader issue, try running the binary directly:"
  echo "  ./test/wshell_tests --gtest_list_tests"
  echo "  (or whatever path was printed above)"

  # Helpful on Linux if loader is failing:
  if command -v ldd >/dev/null 2>&1; then
    WSHELL="$(find . -maxdepth 5 -type f -name wshell_tests -perm -111 | head -n 1)"
    if [[ -n "${WSHELL:-}" ]]; then
      echo "ldd on $WSHELL:"
      ldd "$WSHELL" || true
    fi
  fi

  exit "$RC"
fi

echo "All tests passed."
