[CmdletBinding()]
param(
    [string]$BuildDir = "build",
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "Release",

    [int]$Duration = 60,
    [string]$CorpusDir = "fuzz_corpus"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $BuildDir)) {
    throw "Build directory not found: $BuildDir. Run configure/build first."
}

New-Item -ItemType Directory -Force -Path (Join-Path $CorpusDir "command_parser") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $CorpusDir "shell_core")     | Out-Null

Write-Host "Running fuzz tests..." -ForegroundColor Green
Write-Host "BuildDir:   $BuildDir"
Write-Host "Config:     $Config"
Write-Host "Duration:   $Duration"
Write-Host "CorpusDir:  $CorpusDir"

# Search for fuzz executables in a generator-agnostic way (VS puts them under Config subdirs)
function Find-Exe([string]$name) {
    $hits = Get-ChildItem -Path $BuildDir -Recurse -File -Filter $name -ErrorAction SilentlyContinue
    if (-not $hits) { return $null }

    # Prefer the one under the selected config if present
    $preferred = $hits | Where-Object { $_.FullName -match "\\$Config\\" } | Select-Object -First 1
    if ($preferred) { return $preferred.FullName }

    return ($hits | Select-Object -First 1).FullName
}

$fuzz1 = Find-Exe "fuzz_command_parser.exe"
$fuzz2 = Find-Exe "fuzz_shell_core.exe"

if (-not $fuzz1 -and -not $fuzz2) {
    throw "No fuzz executables found under $BuildDir. Did you configure with -Fuzz and build?"
}

if ($fuzz1) {
    Write-Host "Running: $fuzz1" -ForegroundColor Yellow
    & $fuzz1 (Join-Path $CorpusDir "command_parser") "-max_total_time=$Duration"
    if ($LASTEXITCODE -ne 0) { throw "fuzz_command_parser failed with exit code $LASTEXITCODE" }
} else {
    Write-Host "Skipping fuzz_command_parser.exe (not found)" -ForegroundColor Yellow
}

if ($fuzz2) {
    Write-Host "Running: $fuzz2" -ForegroundColor Yellow
    & $fuzz2 (Join-Path $CorpusDir "shell_core") "-max_total_time=$Duration"
    if ($LASTEXITCODE -ne 0) { throw "fuzz_shell_core failed with exit code $LASTEXITCODE" }
} else {
    Write-Host "Skipping fuzz_shell_core.exe (not found)" -ForegroundColor Yellow
}

Write-Host "Fuzz testing complete!" -ForegroundColor Green
