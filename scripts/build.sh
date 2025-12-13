#!/bin/bash
set -e
BUILD_DIR="build"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-dir) BUILD_DIR="$2"; shift 2 ;;
        --jobs|-j) JOBS="$2"; shift 2 ;;
        *) shift ;;
    esac
done
[ ! -d "$BUILD_DIR" ] && echo "Build directory not found. Run configure.sh first." && exit 1
echo "Building wshell..."
cmake --build "$BUILD_DIR" --parallel "$JOBS"
echo "Build complete!"
