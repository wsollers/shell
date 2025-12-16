#!/usr/bin/env bash
set -euo pipefail
# shellcheck source=scripts/common.sh
source "$(dirname "$0")/common.sh"

# Handle help
if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
    cat << EOF
Usage: $0 [PRESET]

Generate SBOM (Software Bill of Materials) using Syft for the specified preset.

Arguments:
  PRESET     CMake preset to build and analyze (default: linux-release)

Examples:
  $0                    # Generate SBOM for linux-release
  $0 linux-debug       # Generate SBOM for linux-debug

This script builds the project and generates SBOM files using Syft.
Requires Syft to be installed: https://github.com/anchore/syft
EOF
    exit 0
fi

PRESET="${1:-linux-release}"

echo "Building project with preset: $PRESET"

# Build the project first
echo "Configuring..."
cmake --preset "$PRESET" -DENABLE_TESTING=OFF -DENABLE_FUZZING=OFF -DENABLE_BENCHMARKS=OFF

echo "Building..."
cmake --build --preset "$PRESET" --parallel

echo "Generating SBOM using Syft..."

# Check if Syft is installed
if ! command -v syft >/dev/null 2>&1; then
    echo "Syft not found. Installing..."
    curl -sSfL https://raw.githubusercontent.com/anchore/syft/main/install.sh | sh -s -- -b /usr/local/bin
fi

# Generate SBOM files
syft packages "dir:build/$PRESET" -o spdx-json=wshell-sbom.spdx.json
syft packages "dir:build/$PRESET" -o spdx-tag=wshell-sbom.spdx

echo "SBOM generation complete:"
echo "  - wshell-sbom.spdx.json (SPDX JSON format)"
echo "  - wshell-sbom.spdx (SPDX tag-value format)"