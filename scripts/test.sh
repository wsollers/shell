#!/bin/bash
set -e
BUILD_DIR="build"
[ ! -d "$BUILD_DIR" ] && echo "Build directory not found." && exit 1
echo "Running tests..."
cd "$BUILD_DIR" && ctest --output-on-failure --verbose
echo "All tests passed!"
