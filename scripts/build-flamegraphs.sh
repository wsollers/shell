#!/bin/bash

# build-flamegraphs.sh
# Quick build script for flame graph generation

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." &> /dev/null && pwd)"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

log() {
    echo -e "${GREEN}[$(date +'%H:%M:%S')] $*${NC}"
}

info() {
    echo -e "${BLUE}[INFO] $*${NC}"
}

main() {
    cd "$PROJECT_DIR"
    
    log "Building Shell project with flame graph support..."
    
    # Configure
    info "Configuring CMake..."
    cmake --preset linux-flamegraph
    
    # Build
    info "Building benchmarks..."
    cmake --build --preset linux-flamegraph --parallel
    
    # Generate flame graphs
    info "Generating flame graphs..."
    ./scripts/generate-flamegraphs.sh
    
    log "🔥 Build and flame graph generation complete!"
    log "View results: file://${PROJECT_DIR}/flamegraphs/index.html"
}

main "$@"