#!/bin/bash

# generate-flamegraphs.sh
# Generates flame graphs from benchmark executables using perf and FlameGraph tools

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." &> /dev/null && pwd)"
BUILD_DIR="${PROJECT_DIR}/build/linux-flamegraph"
OUTPUT_DIR="${PROJECT_DIR}/flamegraphs"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log() {
    echo -e "${GREEN}[$(date +'%H:%M:%S')] $*${NC}" >&2
}

warn() {
    echo -e "${YELLOW}[$(date +'%H:%M:%S')] WARNING: $*${NC}" >&2
}

error() {
    echo -e "${RED}[$(date +'%H:%M:%S')] ERROR: $*${NC}" >&2
}

# Check if running as root or with CAP_SYS_ADMIN (needed for perf)
check_perf_permissions() {
    if [[ $EUID -eq 0 ]]; then
        log "Running as root - perf should work"
        return 0
    fi
    
    # Check if perf_event_paranoid allows user profiling
    if [[ -r /proc/sys/kernel/perf_event_paranoid ]]; then
        local paranoid_level
        paranoid_level=$(cat /proc/sys/kernel/perf_event_paranoid)
        if [[ $paranoid_level -le 1 ]]; then
            log "perf_event_paranoid = $paranoid_level (user profiling allowed)"
            return 0
        else
            warn "perf_event_paranoid = $paranoid_level (may limit profiling)"
            warn "Consider running: echo 1 | sudo tee /proc/sys/kernel/perf_event_paranoid"
        fi
    fi
    
    # Try a simple perf test
    if perf record -e cycles -c 1000000 sleep 0.01 &>/dev/null; then
        log "perf permissions check passed"
        rm -f perf.data
        return 0
    else
        error "perf record test failed - insufficient permissions"
        error "Try: sudo sysctl kernel.perf_event_paranoid=1"
        error "Or run this script with sudo"
        return 1
    fi
}

# Install FlameGraph tools if not available
install_flamegraph_tools() {
    local flamegraph_dir="${PROJECT_DIR}/third_party/FlameGraph"
    
    if [[ -d "$flamegraph_dir" && -x "$flamegraph_dir/flamegraph.pl" ]]; then
        log "FlameGraph tools already available"
        echo "$flamegraph_dir"
        return 0
    fi
    
    log "Installing FlameGraph tools..."
    mkdir -p "${PROJECT_DIR}/third_party"
    
    if command -v git &> /dev/null; then
        git clone https://github.com/brendangregg/FlameGraph.git "$flamegraph_dir"
    else
        error "git not found - cannot install FlameGraph tools"
        return 1
    fi
    
    if [[ -x "$flamegraph_dir/flamegraph.pl" ]]; then
        log "FlameGraph tools installed successfully"
        echo "$flamegraph_dir"
    else
        error "FlameGraph installation failed"
        return 1
    fi
}

# Generate flame graph for a specific benchmark
generate_benchmark_flamegraph() {
    local benchmark_name="$1"
    local benchmark_path="$2"
    local flamegraph_dir="$3"
    
    if [[ ! -x "$benchmark_path" ]]; then
        error "Benchmark executable not found: $benchmark_path"
        return 1
    fi
    
    log "Generating flame graph for $benchmark_name"
    
    # Create output directory
    mkdir -p "$OUTPUT_DIR"
    
    # Perf data file
    local perf_data="${OUTPUT_DIR}/${benchmark_name}.perf.data"
    local folded_file="${OUTPUT_DIR}/${benchmark_name}.folded"
    local flamegraph_file="${OUTPUT_DIR}/${benchmark_name}.svg"
    
    # Record performance data
    log "Recording perf data for $benchmark_name (this may take a few seconds)..."
    perf record -F 997 -g --call-graph=fp -o "$perf_data" -- \
        "$benchmark_path" --benchmark_repetitions=3 --benchmark_min_time=1.0 \
        2>/dev/null || {
        error "perf record failed for $benchmark_name"
        return 1
    }
    
    # Convert perf data to folded format
    log "Processing perf data..."
    perf script -i "$perf_data" | "$flamegraph_dir/stackcollapse-perf.pl" > "$folded_file" || {
        error "Failed to process perf data for $benchmark_name"
        return 1
    }
    
    # Generate flame graph SVG
    log "Generating flame graph SVG..."
    "$flamegraph_dir/flamegraph.pl" \
        --title="$benchmark_name Performance Profile" \
        --width=1200 \
        --height=800 \
        "$folded_file" > "$flamegraph_file" || {
        error "Failed to generate flame graph for $benchmark_name"
        return 1
    }
    
    # Clean up intermediate files
    rm -f "$perf_data" "$folded_file"
    
    log "Flame graph generated: $flamegraph_file"
    
    # Generate metadata
    local metadata_file="${OUTPUT_DIR}/${benchmark_name}.json"
    cat > "$metadata_file" << EOF
{
  "benchmark": "$benchmark_name",
  "timestamp": "$(date -Iseconds)",
  "svg_file": "$(basename "$flamegraph_file")",
  "executable": "$(basename "$benchmark_path")",
  "profiling_frequency": "997 Hz",
  "repetitions": 3,
  "min_time": "1.0s"
}
EOF
    
    return 0
}

