param([string]$BuildDir = "build")
if (-not (Test-Path $BuildDir)) { Write-Host "Build directory not found." -ForegroundColor Red; exit 1 }
Write-Host "Running tests..." -ForegroundColor Green
Push-Location $BuildDir; ctest --output-on-failure --verbose; $r = $LASTEXITCODE; Pop-Location
if ($r -eq 0) { Write-Host "All tests passed!" -ForegroundColor Green } else { exit 1 }
