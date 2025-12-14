#!/usr/bin/env bash
set -euo pipefail

# shellcheck disable=SC2034  # ROOT_DIR is used by scripts that source this file
export ROOT_DIR
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

die() {
  echo "error: $*" >&2
  exit 1
}