# Main execution
main() {
    log "Starting flame graph generation"
    
    # Check if build directory exists
    if [[ ! -d "$BUILD_DIR" ]]; then
        error "Build directory not found: $BUILD_DIR"
        error "Run: cmake --preset linux-flamegraph && cmake --build --preset linux-flamegraph"
        exit 1
    fi
    
    # Check perf permissions
    if ! check_perf_permissions; then
        exit 1
    fi
    
    # Install FlameGraph tools
    local flamegraph_dir
    if ! flamegraph_dir=$(install_flamegraph_tools); then
        exit 1
    fi
    
    # Find benchmark executables
    local benchmarks=()
    if [[ -x "$BUILD_DIR/bench/bench_command_parser" ]]; then
        benchmarks+=("command_parser:$BUILD_DIR/bench/bench_command_parser")
    fi
    if [[ -x "$BUILD_DIR/bench/bench_shell_core" ]]; then
        benchmarks+=("shell_core:$BUILD_DIR/bench/bench_shell_core")
    fi
    
    if [[ ${#benchmarks[@]} -eq 0 ]]; then
        error "No benchmark executables found in $BUILD_DIR/bench/"
        error "Build benchmarks first: cmake --build --preset linux-flamegraph"
        exit 1
    fi
    
    log "Found ${#benchmarks[@]} benchmark(s)"
    
    # Generate flame graphs
    local success_count=0
    local total_count=${#benchmarks[@]}
    for benchmark in "${benchmarks[@]}"; do
        IFS=':' read -r name path <<< "$benchmark"
        if generate_benchmark_flamegraph "$name" "$path" "$flamegraph_dir"; then
            ((success_count++))
        else
            warn "Failed to generate flame graph for $name"
        fi
    done
    
    log "Generated $success_count/$total_count flame graph(s) in $OUTPUT_DIR"
    
    if [[ $success_count -eq 0 ]]; then
        error "No flame graphs were generated successfully"
        exit 1
    fi
    
    # Create index HTML
    local index_file="${OUTPUT_DIR}/index.html"
    cat > "$index_file" << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Shell Project - Flame Graphs</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif; margin: 2rem; }
        .container { max-width: 1200px; margin: 0 auto; }
        .flamegraph { margin: 2rem 0; padding: 1rem; border: 1px solid #ddd; border-radius: 8px; }
        .flamegraph h2 { color: #333; margin-top: 0; }
        .svg-container { border: 1px solid #eee; border-radius: 4px; overflow: hidden; }
        .metadata { background: #f5f5f5; padding: 1rem; margin: 1rem 0; border-radius: 4px; }
        .metadata pre { margin: 0; font-size: 0.9em; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔥 Shell Project Flame Graphs</h1>
        <p>Performance profiling visualizations generated from benchmark runs.</p>
EOF
    
    # Add flame graphs to index
    for benchmark in "${benchmarks[@]}"; do
        IFS=':' read -r name path <<< "$benchmark"
        local svg_file="${name}.svg"
        local metadata_file="${name}.json"
        
        if [[ -f "$OUTPUT_DIR/$svg_file" ]]; then
            cat >> "$index_file" << EOF
        <div class="flamegraph">
            <h2>📊 ${name^} Performance Profile</h2>
            <div class="svg-container">
                <object data="$svg_file" type="image/svg+xml" width="100%"></object>
            </div>
EOF
            
            if [[ -f "$OUTPUT_DIR/$metadata_file" ]]; then
                cat >> "$index_file" << EOF
            <div class="metadata">
                <strong>Profiling Details:</strong>
                <pre>$(cat "$OUTPUT_DIR/$metadata_file")</pre>
            </div>
EOF
            fi
            
            cat >> "$index_file" << EOF
        </div>
EOF
        fi
    done
    
    cat >> "$index_file" << 'EOF'
    </div>
</body>
</html>
EOF
    
    log "Index page created: $index_file"
    log "🔥 Flame graph generation complete!"
    log "View results: file://$index_file"
    
    return 0
}

# Run main function
main "$@"