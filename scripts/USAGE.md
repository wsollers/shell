# Build Scripts Usage Guide

This directory contains build automation scripts for the shell project. Scripts are available for both Unix-like systems (Linux/macOS) and Windows.

## Quick Start

```bash
# Linux/macOS - Full build and test cycle
./scripts/clean.sh
./scripts/configure.sh --release
./scripts/build.sh
./scripts/test.sh

# Windows - Full build and test cycle
.\scripts\clean.ps1
.\scripts\configure.ps1 -Release
.\scripts\build.ps1
.\scripts\test.ps1
```

## Script Overview

| Script | Purpose | Platforms |
|--------|---------|-----------|
| `clean` | Remove build directories | Linux/macOS/Windows |
| `configure` | Configure CMake build system | Linux/macOS/Windows |
| `build` | Compile the project | Linux/macOS/Windows |
| `test` | Run unit tests | Linux/macOS/Windows |
| `benchmark` | Run performance benchmarks | Linux/macOS/Windows |
| `coverage` | Generate code coverage reports | Linux/macOS |
| `fuzz` | Run fuzz testing | Linux/macOS/Windows |
| `common.sh` | Shared utilities (sourced by other scripts) | Linux/macOS |

---

## Script Details

### 🧹 Clean Scripts

Remove build directories to ensure clean builds.

**Linux/macOS:**
```bash
./scripts/clean.sh
```

**Windows:**
```powershell
.\scripts\clean.ps1
.\scripts\clean.ps1 -BuildDir "custom_build"
```

### 🔧 Configure Scripts

Set up CMake build system with various presets.

**Linux/macOS:**
```bash
./scripts/configure.sh [OPTION]

Options:
  --release         Release build (default)
  --debug           Debug build with symbols
  --fuzz            Fuzzing build with libFuzzer
  --bench           Benchmark build with Google Benchmark
  --coverage        Coverage build with instrumentation
  --macos-release   macOS release build
  --macos-debug     macOS debug build
  --macos-bench     macOS benchmark build
  --windows-release Windows MSVC release (cross-platform)
  --windows-debug   Windows MSVC debug (cross-platform)
```

**Windows:**
```powershell
.\scripts\configure.ps1 [OPTIONS]

Build modes (choose one):
  -Release          Release build (default)
  -Debug            Debug build with symbols  
  -Test             Test build
  -Coverage         Coverage build
  -Fuzz             Fuzzing build
  -Benchmark        Benchmark build

Additional options:
  -BuildDir DIR     Custom build directory (default: "build")
  -Generator GEN    CMake generator (default: "Visual Studio 17 2022")
  -Arch ARCH        Architecture (default: "x64")
  -Help             Show detailed help
```

### 🔨 Build Scripts

Compile the configured project.

**Linux/macOS:**
```bash
./scripts/build.sh [PRESET]

Examples:
  ./scripts/build.sh                    # Default: linux-release
  ./scripts/build.sh linux-debug        # Debug build
  ./scripts/build.sh linux-bench        # Benchmark build
```

**Windows:**
```powershell
.\scripts\build.ps1 [OPTIONS]

Parameters:
  -BuildDir DIR     Build directory (default: "build")
  -Config CFG       Debug|Release|RelWithDebInfo|MinSizeRel (default: "Release")
  -Jobs N           Parallel jobs (default: auto-detected CPU cores)

Examples:
  .\scripts\build.ps1                           # Default release build
  .\scripts\build.ps1 -Config Debug -Jobs 8    # Debug with 8 parallel jobs
```

### 🧪 Test Scripts

Run unit tests using CTest.

**Linux/macOS:**
```bash
./scripts/test.sh [PRESET]

Examples:
  ./scripts/test.sh                    # Default: linux-release
  ./scripts/test.sh linux-debug        # Test debug build
```

**Windows:**
```powershell
.\scripts\test.ps1 [OPTIONS]

Parameters:
  -BuildDir DIR     Build directory (default: "build")
  -Config CFG       Build configuration (default: "Release")
  -List             List tests without running them

Examples:
  .\scripts\test.ps1                    # Run all tests
  .\scripts\test.ps1 -Config Debug      # Test debug build
  .\scripts\test.ps1 -List              # List available tests
```

### 📊 Benchmark Scripts

Run performance benchmarks and save results.

**Linux/macOS:**
```bash
./scripts/benchmark.sh
```
- Uses `linux-bench` preset
- Saves JSON results to `benchmark_results/` directory
- Shows console output during execution

**Windows:**
```powershell
.\scripts\benchmark.ps1 [OPTIONS]

Parameters:
  -Preset NAME      CMake preset (default: "windows-msvc-release")
  -Config CFG       Build configuration (default: "Release")  
  -OutputDir DIR    Results directory (default: "benchmark_results")

Examples:
  .\scripts\benchmark.ps1                               # Default benchmark
  .\scripts\benchmark.ps1 -Config Debug                 # Debug benchmarks
  .\scripts\benchmark.ps1 -OutputDir "custom_results"   # Custom output location
```

