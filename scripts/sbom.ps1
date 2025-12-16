# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

[CmdletBinding()]
param(
    [string]$Preset = "windows-msvc-release",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Usage {
@"
Usage: .\scripts\sbom.ps1 [OPTIONS]

Generate SBOM (Software Bill of Materials) for the specified preset.

Options:
  -Preset NAME       CMake preset to build with SBOM (default: windows-msvc-release)
  -Help              Show this help

Examples:
  .\scripts\sbom.ps1                                    # Generate SBOM for windows-msvc-release
  .\scripts\sbom.ps1 -Preset "windows-msvc-debug"       # Generate SBOM for debug build

This script enables SBOM generation and requires Python dependencies for
SBOM tools (reuse, spdx-tools, ntia-conformance-checker).
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

Write-Host "Configuring and building with SBOM for preset: $Preset" -ForegroundColor Green

# Configure with SBOM enabled
Write-Host "Configuring..." -ForegroundColor Yellow
cmake --preset $Preset -DENABLE_SBOM=ON -DENABLE_TESTING=OFF -DENABLE_FUZZING=OFF -DENABLE_BENCHMARKS=OFF

# Build
Write-Host "Building..." -ForegroundColor Yellow
cmake --build --preset $Preset --parallel

Write-Host "SBOM generation complete" -ForegroundColor Green