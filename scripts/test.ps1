# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

[CmdletBinding()]
param(
    [string]$Preset = "windows-msvc-release",
    [string]$BuildDir = "",
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "",
    [switch]$List,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Usage {
@"
Usage: .\scripts\test.ps1 [OPTIONS]

Options:
  -Preset NAME       CTest preset to run (default: windows-msvc-release)
  -BuildDir DIR      Override build directory (extracted from preset if not specified)
  -Config CFG        Override configuration (extracted from preset if not specified)
  -List              List tests without running them
  -Help              Show this help

Examples:
  .\scripts\test.ps1                                    # Default windows-msvc-release
  .\scripts\test.ps1 -Preset "windows-msvc-debug"       # Debug tests
  .\scripts\test.ps1 -List                              # List available tests
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

# Use CTest preset for testing (feature parity with test.sh)
Write-Host "Running tests with preset: $Preset" -ForegroundColor Green

if ($List) {
    & ctest --preset $Preset --show-only
    if ($LASTEXITCODE -ne 0) { throw "ctest list failed with exit code $LASTEXITCODE" }
} else {
    & ctest --preset $Preset --output-on-failure
    $rc = $LASTEXITCODE
    if ($rc -eq 0) {
        Write-Host "All tests passed." -ForegroundColor Green
    } else {
        throw "Tests failed with exit code $rc"
    }
}
