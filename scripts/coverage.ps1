# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

[CmdletBinding()]
param(
    [string]$BuildDir = "build",
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "Debug",
    [string]$OutFile = "coverage.info",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Usage {
@"
Usage: .\scripts\coverage.ps1 [OPTIONS]

Options:
  -BuildDir DIR      Build directory (default: build)
  -Config CFG        Build configuration (default: Debug)
  -OutFile FILE      Output coverage file (default: coverage.info)
  -Help              Show this help

Notes:
- Coverage collection on Windows is limited compared to Linux/Clang
- This script attempts to use available coverage tools
- For comprehensive coverage analysis, consider using Linux build

Examples:
  .\scripts\coverage.ps1                           # Default coverage
  .\scripts\coverage.ps1 -Config Debug             # Debug configuration
  .\scripts\coverage.ps1 -OutFile detailed.info    # Custom output file
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

if (-not (Test-Path $BuildDir)) {
    throw "Build directory not found: $BuildDir. Run configure/build first."
}

Push-Location $BuildDir
try {
    if (-not (Test-Path "CMakeCache.txt")) {
        throw "CMakeCache.txt not found in $BuildDir; run configure first."
    }

    # Check if coverage is enabled
    $cacheContent = Get-Content "CMakeCache.txt" -Raw
    if ($cacheContent -notmatch "ENABLE_COVERAGE:BOOL=ON") {
        Write-Warning "ENABLE_COVERAGE appears to be OFF. Reconfigure with:"
        Write-Host "  .\scripts\configure.ps1 -Coverage -BuildDir $BuildDir" -ForegroundColor Yellow
        Write-Host "Continuing anyway..." -ForegroundColor Gray
    }

    Write-Host "Generating coverage report..." -ForegroundColor Green
    Write-Host "BuildDir: $BuildDir" -ForegroundColor Gray
    Write-Host "Config:   $Config" -ForegroundColor Gray  
    Write-Host "OutFile:  $OutFile" -ForegroundColor Gray

    # Run tests first to generate coverage data
    Write-Host "Running tests to generate coverage data..." -ForegroundColor Yellow
    & ctest --output-on-failure -C $Config
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Some tests failed, but continuing with coverage generation..."
    }

    # Windows coverage support is limited, but try common approaches
    $coverageGenerated = $false

    # Try Visual Studio coverage tools (if available)
    $vsTestConsole = Get-Command "vstest.console.exe" -ErrorAction SilentlyContinue
    if ($vsTestConsole) {
        Write-Host "Attempting Visual Studio code coverage..." -ForegroundColor Yellow
        try {
            # This is a basic attempt - VS coverage tools have complex setup
            Write-Warning "Visual Studio coverage tools require additional setup."
            Write-Host "Consider using OpenCppCoverage or similar tools for Windows coverage." -ForegroundColor Yellow
        } catch {
            Write-Warning "Visual Studio coverage failed: $_"
        }
    }

    # Try OpenCppCoverage if available
    $openCppCov = Get-Command "OpenCppCoverage.exe" -ErrorAction SilentlyContinue
    if ($openCppCov) {
        Write-Host "Using OpenCppCoverage..." -ForegroundColor Green
        try {
            # Find test executable
            $testExe = Get-ChildItem -Path . -Filter "*test*.exe" -Recurse | Select-Object -First 1
            if ($testExe) {
                & OpenCppCoverage.exe --export_type cobertura:coverage.xml --sources src -- $testExe.FullName
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "OpenCppCoverage completed successfully" -ForegroundColor Green
                    $coverageGenerated = $true
                    
                    # Convert to lcov format if possible (basic conversion)
                    if (Test-Path "coverage.xml") {
                        # This would need a proper XML to LCOV converter
                        # For now, just rename the file
                        Copy-Item "coverage.xml" $OutFile -Force
                    }
                }
            } else {
                Write-Warning "No test executable found for OpenCppCoverage"
            }
        } catch {
            Write-Warning "OpenCppCoverage failed: $_"
        }
    }

    # Try LLVM coverage if clang-cl was used
    $llvmProf = Get-Command "llvm-profdata.exe" -ErrorAction SilentlyContinue
    if ($llvmProf -and (Test-Path "*.profraw")) {
        Write-Host "Found LLVM coverage data, processing..." -ForegroundColor Green
        try {
            & llvm-profdata.exe merge -sparse *.profraw -o coverage.profdata
            if ($LASTEXITCODE -eq 0) {
                # Find the main executable for coverage
                $mainExe = Get-ChildItem -Path . -Filter "*.exe" -Recurse | Where-Object { $_.Name -notmatch "test" } | Select-Object -First 1
                if ($mainExe) {
                    & llvm-cov.exe export --format=lcov --instr-profile=coverage.profdata $mainExe.FullName > $OutFile
                    if ($LASTEXITCODE -eq 0) {
                        Write-Host "LLVM coverage generated successfully" -ForegroundColor Green
                        $coverageGenerated = $true
                    }
                }
            }
        } catch {
            Write-Warning "LLVM coverage processing failed: $_"
        }
    }

    if (-not $coverageGenerated) {
        Write-Warning "No suitable coverage tools found or coverage generation failed."
        Write-Host @"
For Windows coverage, consider installing:
1. OpenCppCoverage: https://github.com/OpenCppCoverage/OpenCppCoverage
2. Use Clang with LLVM coverage tools
3. Use Visual Studio with Code Coverage tools
4. For best coverage support, consider using Linux build

Creating empty coverage file...
"@ -ForegroundColor Yellow
        
        # Create empty coverage file
        "" | Out-File -FilePath $OutFile -Encoding UTF8
    }

    Write-Host "Coverage processing complete!" -ForegroundColor Green
    Write-Host "Output file: $OutFile" -ForegroundColor Green

} finally {
    Pop-Location
}