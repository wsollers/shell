#!/bin/bash
BUILD_DIR="build"
[ -d "$BUILD_DIR" ] && rm -rf "$BUILD_DIR" && echo "Clean complete!" || echo "Nothing to clean."
