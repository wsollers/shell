# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

[CmdletBinding()]
param(
    [switch]$Release,
    [switch]$Debug,
    [switch]$Fuzz,
    [switch]$Benchmark,
    [switch]$Coverage,
    [switch]$Test,
    
    # Cross-platform preset support
    [switch]$MacOSRelease,
    [switch]$MacOSDebug, 
    [switch]$MacOSBench,
    [switch]$LinuxRelease,
    [switch]$LinuxDebug,
    
    [string]$Preset = "",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Usage {
@"
Usage: .\scripts\configure.ps1 [OPTIONS]

Build modes (choose one; default: -Release):
  -Release           Windows MSVC Release
  -Debug             Windows MSVC Debug
  -Test              Windows MSVC RelWithDebInfo + testing features
  -Coverage          Windows Debug + coverage (limited on Windows)
  -Fuzz              Windows Fuzzing build 
  -Benchmark         Windows Benchmark build
  
Cross-platform presets:
  -MacOSRelease      macOS Release build
  -MacOSDebug        macOS Debug build
  -MacOSBench        macOS Benchmark build
  -LinuxRelease      Linux Release build (cross-compile)
  -LinuxDebug        Linux Debug build (cross-compile)
  
Other:
  -Preset NAME       Use specific CMake preset directly
  -Help              Show this help

Examples:
  .\scripts\configure.ps1 -Release
  .\scripts\configure.ps1 -Debug -Coverage 
  .\scripts\configure.ps1 -Preset "windows-msvc-debug"
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

# Determine preset based on switches (feature parity with configure.sh)
$selectedPreset = ""

if ($Preset) {
    $selectedPreset = $Preset
} elseif ($Debug) {
    $selectedPreset = "windows-msvc-debug"
} elseif ($Fuzz) {
    # Note: Fuzzing may not work well on Windows/MSVC, but preset should exist
    $selectedPreset = "windows-msvc-release"  # Fallback - fuzzing typically needs Clang
    Write-Warning "Fuzzing typically requires Clang. Consider Linux build for fuzzing."
} elseif ($Benchmark) {
    $selectedPreset = "windows-msvc-release"  # Use release for benchmarks
} elseif ($Coverage) {
    $selectedPreset = "windows-msvc-debug"
    Write-Warning "Coverage collection is limited on Windows. Consider Linux build for full coverage support."
} elseif ($Test) {
    $selectedPreset = "windows-msvc-debug"
} elseif ($MacOSRelease) {
    $selectedPreset = "macos-release"
} elseif ($MacOSDebug) {
    $selectedPreset = "macos-debug" 
} elseif ($MacOSBench) {
    $selectedPreset = "macos-bench"
} elseif ($LinuxRelease) {
    $selectedPreset = "linux-release"
} elseif ($LinuxDebug) {
    $selectedPreset = "linux-debug"
} else {
    # Default to release (feature parity with configure.sh)
    $selectedPreset = "windows-msvc-release"
}

Write-Host "Configuring with preset: $selectedPreset" -ForegroundColor Green
& cmake --preset $selectedPreset
if ($LASTEXITCODE -ne 0) { 
    throw "CMake configuration failed with exit code $LASTEXITCODE" 
}

Write-Host "Configuration complete!" -ForegroundColor Green

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