**Benchmark Output:**
- `command_parser_bench.json` - JSON results for command parser benchmarks
- `shell_core_bench.json` - JSON results for shell core benchmarks
- Console output shows real-time performance metrics

### 📈 Coverage Script

Generate code coverage reports (Linux/macOS only).

```bash
./scripts/coverage.sh [OPTIONS]

Options:
  --build-dir DIR   Build directory (default: "build")
  --config CFG      Build configuration for multi-config generators
  --out FILE        Output LCOV file (default: "coverage.info")

Examples:
  ./scripts/coverage.sh                           # Default coverage
  ./scripts/coverage.sh --out detailed.info       # Custom output file
  ./scripts/coverage.sh --build-dir mybuild      # Custom build directory
```

**Requirements:**
- Clang compiler with LLVM source-based coverage
- `llvm-cov` and `llvm-profdata` tools
- Project configured with coverage preset (`--coverage`)

### 🐛 Fuzz Scripts

Run fuzzing tests to find edge cases and vulnerabilities.

**Linux/macOS:**
```bash
./scripts/fuzz.sh <target> [duration_seconds]

Targets:
  fuzz_command_parser   # Fuzz the command parser
  fuzz_shell_core       # Fuzz the shell core

Examples:
  ./scripts/fuzz.sh fuzz_command_parser          # Run for 300 seconds (default)
  ./scripts/fuzz.sh fuzz_shell_core 600         # Run for 10 minutes
```

**Windows:**
```powershell
.\scripts\fuzz.ps1 [OPTIONS]

Parameters:
  -BuildDir DIR     Build directory (default: "build")
  -Config CFG       Build configuration (default: "Release")
  -Duration SEC     Fuzzing duration in seconds (default: 60)
  -CorpusDir DIR    Corpus directory (default: "fuzz_corpus")

Examples:
  .\scripts\fuzz.ps1                           # Default 60-second fuzz
  .\scripts\fuzz.ps1 -Duration 300             # 5-minute fuzz session
  .\scripts\fuzz.ps1 -Config Debug             # Fuzz debug build
```

**Requirements:**
- Project configured with fuzz preset (`--fuzz` or `-Fuzz`)
- Clang compiler with libFuzzer support

---

## Common Workflows

### 🚀 Development Workflow

```bash
# Linux/macOS
./scripts/clean.sh
./scripts/configure.sh --debug
./scripts/build.sh linux-debug
./scripts/test.sh linux-debug

# Windows  
.\scripts\clean.ps1
.\scripts\configure.ps1 -Debug
.\scripts\build.ps1 -Config Debug
.\scripts\test.ps1 -Config Debug
```

### 🏃‍♂️ Performance Analysis

```bash
# Linux/macOS
./scripts/configure.sh --bench
./scripts/build.sh linux-bench
./scripts/benchmark.sh

# Windows
.\scripts\configure.ps1 -Benchmark
.\scripts\build.ps1
.\scripts\benchmark.ps1
```

### 🔍 Security Testing

```bash
# Linux/macOS
./scripts/configure.sh --fuzz
./scripts/build.sh linux-fuzz
./scripts/fuzz.sh fuzz_command_parser 1800    # 30 minutes

# Windows
.\scripts\configure.ps1 -Fuzz
.\scripts\build.ps1
.\scripts\fuzz.ps1 -Duration 1800
```

### 📊 Coverage Analysis

```bash
# Linux/macOS only
./scripts/configure.sh --coverage
./scripts/build.sh linux-coverage
./scripts/coverage.sh
```

## Prerequisites

### Linux/macOS
- CMake 3.25+
- Clang 18+ with libc++
- Make or Ninja build system
- For fuzzing: libFuzzer (included with Clang)
- For coverage: `llvm-cov`, `llvm-profdata`

### Windows
- CMake 3.25+
- Visual Studio 2022 with MSVC
- PowerShell 5.0+
- For fuzzing: Clang with libFuzzer support

## Troubleshooting

### Common Issues

**"Command not found" errors:**
- Ensure scripts have execute permissions: `chmod +x scripts/*.sh`
- On Windows, ensure PowerShell execution policy allows scripts: `Set-ExecutionPolicy RemoteSigned`

**CMake preset not found:**
- Check `CMakePresets.json` exists in project root
- Verify preset name matches available presets: `cmake --list-presets`

**Build failures:**
- Ensure all dependencies are installed
- Try cleaning and reconfiguring: `./scripts/clean.sh && ./scripts/configure.sh`
- Check compiler versions meet requirements

**Test failures:**
- Build the project first: `./scripts/build.sh`
- Ensure test binaries were compiled: check `build/*/test/` directories
- Run tests with verbose output: `ctest -VV`

### Getting Help

**Script-specific help:**
```bash
# Linux/macOS
./scripts/configure.sh --help  # (if implemented)

# Windows
.\scripts\configure.ps1 -Help
```

**Check CMake presets:**
```bash
cmake --list-presets=configure
cmake --list-presets=build
cmake --list-presets=test
```

**Verify build targets:**
```bash
cd build/your-preset
cmake --build . --target help
```