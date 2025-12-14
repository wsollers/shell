[CmdletBinding()]
param(
    [string]$BuildDir = "build",
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "Release",

    # Optional: only list tests (like `ctest -N`)
    [switch]$List
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $BuildDir)) {
    throw "Build directory not found: $BuildDir. Run configure/build first."
}

Push-Location $BuildDir
try {
    Write-Host "Discovered tests:" -ForegroundColor Yellow
    & ctest -N -C $Config
    if ($LASTEXITCODE -ne 0) { throw "ctest -N failed with exit code $LASTEXITCODE" }

    if ($List) { return }

    Write-Host "Executing tests (ctest -VV)..." -ForegroundColor Green
    & ctest --output-on-failure -VV -C $Config
    $rc = $LASTEXITCODE
    Write-Host "ctest exit code: $rc"
    if ($rc -ne 0) { exit $rc }

    Write-Host "All tests passed." -ForegroundColor Green
}
finally {
    Pop-Location
}
