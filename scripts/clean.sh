#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
if [[ -d "$BUILD_DIR" ]]; then
  rm -rf "$BUILD_DIR"
  echo "Clean complete!"
else
  echo "Nothing to clean."
fi
