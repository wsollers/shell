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
    # Essential tools
    ca-certificates=20240203 \
    curl=8.5.0-2ubuntu10.5 \
    wget=1.21.4-1ubuntu4.1 \
    gnupg=2.4.4-2ubuntu17 \
    lsb-release=12.0-2 \
    software-properties-common=0.99.48 \
    # Development tools
    git=1:2.43.0-1ubuntu7.1 \
    vim=2:9.1.0016-1ubuntu7.3 \
    nano=7.2-1build1 \
    tree=2.1.1-2ubuntu3 \
    htop=3.3.0-2build1 \
    # Build essentials
    build-essential=12.10ubuntu1 \
    make=4.3-4.1build2 \
    pkg-config=1.8.1-2build1 \
    && rm -rf /var/lib/apt/lists/*

# Install language runtimes and compilers (occasionally change)
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Clang with C++23 support
    clang-${CLANG_VERSION}=1:18.1.3-1ubuntu1 \
    clang++-${CLANG_VERSION}=1:18.1.3-1ubuntu1 \
    # libc++ for C++23 features like std::expected
    libc++-${CLANG_VERSION}-dev=1:18.1.3-1ubuntu1 \
    libc++abi-${CLANG_VERSION}-dev=1:18.1.3-1ubuntu1 \
    # Development tools
    gdb=15.0.50.20240403-0ubuntu1 \
    valgrind=1:3.22.0-0ubuntu2 \
    strace=6.8-0ubuntu2 \
    # Python for SBOM tools
    python${PYTHON_VERSION}=3.12.3-1ubuntu0.4 \
    python${PYTHON_VERSION}-dev=3.12.3-1ubuntu0.4 \
    python3-pip=24.0+dfsg-1ubuntu1.1 \
    python3-venv=3.12.3-1ubuntu0.4 \
    && rm -rf /var/lib/apt/lists/*

# Install CMake from Kitware's repository for latest version
RUN wget -qO- https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /usr/share/keyrings/kitware-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" > /etc/apt/sources.list.d/kitware.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends cmake=3.30.5-0kitware1ubuntu$(lsb_release -rs)1 \
    && rm -rf /var/lib/apt/lists/*

# Set up compiler alternatives
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${CLANG_VERSION} 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${CLANG_VERSION} 100

# Install Python SBOM dependencies with pinned versions
COPY requirements-dev.txt /tmp/
RUN python3 -m pip install --no-cache-dir --upgrade pip==24.2 \
    && python3 -m pip install --no-cache-dir -r /tmp/requirements-dev.txt \
    && rm /tmp/requirements-dev.txt

# Create non-root user for development
RUN groupadd --gid 1000 vscode \
    && useradd --uid 1000 --gid vscode --shell /bin/bash --create-home vscode \
    && echo 'vscode ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers.d/vscode \
    && chmod 0440 /etc/sudoers.d/vscode

# Set up workspace directory
RUN mkdir -p /workspace && chown vscode:vscode /workspace

# Switch to non-root user
USER vscode
WORKDIR /workspace

# Set environment variables for C++23 development
ENV CC=clang-${CLANG_VERSION}
ENV CXX=clang++-${CLANG_VERSION}
ENV CMAKE_GENERATOR="Unix Makefiles"
ENV PYTHONPATH=/usr/local/lib/python${PYTHON_VERSION}/site-packages:$PYTHONPATH

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