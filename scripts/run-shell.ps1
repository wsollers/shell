# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

<#
.SYNOPSIS
    Build and run the wshell for manual testing

.DESCRIPTION
    This script builds the project and then launches the wshell executable
    for interactive testing.

.PARAMETER Config
    Build configuration (Debug or Release). Default: Release

.EXAMPLE
    .\run-shell.ps1
    .\run-shell.ps1 -Config Debug
#>

param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"

# Script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

Write-Host "================================" -ForegroundColor Cyan
Write-Host "Build and Run Shell" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# Determine build directory
$BuildDir = Join-Path $ProjectRoot "build\windows-msvc-$($Config.ToLower())"

# Build the project
Write-Host "Building project ($Config)..." -ForegroundColor Yellow
try {
    $BuildArgs = @(
        "--build", $BuildDir,
        "--config", $Config,
        "--parallel"
    )
    
    & cmake @BuildArgs
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
} catch {
    Write-Host "Error: $_" -ForegroundColor Red
    exit 1
}

Write-Host "Build completed successfully" -ForegroundColor Green
Write-Host ""

# Find the shell executable
$ShellExec = Join-Path $BuildDir "wshell.exe"

if (-not (Test-Path $ShellExec)) {
    Write-Host "Error: Shell executable not found at: $ShellExec" -ForegroundColor Red
    exit 1
}

Write-Host "Starting wshell..." -ForegroundColor Yellow
Write-Host ""
Write-Host "Type 'exit' to quit the shell"
Write-Host "Press Ctrl+C to interrupt"
Write-Host ""

# Run the shell
try {
    & $ShellExec @args
} catch {
    Write-Host "Shell exited with error: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Shell exited" -ForegroundColor Green
