# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

[CmdletBinding()]
param(
    [string]$Preset = "windows-msvc-release",
    [string]$BuildDir = "",
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "",
    [int]$Jobs = 0,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Usage {
@"
Usage: .\scripts\build.ps1 [OPTIONS]

Options:
  -Preset NAME       CMake preset to build (default: windows-msvc-release)
  -BuildDir DIR      Override build directory (extracted from preset if not specified)
  -Config CFG        Override configuration (extracted from preset if not specified)  
  -Jobs N            Parallel jobs (default: auto-detected CPU cores)
  -Help              Show this help

Examples:
  .\scripts\build.ps1                                    # Default windows-msvc-release
  .\scripts\build.ps1 -Preset "windows-msvc-debug"       # Debug build
  .\scripts\build.ps1 -Jobs 8                            # Use 8 parallel jobs
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

if ($Jobs -le 0) {
    try { $Jobs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors }
    catch { $Jobs = 4 }
}

# Use CMake preset for building (feature parity with build.sh)
Write-Host "Building with preset: $Preset" -ForegroundColor Green
Write-Host "Parallel jobs: $Jobs" -ForegroundColor Gray

& cmake --build --preset $Preset --parallel $Jobs
if ($LASTEXITCODE -ne 0) { 
    throw "Build failed with exit code $LASTEXITCODE" 
}

Write-Host "Build complete!" -ForegroundColor Green
