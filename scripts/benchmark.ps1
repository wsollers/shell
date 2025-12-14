[CmdletBinding()]
param(
    [string]$Preset = "windows-msvc-release",
    [string]$Config = "Release",
    [string]$OutputDir = "benchmark_results"
)

$ErrorActionPreference = "Stop"

$BuildDir = switch ($Preset) {
    "windows-msvc-release" { "build\windows-msvc-release" }
    "windows-msvc-debug"   { "build\windows-msvc-debug" }
    default                { "build\$Preset" }
}

Write-Host "🧹 Cleaning previous builds..." -ForegroundColor Yellow
if (Test-Path "build") {
    Remove-Item -Recurse -Force "build"
    Write-Host "Clean complete!" -ForegroundColor Green
} else {
    Write-Host "Nothing to clean." -ForegroundColor Green
}

Write-Host "🔧 Configuring CMake for benchmarks..." -ForegroundColor Yellow
try {
    # For Windows, we'll use the release preset but enable benchmarks
    & cmake --preset $Preset -DENABLE_BENCHMARKS=ON
    if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }
} catch {
    Write-Host "Failed to configure with preset $Preset, trying manual configuration..." -ForegroundColor Yellow
    & cmake -B $BuildDir -DENABLE_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=$Config
    if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }
}

Write-Host "🔨 Building benchmark targets..." -ForegroundColor Yellow
& cmake --build $BuildDir --config $Config --parallel
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

Write-Host "📊 Running benchmarks..." -ForegroundColor Yellow
Push-Location $BuildDir

try {
    # Create output directory for results
    $FullOutputDir = Join-Path (Get-Location).Path "..\..\" $OutputDir
    New-Item -ItemType Directory -Force -Path $FullOutputDir | Out-Null

    # Run benchmarks and save results
    Write-Host "Running command_parser benchmarks..." -ForegroundColor Cyan
    $BenchCommand = ".\bench\$Config\bench_command_parser.exe"
    if (-not (Test-Path $BenchCommand)) {
        $BenchCommand = ".\bench\bench_command_parser.exe"
    }
    
    if (Test-Path $BenchCommand) {
        & $BenchCommand --benchmark_format=json | Out-File -FilePath "$FullOutputDir\command_parser_bench.json" -Encoding UTF8
        & $BenchCommand --benchmark_format=console
    } else {
        Write-Warning "bench_command_parser.exe not found at $BenchCommand"
    }

    Write-Host ""
    Write-Host "Running shell_core benchmarks..." -ForegroundColor Cyan
    $BenchShell = ".\bench\$Config\bench_shell_core.exe"
    if (-not (Test-Path $BenchShell)) {
        $BenchShell = ".\bench\bench_shell_core.exe"
    }
    
    if (Test-Path $BenchShell) {
        & $BenchShell --benchmark_format=json | Out-File -FilePath "$FullOutputDir\shell_core_bench.json" -Encoding UTF8
        & $BenchShell --benchmark_format=console
    } else {
        Write-Warning "bench_shell_core.exe not found at $BenchShell"
    }

    Write-Host ""
    Write-Host "✅ Benchmarks complete!" -ForegroundColor Green
    Write-Host "📁 Results saved to: $FullOutputDir" -ForegroundColor Green
    Write-Host "   - command_parser_bench.json" -ForegroundColor Gray
    Write-Host "   - shell_core_bench.json" -ForegroundColor Gray

} finally {
    Pop-Location
}