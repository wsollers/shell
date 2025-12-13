param([string]$BuildDir = "build", [int]$Duration = 60, [string]$CorpusDir = "fuzz_corpus")
New-Item -ItemType Directory -Force -Path "$CorpusDir\command_parser","$CorpusDir\shell_core" | Out-Null
Write-Host "Running fuzz tests..." -ForegroundColor Green
if (Test-Path "$BuildDir\fuzz\fuzz_command_parser.exe") { & "$BuildDir\fuzz\fuzz_command_parser.exe" "$CorpusDir\command_parser" -max_total_time=$Duration }
if (Test-Path "$BuildDir\fuzz\fuzz_shell_core.exe") { & "$BuildDir\fuzz\fuzz_shell_core.exe" "$CorpusDir\shell_core" -max_total_time=$Duration }
Write-Host "Fuzz testing complete!" -ForegroundColor Green
