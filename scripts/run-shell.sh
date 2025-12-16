#!/usr/bin/env bash
# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

set -euo pipefail

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Load common functions
source "${SCRIPT_DIR}/common.sh"

echo "================================"
echo "Build and Run Shell"
echo "================================"
echo ""

# Determine build directory
BUILD_DIR="${PROJECT_ROOT}/build/linux-release"

# Build the project
echo "Building project..."
if ! cmake --build "${BUILD_DIR}" --parallel; then
    echo "error: Build failed" >&2
    exit 1
fi

echo "Build completed successfully"
echo ""

# Find the shell executable
SHELL_EXEC="${BUILD_DIR}/src/main/wshell"

if [[ ! -f "${SHELL_EXEC}" ]]; then
    echo "error: Shell executable not found at: ${SHELL_EXEC}" >&2
    exit 1
fi

echo "Starting wshell..."
echo ""
echo "Type 'exit' to quit the shell"
echo "Press Ctrl+C to interrupt"
echo ""

# Run the shell
"${SHELL_EXEC}" "$@"
