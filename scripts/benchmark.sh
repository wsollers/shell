#!/usr/bin/env bash
set -euo pipefail
# shellcheck source=scripts/common.sh
source "$(dirname "$0")/common.sh"

PRESET="linux-bench"
BUILD_DIR="build/linux-bench"
OUTPUT_DIR="${ROOT_DIR}/benchmark_results"

echo "🧹 Cleaning previous builds..."
if [[ -d "build" ]]; then
  rm -rf "build"
  echo "Clean complete!"
else
  echo "Nothing to clean."
fi

echo "🔧 Configuring CMake for benchmarks..."
cmake --preset "$PRESET"

echo "🔨 Building benchmark targets..."
cmake --build "$BUILD_DIR" --parallel

echo "📊 Running benchmarks..."
cd "$BUILD_DIR"

# Create output directory for results
mkdir -p "$OUTPUT_DIR"

# Run benchmarks and save results
echo "Running command_parser benchmarks..."
if [[ -f "./bench/bench_command_parser" ]]; then
  ./bench/bench_command_parser --benchmark_format=json > "$OUTPUT_DIR/command_parser_bench.json"
  ./bench/bench_command_parser --benchmark_format=console
else
  echo "Warning: bench_command_parser not found"
fi

echo ""
echo "Running shell_core benchmarks..."
if [[ -f "./bench/bench_shell_core" ]]; then
  ./bench/bench_shell_core --benchmark_format=json > "$OUTPUT_DIR/shell_core_bench.json"
  ./bench/bench_shell_core --benchmark_format=console
else
  echo "Warning: bench_shell_core not found"
fi

echo ""
echo "✅ Benchmarks complete!"
echo "📁 Results saved to: $OUTPUT_DIR"
echo "   - command_parser_bench.json"
echo "   - shell_core_bench.json"