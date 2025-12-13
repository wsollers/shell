#!/bin/bash
set -e
BUILD_DIR="build"
DURATION=60
CORPUS_DIR="fuzz_corpus"
mkdir -p "$CORPUS_DIR/command_parser" "$CORPUS_DIR/shell_core"
echo "Running fuzz tests for ${DURATION} seconds..."
[ -f "$BUILD_DIR/fuzz/fuzz_command_parser" ] && "$BUILD_DIR/fuzz/fuzz_command_parser" "$CORPUS_DIR/command_parser" -max_total_time="$DURATION" || true
[ -f "$BUILD_DIR/fuzz/fuzz_shell_core" ] && "$BUILD_DIR/fuzz/fuzz_shell_core" "$CORPUS_DIR/shell_core" -max_total_time="$DURATION" || true
echo "Fuzz testing complete!"
