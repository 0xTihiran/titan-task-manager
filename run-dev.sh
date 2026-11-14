#!/usr/bin/env bash
# Launch from the source checkout. Does not install or modify shared OS files.
set -euo pipefail
TASK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${TASK_DIR}/../.." && pwd)"

if [[ -f "${REPO_DIR}/titan-ui/build/qml/ArchTitan/TitanUI/qmldir" ]]; then
    export ARCHTITAN_QML_IMPORT_PATH="${REPO_DIR}/titan-ui/build/qml"
    export LD_LIBRARY_PATH="${REPO_DIR}/titan-ui/build:${LD_LIBRARY_PATH:-}"
fi

if [[ ! -x "${TASK_DIR}/build/titan-task-manager" ]]; then
    echo 'Build Task Manager first: ./build.sh' >&2
    exit 1
fi
exec "${TASK_DIR}/build/titan-task-manager" "$@"
