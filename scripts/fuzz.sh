#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/common.sh"

TARGET="${1:-}"
DURATION="${2:-300}"

[[ -n "$TARGET" ]] || die "usage: fuzz.sh <fuzzer_target> [duration_seconds]"

BIN="$ROOT_DIR/build/linux-fuzz/fuzz/$TARGET"
CORPUS="$ROOT_DIR/fuzz_corpus/$TARGET"

mkdir -p "$CORPUS"
test -x "$BIN" || die "missing fuzzer binary: $BIN (did you run ./scripts/configure.sh --fuzz && ./scripts/build.sh linux-fuzz?)"

timeout "$DURATION" "$BIN" "$CORPUS"   -max_total_time="$DURATION"   -print_final_stats=1 || true
