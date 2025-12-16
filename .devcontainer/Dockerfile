# C++23 Shell Development Environment
# Based on Ubuntu with Clang 18, CMake 3.25+, and Python 3.12
# Updated 2024-12-15: Initial devcontainer setup

ARG UBUNTU_VERSION=24.04
ARG CLANG_VERSION=18
ARG CMAKE_VERSION=3.25
ARG PYTHON_VERSION=3.12

FROM ubuntu:${UBUNTU_VERSION}

# Install system dependencies in order of change frequency
# Base system packages (rarely change)
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Essential tools (use latest stable versions)
    ca-certificates \
    curl \
    wget \
    gnupg \
    lsb-release \
    software-properties-common \
    # Development tools (use latest stable versions)
    git \
    vim \
    nano \
    tree \
    htop \
    # Build essentials (use latest stable versions)
    build-essential \
    make \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Install language runtimes and compilers (occasionally change)
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Clang with C++23 support + fuzzing/sanitizer runtimes
    clang-18 \
    clang++-18 \
    clang-tools-18 \
    # libc++ for C++23 features like std::expected  
    libc++-18-dev \
    libc++abi-18-dev \
    # Clang runtime libraries for fuzzing and sanitizers
    libclang-rt-18-dev \
    # Development tools (use latest stable versions)
    gdb \
    valgrind \
    strace \
    # Python for SBOM tools
    python3.12 \
    python3.12-dev \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# Install CMake from Kitware's repository for latest version
RUN wget -qO- https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /usr/share/keyrings/kitware-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" > /etc/apt/sources.list.d/kitware.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends cmake \
    && rm -rf /var/lib/apt/lists/*

# Set up compiler alternatives
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100

# Create Python virtual environment and install packages
COPY requirements-dev.txt /tmp/
RUN python3 -m venv /opt/venv \
    && /opt/venv/bin/pip install --no-cache-dir -r /tmp/requirements-dev.txt \
    && rm /tmp/requirements-dev.txt

# Add virtual environment to PATH
ENV PATH="/opt/venv/bin:$PATH"

# Create non-root user for development
# Use UID/GID 1001 to avoid conflict with Ubuntu's default ubuntu user (1000)
RUN groupadd --gid 1001 vscode \
    && useradd --uid 1001 --gid 1001 --shell /bin/bash --create-home vscode \
    && mkdir -p /etc/sudoers.d \
    && echo 'vscode ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers.d/vscode \
    && chmod 0440 /etc/sudoers.d/vscode

# Set up workspace directory
RUN mkdir -p /workspace && chown vscode:vscode /workspace

# Switch to non-root user
USER vscode
WORKDIR /workspace

# Set environment variables for C++23 development
ENV CC=clang-18
ENV CXX=clang++-18
ENV CMAKE_GENERATOR="Unix Makefiles"

# Add helpful aliases
RUN echo 'alias ll="ls -alF"' >> ~/.bashrc \
    && echo 'alias la="ls -A"' >> ~/.bashrc \
    && echo 'alias l="ls -CF"' >> ~/.bashrc \
    && echo 'alias cmake-configure="cmake --preset linux-debug"' >> ~/.bashrc \
    && echo 'alias cmake-build="cmake --build --preset linux-debug"' >> ~/.bashrc \
    && echo 'alias cmake-test="cd build/linux-debug && ctest --output-on-failure"' >> ~/.bashrc

# Print versions for verification
RUN echo "=== Development Environment Versions ===" \
    && echo "Ubuntu: $(lsb_release -d | cut -f2)" \
    && echo "Clang: $(clang++ --version | head -1)" \
    && echo "CMake: $(cmake --version | head -1)" \
    && echo "Python: $(python3 --version)" \
    && echo "Git: $(git --version)"