#!/usr/bin/env bash
set -euo pipefail
# shellcheck source=scripts/common.sh
source "$(dirname "$0")/common.sh"

PRESET="linux-release"

case "${1:-}" in
  --release|"") PRESET="linux-release" ;;
  --debug)      PRESET="linux-debug" ;;
  --fuzz)       PRESET="linux-fuzz" ;;
  --bench|--benchmark) PRESET="linux-bench" ;;
  --coverage)   PRESET="linux-coverage" ;;
  --macos-release) PRESET="macos-release" ;;
  --macos-debug)   PRESET="macos-debug" ;;
  --macos-bench)   PRESET="macos-bench" ;;
  --windows-release) PRESET="windows-msvc-release" ;;
  --windows-debug)   PRESET="windows-msvc-debug" ;;
  *) die "usage: configure.sh [--release|--debug|--fuzz|--bench|--coverage|--macos-release|--macos-debug|--macos-bench|--windows-release|--windows-debug]" ;;
esac

echo "Configuring with preset: $PRESET"
cmake --preset "$PRESET"
