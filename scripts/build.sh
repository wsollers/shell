#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause

set -euo pipefail

BUILD_DIR="build"
JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
CONFIG=""  # for multi-config generators (Xcode/VS)

usage() {
  cat <<'EOF'
Usage: ./scripts/build.sh [OPTIONS]

Options:
  --build-dir DIR     Build directory (default: build)
  --jobs N, -j N      Parallel jobs (default: auto)
  --config CFG        Debug|Release|RelWithDebInfo|MinSizeRel (for multi-config)

EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir) BUILD_DIR="${2:?Missing value for --build-dir}"; shift 2 ;;
    --jobs|-j)   JOBS="${2:?Missing value for --jobs}"; shift 2 ;;
    --config)    CONFIG="${2:?Missing value for --config}"; shift 2 ;;
    --help|-h)   usage; exit 0 ;;
    *)
      echo "Unknown option: $1"
      usage
      exit 1
      ;;
  esac
done

[ ! -d "$BUILD_DIR" ] && echo "Build directory not found. Run configure.sh first." && exit 1

echo "Building wshell..."
if [[ -n "$CONFIG" ]]; then
  cmake --build "$BUILD_DIR" --config "$CONFIG" --parallel "$JOBS"
else
  cmake --build "$BUILD_DIR" --parallel "$JOBS"
fi
echo "Build complete!"
