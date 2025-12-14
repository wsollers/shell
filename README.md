# wshell - A Secure Command Shell Interpreter

[![Build Status](https://github.com/wsollers/shell/workflows/CI/badge.svg)](https://github.com/wsollers/shell/actions)
[![codecov](https://codecov.io/gh/wsollers/shell/branch/main/graph/badge.svg)](https://codecov.io/gh/wsollers/shell)
[![License](https://img.shields.io/badge/License-BSD_2--Clause-blue.svg)](https://opensource.org/licenses/BSD-2-Clause)

A modern, secure command shell interpreter written in C++23 with a focus on security, performance, and code quality.

## 📊 Performance Dashboard

View live performance metrics, coverage reports, and API documentation at:
**[wsollers.github.io/shell](https://wsollers.github.io/shell/)**

The dashboard includes:
- 📈 **Real-time Benchmarks**: Performance tracking with interactive charts
- 🛡️ **Coverage Reports**: Line/branch coverage with detailed analysis
- 📖 **API Documentation**: Auto-generated from source code
- 🔍 **Security Analysis**: Fuzzing results and vulnerability scans

## Features

- **Modern C++23**: Leverages latest C++ features including ranges, concepts, and `std::expected`
- **Best Compilers**: Clang 16+ on Linux/macOS, MSVC 2022+ on Windows
- **Security First**: Built with comprehensive security measures following CWE, OWASP, and NIST guidelines
- **Thoroughly Tested**: Unit tests, fuzz tests, and benchmarks
- **Cross-Platform**: Supports Linux, macOS, and Windows
- **Well Documented**: Doxygen API documentation and comprehensive guides

## Quick Start

### Prerequisites

- CMake 3.25 or higher
- **Compiler Requirements:**
  - **Linux/macOS**: Clang 16+ (automatically configured)
  - **Windows**: MSVC 2022+ (Visual Studio 2022)
- Git (for fetching Google Test automatically)

**Note**: Google Test will be downloaded automatically if not installed!

**Installing Clang on Linux/macOS:**
```bash
# Ubuntu/Debian
sudo apt-get install clang

# macOS
brew install llvm
```

### Building

**Linux/macOS:**
```bash
./scripts/configure.sh --release
./scripts/build.sh
./scripts/test.sh
```

**Windows (PowerShell):**
```powershell
.\scripts\configure.ps1 -Release
.\scripts\build.ps1
.\scripts\test.ps1
```

### Build Types

- `--release`: Optimized production build
- `--debug`: Debug build with symbols
- `--test`: Release with debug info and sanitizers enabled
- `--coverage`: Debug build with code coverage instrumentation

## Documentation

- [API Documentation](https://wsollers.github.io/shell/)
- [AI Coding Guidelines](.github/AI_CODING_GUIDELINES.md)
- [Quick Start Guide](QUICKSTART.md)

## Project Structure

```
shell/
├── src/
│   ├── lib/          # Core library (wshell.so/dll)
│   └── main/         # Main executable (wshell)
├── test/             # Unit tests
├── fuzz/             # Fuzz tests
├── scripts/          # Build utility scripts
├── .github/          # GitHub workflows and configs
└── docs/             # Documentation
```

## Security

Security is a top priority. We follow:
- CWE (Common Weakness Enumeration) guidelines
- OWASP Application Security Verification Standard
- NIST Secure Software Development Framework
- Comprehensive compiler hardening flags

## 🚀 Deployment & CI/CD

The project includes comprehensive GitHub Actions workflows:
- **Continuous Integration**: Cross-platform builds (Linux, Windows, macOS)
- **Security Scanning**: CodeQL, Microsoft DevSkim, Defender for DevOps, Trivy
- **Performance Dashboard**: Automatic deployment to GitHub Pages
- **Coverage Tracking**: Integration with Codecov.io

**Set up GitHub Pages dashboard:**
```bash
./scripts/setup-github-pages.sh
```

See [docs/DASHBOARD.md](docs/DASHBOARD.md) for detailed setup instructions.

## Contributing

Contributions are welcome! Please read the [AI Coding Guidelines](.github/AI_CODING_GUIDELINES.md) before submitting pull requests.

## License

Copyright (c) 2024 William Sollers

This project is licensed under the BSD 2-Clause License - see the [LICENSE](LICENSE) file for details.
