#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/common.sh"

PRESET="${1:-linux-release}"

echo "Running tests with preset: $PRESET"
ctest --preset "$PRESET" --output-on-failure
