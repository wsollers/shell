#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2025 wsollers
# SPDX-License-Identifier: MIT
#
# Install prerequisites for building wshell
# Usage: ./scripts/prerequisites.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Installing wshell Prerequisites ===${NC}"

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
else
    echo -e "${RED}Unsupported OS: $OSTYPE${NC}"
    exit 1
fi

# Check if running as root (not recommended)
if [[ $EUID -eq 0 ]]; then
    echo -e "${YELLOW}Warning: Running as root. Consider running as normal user.${NC}"
fi

echo -e "${GREEN}Detected OS: $OS${NC}"

# Install system packages
if [[ "$OS" == "linux" ]]; then
    echo -e "${GREEN}Installing Linux dependencies...${NC}"
    
    # Detect package manager
    if command -v apt-get &> /dev/null; then
        echo "Using apt-get..."
        sudo apt-get update
        
        # Install LLVM/Clang 18
        if ! command -v clang-18 &> /dev/null; then
            echo "Installing Clang 18..."
            wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
            CODENAME=$(lsb_release -cs)
            echo "deb https://apt.llvm.org/${CODENAME}/ llvm-toolchain-${CODENAME}-18 main" | sudo tee /etc/apt/sources.list.d/llvm.list
            sudo apt-get update
        fi
        
        sudo apt-get install -y \
            clang-18 \
            clang++-18 \
            clang-tidy-18 \
            libc++-18-dev \
            libc++abi-18-dev \
            cmake \
            ninja-build \
            git \
            lcov \
            python3 \
            python3-pip \
            python3-venv \
            build-essential
        
        # Set up alternatives
        sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100
        sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100
        
    elif command -v dnf &> /dev/null; then
        echo "Using dnf..."
        sudo dnf install -y \
            clang \
            cmake \
            ninja-build \
            git \
            python3 \
            python3-pip \
            libcxx-devel
    else
        echo -e "${RED}Unsupported Linux distribution. Please install dependencies manually.${NC}"
        exit 1
    fi
    
elif [[ "$OS" == "macos" ]]; then
    echo -e "${GREEN}Installing macOS dependencies...${NC}"
    
    # Check for Homebrew
    if ! command -v brew &> /dev/null; then
        echo -e "${RED}Homebrew not found. Install from https://brew.sh${NC}"
        exit 1
    fi
    
    brew install \
        llvm \
        cmake \
        ninja \
        git \
        python@3.12
fi

# Verify installations
echo -e "${GREEN}Verifying installations...${NC}"

if command -v clang++ &> /dev/null; then
    echo -e "${GREEN}✓ clang++: $(clang++ --version | head -1)${NC}"
else
    echo -e "${RED}✗ clang++ not found${NC}"
fi

if command -v cmake &> /dev/null; then
    echo -e "${GREEN}✓ cmake: $(cmake --version | head -1)${NC}"
else
    echo -e "${RED}✗ cmake not found${NC}"
fi

if command -v python3 &> /dev/null; then
    echo -e "${GREEN}✓ python3: $(python3 --version)${NC}"
else
    echo -e "${RED}✗ python3 not found${NC}"
fi

# Set up Python virtual environment for SBOM tools
echo -e "${GREEN}Setting up Python environment for SBOM tools...${NC}"

VENV_DIR=".venv"
if [[ ! -d "$VENV_DIR" ]]; then
    python3 -m venv "$VENV_DIR"
    echo -e "${GREEN}✓ Created Python virtual environment${NC}"
fi

# Activate virtual environment and install packages
source "$VENV_DIR/bin/activate"

echo "Installing Python packages for SBOM and validation tools..."
pip install --upgrade pip
pip install \
    reuse \
    "spdx-tools>=0.8.0" \
    ntia-conformance-checker \
    yamllint

echo -e "${GREEN}✓ Installed Python tools:${NC}"
echo "  - reuse: $(pip show reuse | grep Version | cut -d' ' -f2)"
echo "  - spdx-tools: $(pip show spdx-tools | grep Version | cut -d' ' -f2)"
echo "  - ntia-conformance-checker: $(pip show ntia-conformance-checker | grep Version | cut -d' ' -f2)"
echo "  - yamllint: $(pip show yamllint | grep Version | cut -d' ' -f2)"

deactivate

echo ""
echo -e "${GREEN}=== Prerequisites Installation Complete ===${NC}"
echo ""
echo "To build the project:"
echo "  1. Activate the Python environment: source .venv/bin/activate"
echo "  2. Configure: cmake --preset linux-release (or macos-release)"
echo "  3. Build: cmake --build build/linux-release"
echo "  4. Test: cd build/linux-release && ctest"
echo "  5. Install (generates SBOM): cmake --install build/linux-release"
echo ""
echo -e "${GREEN}SBOM will be generated at:${NC}"
echo "  - Tag-value: build/linux-release/_deps/cmake-sbom-build/example/deploy/wshell-sbom.spdx"
echo "  - JSON: build/linux-release/_deps/cmake-sbom-build/example/deploy/wshell-sbom.spdx.json"
