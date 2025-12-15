# Quick Start Guide

## Step 1: Install Prerequisites

**Automated installation (recommended):**

### Linux/macOS
```bash
./scripts/prerequisites.sh
```

### Windows (as Administrator)
```powershell
.\scripts\prerequisites.ps1
```

**What gets installed:**
- Clang 18+ (Linux/macOS) or Visual Studio 2022 Build Tools (Windows)
- CMake 3.20+
- Python 3.12+ with virtual environment
- SBOM tools: reuse, spdx-tools, ntia-conformance-checker
- Build essentials and platform-specific libraries

**Manual installation:**
If you prefer manual setup, see [README.md](README.md#prerequisites-installation) for detailed instructions.

## Step 2: Activate Python Environment

### Linux/macOS
```bash
source .venv/bin/activate
```

### Windows
```powershell
.\.venv\Scripts\Activate.ps1
```

## Step 3: Build and Test

### Linux/macOS
```bash
./scripts/configure.sh --release
./scripts/build.sh
./scripts/test.sh
```

### Windows
```powershell
.\scripts\configure.ps1 -Release
.\scripts\build.ps1
.\scripts\test.ps1
```

## Step 4: Generate SBOM (Optional)

Software Bill of Materials (SBOM) for supply chain security:

### Linux/macOS
```bash
cmake --install build/linux-release
```

### Windows
```powershell
cmake --install build\windows-msvc-release
```

This generates:
- `wshell-sbom.spdx` (SPDX tag-value format)
- `wshell-sbom.spdx.json` (JSON format, NTIA-compliant)

## What Happens During Build

1. **Prerequisites Check**: Verifies compiler, CMake, Python environment
2. **Configure**: CMake sets up build with security hardening flags
3. **Auto-fetch**: Downloads Google Test v1.14.0 and cmake-sbom if needed
4. **Build**: Compiles with full security hardening (ASLR, DEP, CFG, etc.)
5. **Test**: Runs 31+ unit tests
6. **SBOM**: Generates software bill of materials (on install)

## Dependencies Downloaded Automatically

The project uses CMake FetchContent to automatically download:
- ✅ **Google Test** v1.14.0 - Unit testing framework
- ✅ **cmake-sbom** - SBOM generation tool
- ✅ Works out of the box
- ✅ No manual dependency installation
- ✅ Consistent test environment

## Run the Executable
```bash
# Linux/macOS
./build/linux-release/src/main/wshell

# Windows
.\build\windows-msvc-release\src\main\Release\wshell.exe
```

## Pre-Push Validation

**Before pushing to GitHub, run all validation steps:**

```bash
# Linux/macOS - Complete validation
./scripts/clean.sh
./scripts/configure.sh --release
./scripts/build.sh
./scripts/test.sh
./scripts/fuzz.sh          # Fuzz testing
./scripts/benchmark.sh     # Performance benchmarks
cmake --install build/linux-release  # SBOM generation
```

```powershell
# Windows - Complete validation
.\scripts\clean.ps1
.\scripts\configure.ps1 -Release
.\scripts\build.ps1
.\scripts\test.ps1
.\scripts\benchmark.ps1
cmake --install build\windows-msvc-release
```

**Why?** This ensures:
- No build failures in CI/CD
- All tests pass
- No performance regressions
- SBOM is valid
- Security checks complete

## Next Steps
- Read [AI_CODING_GUIDELINES.md](.github/AI_CODING_GUIDELINES.md)
- Review [Pre-Push Validation Requirements](.github/AI_CODING_GUIDELINES.md#pre-push-validation-requirements)
- Explore [Configuration System](docs/CONFIG.md)
- Run coverage tests: `./scripts/coverage.sh`
