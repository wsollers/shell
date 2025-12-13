# Quick Start Guide

## Prerequisites

**Compiler requirements (platform-specific):**

**Linux/macOS - Clang:**
```bash
# Ubuntu/Debian
sudo apt-get install clang

# macOS (via Homebrew)
brew install llvm
```

**Windows - MSVC:**
```powershell
# Install Visual Studio 2022 with C++ workload
# Download from: https://visualstudio.microsoft.com/
```

## Extract and Navigate
```bash
unzip shell-repo.zip
cd shell-repo
```

## Build and Test

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

## What Happens During Build

1. **Configure**: CMake checks for dependencies
2. **Auto-fetch**: If Google Test isn't installed, it downloads v1.14.0 automatically
3. **Build**: Compiles the library and executable with full security hardening
4. **Test**: Runs 22 unit tests

## No Installation Required

The project uses CMake FetchContent to automatically download Google Test if it's not found on your system. This means:
- ✅ Works out of the box
- ✅ No manual dependency installation
- ✅ Consistent test environment

## Run the Executable
```bash
./build/src/main/wshell echo "Hello, World!"
```

## Next Steps
- Read [AI_CODING_GUIDELINES.md](.github/AI_CODING_GUIDELINES.md)
- Add shell functionality
- Run fuzz tests: `./scripts/configure.sh --fuzz && ./scripts/build.sh && ./scripts/fuzz.sh`
