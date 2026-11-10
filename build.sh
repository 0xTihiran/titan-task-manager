#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cmake -S "$SCRIPT_DIR" -B "$SCRIPT_DIR/build" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$SCRIPT_DIR/build" -j"$(nproc)"
echo "Built $SCRIPT_DIR/build/titan-task-manager"
