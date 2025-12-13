param([string]$BuildDir = "build", [int]$Jobs = 0)
if (-not (Test-Path $BuildDir)) { Write-Host "Build directory not found." -ForegroundColor Red; exit 1 }
if ($Jobs -eq 0) { $Jobs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors }
Write-Host "Building wshell..." -ForegroundColor Green
cmake --build $BuildDir --parallel $Jobs
if ($LASTEXITCODE -eq 0) { Write-Host "Build complete!" -ForegroundColor Green } else { exit 1 }
