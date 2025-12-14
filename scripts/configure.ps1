# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

[CmdletBinding()]
param(
    [switch]$Debug,
    [switch]$Release,
    [switch]$Test,
    [switch]$Coverage,
    [switch]$Fuzz,
    [switch]$Benchmark,

    [string]$BuildDir = "build",

    # Optional: choose generator/arch explicitly (defaults match MSVC 2022 x64 best practice)
    [string]$Generator = "Visual Studio 17 2022",
    [string]$Arch = "x64",

    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Usage {
@"
Usage: .\scripts\configure.ps1 [OPTIONS]

Build modes (choose one; default: -Release):
  -Debug         Configure Debug
  -Release       Configure Release
  -Test          Configure RelWithDebInfo + ENABLE_SANITIZERS=ON (MSVC may ignore sanitizers unless clang-cl)
  -Coverage      Configure Debug + ENABLE_COVERAGE=ON (coverage collection is typically Linux/Clang)

Feature toggles:
  -Fuzz          ENABLE_FUZZING=ON
  -Benchmark     ENABLE_BENCHMARKS=ON

Other:
  -BuildDir DIR  Build directory (default: build)
  -Generator STR CMake generator (default: Visual Studio 17 2022)
  -Arch STR      Generator architecture (default: x64)
  -Help          Show help

Examples:
  .\scripts\configure.ps1 -Debug
  .\scripts\configure.ps1 -Release -Benchmark
  .\scripts\configure.ps1 -Coverage -BuildDir build
"@
}

if ($Help) { Show-Usage; exit 0 }

# Determine config semantics (MSVC generators are typically multi-config)
$BuildType = "Release"
if ($Debug)   { $BuildType = "Debug" }
if ($Test)    { $BuildType = "RelWithDebInfo" }
if ($Coverage){ $BuildType = "Debug" }
if (-not ($Debug -or $Release -or $Test -or $Coverage)) { $BuildType = "Release" }

$EnableTesting    = "ON"
$EnableFuzzing    = if ($Fuzz) { "ON" } else { "OFF" }
$EnableBenchmarks = if ($Benchmark) { "ON" } else { "OFF" }
$EnableCoverage   = if ($Coverage) { "ON" } else { "OFF" }
$EnableSanitizers = if ($Test) { "ON" } else { "OFF" }

Write-Host "Configuring wshell..." -ForegroundColor Green
Write-Host "Build directory: $BuildDir"
Write-Host "Build type:      $BuildType"
Write-Host "Testing:         $EnableTesting"
Write-Host "Fuzzing:         $EnableFuzzing"
Write-Host "Benchmarks:      $EnableBenchmarks"
Write-Host "Coverage:        $EnableCoverage"
Write-Host "Sanitizers:      $EnableSanitizers"
Write-Host "Generator:       $Generator"
Write-Host "Arch:            $Arch"

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

# For Visual Studio generators, CMAKE_BUILD_TYPE is ignored at build time,
# but we still pass it because it helps non-MSVC generators and keeps parity with Unix.
$cmakeArgs = @(
  "-S", ".", "-B", $BuildDir,
  "-G", $Generator,
  "-A", $Arch,
  "-DCMAKE_BUILD_TYPE=$BuildType",
  "-DENABLE_TESTING=$EnableTesting",
  "-DENABLE_FUZZING=$EnableFuzzing",
  "-DENABLE_BENCHMARKS=$EnableBenchmarks",
  "-DENABLE_COVERAGE=$EnableCoverage",
  "-DENABLE_SANITIZERS=$EnableSanitizers"
)

& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed with exit code $LASTEXITCODE" }

Write-Host "Configuration complete!" -ForegroundColor Green
Write-Host "Next:" -ForegroundColor Green
Write-Host "  .\scripts\build.ps1 -BuildDir $BuildDir -Config $BuildType"
Write-Host "  .\scripts\test.ps1  -BuildDir $BuildDir -Config $BuildType"
