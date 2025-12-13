# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

param(
    [switch]$Debug,
    [switch]$Release,
    [switch]$Test,
    [switch]$Coverage,
    [switch]$Fuzz,
    [switch]$Benchmark,
    [string]$BuildDir = "build",
    [switch]$Help
)

if ($Help) {
    Write-Host "Usage: .\configure.ps1 [OPTIONS]"
    Write-Host "Options:"
    Write-Host "  -Debug          Configure for Debug build"
    Write-Host "  -Release        Configure for Release build (default)"
    Write-Host "  -Test           Configure for testing with sanitizers"
    Write-Host "  -Coverage       Configure for code coverage"
    Write-Host "  -Fuzz           Enable fuzz testing"
    Write-Host "  -Benchmark      Enable benchmarks"
    Write-Host "  -BuildDir DIR   Set build directory (default: build)"
    Write-Host "  -Help           Show this help message"
    exit 0
}

$BuildType = "Release"
$EnableTesting = "ON"
$EnableFuzzing = "OFF"
$EnableBenchmarks = "OFF"
$EnableCoverage = "OFF"
$EnableSanitizers = "OFF"

if ($Debug) { $BuildType = "Debug" }
elseif ($Test) { $BuildType = "RelWithDebInfo"; $EnableSanitizers = "ON" }
elseif ($Coverage) { $BuildType = "Debug"; $EnableCoverage = "ON" }

if ($Fuzz) { $EnableFuzzing = "ON" }
if ($Benchmark) { $EnableBenchmarks = "ON" }

Write-Host "Configuring wshell..." -ForegroundColor Green
Write-Host "Build type: $BuildType"
Write-Host "Build directory: $BuildDir"
Write-Host "Testing: $EnableTesting"
Write-Host "Fuzzing: $EnableFuzzing"
Write-Host "Benchmarks: $EnableBenchmarks"
Write-Host "Coverage: $EnableCoverage"
Write-Host "Sanitizers: $EnableSanitizers"

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

cmake -S . -B $BuildDir `
    -DCMAKE_BUILD_TYPE=$BuildType `
    -DENABLE_TESTING=$EnableTesting `
    -DENABLE_FUZZING=$EnableFuzzing `
    -DENABLE_BENCHMARKS=$EnableBenchmarks `
    -DENABLE_COVERAGE=$EnableCoverage `
    -DENABLE_SANITIZERS=$EnableSanitizers

if ($LASTEXITCODE -eq 0) {
    Write-Host "Configuration complete!" -ForegroundColor Green
    Write-Host "To build, run: .\scripts\build.ps1"
} else {
    Write-Host "Configuration failed!" -ForegroundColor Red
    exit 1
}
