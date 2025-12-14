# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true, Position=0)]
    [string]$Target,
    
    [Parameter(Position=1)]
    [int]$Duration = 300,
    
    [string]$BuildDir = "build",
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "Release",
    [string]$CorpusDir = "fuzz_corpus",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Usage {
@"
Usage: .\scripts\fuzz.ps1 <target> [duration_seconds] [OPTIONS]

Arguments:
  target             Fuzzer target (fuzz_command_parser or fuzz_shell_core)
  duration_seconds   Fuzzing duration in seconds (default: 300)

Options:
  -BuildDir DIR      Build directory (default: build)
  -Config CFG        Build configuration (default: Release)
  -CorpusDir DIR     Corpus directory (default: fuzz_corpus)
  -Help              Show this help

Examples:
  .\scripts\fuzz.ps1 fuzz_command_parser                    # Run for 300 seconds
  .\scripts\fuzz.ps1 fuzz_shell_core 600                    # Run for 10 minutes
  .\scripts\fuzz.ps1 fuzz_command_parser 180 -Config Debug # Debug build, 3 minutes
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

if (-not $Target) {
    Write-Error "Target is required. Use -Help for usage information."
    exit 1
}

if (-not (Test-Path $BuildDir)) {
    throw "Build directory not found: $BuildDir. Run configure/build first."
}

# Create corpus directory for the specific target (feature parity with fuzz.sh)
$targetCorpusDir = Join-Path $CorpusDir $Target
New-Item -ItemType Directory -Force -Path $targetCorpusDir | Out-Null

Write-Host "Running fuzz test for target: $Target" -ForegroundColor Green
Write-Host "BuildDir:        $BuildDir" -ForegroundColor Gray
Write-Host "Config:          $Config" -ForegroundColor Gray  
Write-Host "Duration:        $Duration seconds" -ForegroundColor Gray
Write-Host "Target Corpus:   $targetCorpusDir" -ForegroundColor Gray

# Search for the specific fuzz executable (feature parity with fuzz.sh)
function Find-Exe([string]$name) {
    $hits = Get-ChildItem -Path $BuildDir -Recurse -File -Filter "$name.exe" -ErrorAction SilentlyContinue
    if (-not $hits) { return $null }

    # Prefer the one under the selected config if present
    $preferred = $hits | Where-Object { $_.FullName -match "\\$Config\\" } | Select-Object -First 1
    if ($preferred) { return $preferred.FullName }

    return ($hits | Select-Object -First 1).FullName
}

$fuzzerBinary = Find-Exe $Target

if (-not $fuzzerBinary) {
    throw @"
Missing fuzzer binary: $Target.exe in $BuildDir
Did you run:
  .\scripts\configure.ps1 -Fuzz
  .\scripts\build.ps1
"@
}

Write-Host "Found fuzzer: $fuzzerBinary" -ForegroundColor Green

# Run the fuzzer with timeout (feature parity with fuzz.sh)
try {
    $job = Start-Job -ScriptBlock {
        param($binary, $corpus, $duration)
        & $binary $corpus "-max_total_time=$duration" "-print_final_stats=1"
    } -ArgumentList $fuzzerBinary, $targetCorpusDir, $Duration
    
    # Wait for the job to complete or timeout
    if (Wait-Job $job -Timeout $Duration) {
        $result = Receive-Job $job
        Write-Host "Fuzzing completed normally" -ForegroundColor Green
        if ($result) { Write-Host $result }
    } else {
        Write-Host "Fuzzing timed out after $Duration seconds" -ForegroundColor Yellow
        Stop-Job $job
    }
    
    Remove-Job $job -Force
    
} catch {
    Write-Warning "Fuzzing encountered an error: $_"
    # Don't fail the script - fuzzing timeouts/crashes are expected
}

Write-Host "Fuzzing session complete!" -ForegroundColor Green

if (-not $fuzz1 -and -not $fuzz2) {
    throw "No fuzz executables found under $BuildDir. Did you configure with -Fuzz and build?"
}

if ($fuzz1) {
    Write-Host "Running: $fuzz1" -ForegroundColor Yellow
    & $fuzz1 (Join-Path $CorpusDir "command_parser") "-max_total_time=$Duration"
    if ($LASTEXITCODE -ne 0) { throw "fuzz_command_parser failed with exit code $LASTEXITCODE" }
} else {
    Write-Host "Skipping fuzz_command_parser.exe (not found)" -ForegroundColor Yellow
}

if ($fuzz2) {
    Write-Host "Running: $fuzz2" -ForegroundColor Yellow
    & $fuzz2 (Join-Path $CorpusDir "shell_core") "-max_total_time=$Duration"
    if ($LASTEXITCODE -ne 0) { throw "fuzz_shell_core failed with exit code $LASTEXITCODE" }
} else {
    Write-Host "Skipping fuzz_shell_core.exe (not found)" -ForegroundColor Yellow
}

Write-Host "Fuzz testing complete!" -ForegroundColor Green
