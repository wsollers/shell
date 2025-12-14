[CmdletBinding()]
param(
    [string]$BuildDir = "build",
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "Release",
    [int]$Jobs = 0
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $BuildDir)) {
    throw "Build directory not found: $BuildDir. Run .\scripts\configure.ps1 first."
}

if ($Jobs -le 0) {
    try { $Jobs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors }
    catch { $Jobs = 4 }
}

Write-Host "Building wshell..." -ForegroundColor Green
Write-Host "BuildDir: $BuildDir"
Write-Host "Config:   $Config"
Write-Host "Jobs:     $Jobs"

# Multi-config generators (VS) need --config; it is harmless for single-config too.
& cmake --build $BuildDir --config $Config --parallel $Jobs
if ($LASTEXITCODE -ne 0) { throw "Build failed with exit code $LASTEXITCODE" }

Write-Host "Build complete!" -ForegroundColor Green
