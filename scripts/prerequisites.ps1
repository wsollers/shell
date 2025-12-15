# SPDX-FileCopyrightText: 2025 wsollers
# SPDX-License-Identifier: MIT
#
# Install prerequisites for building wshell on Windows
# Usage: .\scripts\prerequisites.ps1

#Requires -RunAsAdministrator

$ErrorActionPreference = "Stop"

Write-Host "=== Installing wshell Prerequisites (Windows) ===" -ForegroundColor Green

# Check if Chocolatey is installed
if (!(Get-Command choco -ErrorAction SilentlyContinue)) {
    Write-Host "Installing Chocolatey..." -ForegroundColor Yellow
    Set-ExecutionPolicy Bypass -Scope Process -Force
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
    
    # Refresh environment
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
}

Write-Host "✓ Chocolatey installed" -ForegroundColor Green

# Install build tools
Write-Host "Installing build tools..." -ForegroundColor Yellow

$packages = @(
    "cmake",
    "git",
    "python312",
    "visualstudio2022buildtools",
    "visualstudio2022-workload-vctools"
)

foreach ($package in $packages) {
    Write-Host "Installing $package..." -ForegroundColor Yellow
    choco install $package -y --no-progress
}

# Refresh environment variables
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Verify installations
Write-Host "`nVerifying installations..." -ForegroundColor Green

if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $cmakeVersion = cmake --version | Select-Object -First 1
    Write-Host "✓ cmake: $cmakeVersion" -ForegroundColor Green
} else {
    Write-Host "✗ cmake not found" -ForegroundColor Red
}

if (Get-Command git -ErrorAction SilentlyContinue) {
    $gitVersion = git --version
    Write-Host "✓ git: $gitVersion" -ForegroundColor Green
} else {
    Write-Host "✗ git not found" -ForegroundColor Red
}

if (Get-Command python -ErrorAction SilentlyContinue) {
    $pythonVersion = python --version
    Write-Host "✓ python: $pythonVersion" -ForegroundColor Green
} else {
    Write-Host "✗ python not found" -ForegroundColor Red
}

# Set up Python virtual environment for SBOM tools
Write-Host "`nSetting up Python environment for SBOM tools..." -ForegroundColor Green

$venvDir = ".venv"
if (!(Test-Path $venvDir)) {
    python -m venv $venvDir
    Write-Host "✓ Created Python virtual environment" -ForegroundColor Green
}

# Activate virtual environment and install packages
& "$venvDir\Scripts\Activate.ps1"

Write-Host "Installing Python packages for SBOM and validation tools..." -ForegroundColor Yellow
python -m pip install --upgrade pip
python -m pip install reuse "spdx-tools>=0.8.0" ntia-conformance-checker yamllint

Write-Host "✓ Installed Python tools:" -ForegroundColor Green
$reuseVersion = python -m pip show reuse | Select-String "Version:" | ForEach-Object { $_.ToString().Split()[1] }
$spdxVersion = python -m pip show spdx-tools | Select-String "Version:" | ForEach-Object { $_.ToString().Split()[1] }
$ntiaVersion = python -m pip show ntia-conformance-checker | Select-String "Version:" | ForEach-Object { $_.ToString().Split()[1] }
$yamllintVersion = python -m pip show yamllint | Select-String "Version:" | ForEach-Object { $_.ToString().Split()[1] }

Write-Host "  - reuse: $reuseVersion" -ForegroundColor White
Write-Host "  - spdx-tools: $spdxVersion" -ForegroundColor White
Write-Host "  - ntia-conformance-checker: $ntiaVersion" -ForegroundColor White
Write-Host "  - yamllint: $yamllintVersion" -ForegroundColor White

deactivate

Write-Host "`n=== Prerequisites Installation Complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "To build the project:" -ForegroundColor Yellow
Write-Host "  1. Open a new PowerShell window (to refresh environment)"
Write-Host "  2. Activate Python environment: .\.venv\Scripts\Activate.ps1"
Write-Host "  3. Configure: cmake --preset windows-msvc-release"
Write-Host "  4. Build: cmake --build build\windows-msvc-release --config Release"
Write-Host "  5. Test: cd build\windows-msvc-release; ctest -C Release"
Write-Host "  6. Install (generates SBOM): cmake --install build\windows-msvc-release"
Write-Host ""
Write-Host "SBOM will be generated in the install directory" -ForegroundColor Green
Write-Host ""
Write-Host "Note: You may need to restart your terminal for all changes to take effect." -ForegroundColor Yellow
