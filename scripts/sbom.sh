#!/usr/bin/env bash
set -euo pipefail
# shellcheck source=scripts/common.sh
source "$(dirname "$0")/common.sh"

# Handle help
if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
    cat << EOF
Usage: $0 [PRESET]

Generate SBOM (Software Bill of Materials) for the specified preset.

Arguments:
  PRESET     CMake preset to build with SBOM (default: linux-release)

Examples:
  $0                    # Generate SBOM for linux-release
  $0 linux-debug       # Generate SBOM for linux-debug

This script enables SBOM generation and installs Python dependencies required
for SBOM tools (reuse, spdx-tools, ntia-conformance-checker).
EOF
    exit 0
fi

PRESET="${1:-linux-release}"

echo "Configuring and building with SBOM for preset: $PRESET"

# Configure with SBOM enabled
echo "Configuring..."
cmake --preset "$PRESET" -DENABLE_SBOM=ON -DENABLE_TESTING=OFF -DENABLE_FUZZING=OFF -DENABLE_BENCHMARKS=OFF

# Build
echo "Building..."
cmake --build --preset "$PRESET" --parallel

echo "SBOM generation complete"