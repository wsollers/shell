[CmdletBinding()]
param(
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
    Write-Host "Clean complete: removed $BuildDir" -ForegroundColor Green
} else {
    Write-Host "Nothing to clean: $BuildDir does not exist" -ForegroundColor Yellow
}
