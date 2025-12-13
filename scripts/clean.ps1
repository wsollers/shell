param([string]$BuildDir = "build")
if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir; Write-Host "Clean complete!" -ForegroundColor Green }
