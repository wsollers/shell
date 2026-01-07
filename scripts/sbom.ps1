# Copyright (c) 2025 William Sollers
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

Generate SBOM (Software Bill of Materials) using Syft for the specified preset.

Options:
  -Preset NAME       CMake preset to build and analyze (default: windows-msvc-release)
  -Help              Show this help

Examples:
  .\scripts\sbom.ps1                                    # Generate SBOM for windows-msvc-release
  .\scripts\sbom.ps1 -Preset "windows-msvc-debug"       # Generate SBOM for debug build

This script builds the project and generates SBOM files using Syft.
Requires Syft to be installed: https://github.com/anchore/syft
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

Write-Host "Building project with preset: $Preset" -ForegroundColor Green

# Build the project first
Write-Host "Configuring..." -ForegroundColor Yellow
cmake --preset $Preset -DENABLE_TESTING=OFF -DENABLE_FUZZING=OFF -DENABLE_BENCHMARKS=OFF

Write-Host "Building..." -ForegroundColor Yellow
cmake --build --preset $Preset --parallel

Write-Host "Generating SBOM using Syft..." -ForegroundColor Yellow

# Check if Syft is installed
$syftPath = Get-Command syft -ErrorAction SilentlyContinue
if (-not $syftPath) {
    Write-Host "Syft not found. Please install Syft: https://github.com/anchore/syft" -ForegroundColor Red
    exit 1
}

# Generate SBOM files
syft packages "dir:build/$Preset" -o spdx-json=wshell-sbom.spdx.json
syft packages "dir:build/$Preset" -o spdx-tag=wshell-sbom.spdx

Write-Host "SBOM generation complete:" -ForegroundColor Green
Write-Host "  - wshell-sbom.spdx.json (SPDX JSON format)" -ForegroundColor White
Write-Host "  - wshell-sbom.spdx (SPDX tag-value format)" -ForegroundColor White

# Build
Write-Host "Building..." -ForegroundColor Yellow
cmake --build --preset $Preset --parallel

Write-Host "SBOM generation complete" -ForegroundColor Green