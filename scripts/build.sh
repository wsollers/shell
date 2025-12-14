#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/common.sh"

PRESET="${1:-linux-release}"

echo "Building with preset: $PRESET"
cmake --build --preset "$PRESET" --parallel
