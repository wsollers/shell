# Development Container

This directory contains the development container configuration for the C++23 Shell project.

## 🐳 What's Included

- **Ubuntu 24.04** - Stable LTS base
- **Clang 18** - Latest C++23 support with std::expected
- **CMake 3.30+** - Modern build system
- **Python 3.12** - For SBOM tools and development scripts
- **libc++** - C++ standard library with C++23 features
- **Development tools** - GDB, Valgrind, Git, Vim

## 📦 Pinned Dependencies

All dependencies are pinned to specific versions for reproducibility:
- Ubuntu: 24.04
- Clang: 18.1.3
- CMake: 3.30.5 (from Kitware)
- Python: 3.12.3
- SBOM tools: See `requirements-dev.txt`

## 🚀 Quick Start

1. **Open in VS Code**: Install the "Dev Containers" extension
2. **Reopen in Container**: Command Palette → "Dev Containers: Reopen in Container"
3. **Wait for build**: First time takes ~5-10 minutes
4. **Start developing**: All tools are pre-configured

## 🔧 Pre-configured Commands

The container includes helpful aliases:
```bash
cmake-configure  # Configure with linux-debug preset
cmake-build      # Build the project
cmake-test       # Run tests
```

## 📝 Adding Dependencies

To add new dependencies:

1. **System packages**: Update `Dockerfile` with pinned versions
2. **Python packages**: Update `requirements-dev.txt` with pinned versions
3. **Rebuild container**: Command Palette → "Dev Containers: Rebuild Container"

## 🔍 Environment Variables

- `CC=clang-18`
- `CXX=clang++-18`
- `CMAKE_GENERATOR=Unix Makefiles`

## 📋 VS Code Extensions

Pre-installed extensions:
- C/C++ Tools
- CMake Tools
- Python
- GitHub Copilot
- Hex Editor

## 🎯 Project Structure

The container mounts your workspace to `/workspace` and sets up the environment for:
- C++23 compilation with Clang 18
- CMake build system with presets
- Python-based SBOM generation
- Full testing and benchmarking capabilities