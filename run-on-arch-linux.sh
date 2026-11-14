#!/usr/bin/env bash
# Build, test, preview and optionally install a user-level desktop launcher
# for TITAN Task Manager on ordinary Arch Linux. Does not alter ArchTitan OS.
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./run-on-arch-linux.sh [PROJECT_DIRECTORY] [run|build|test|demo|install-desktop]

  PROJECT_DIRECTORY  Directory containing TITAN Task Manager's CMakeLists.txt.
                     Defaults to the current directory (or ./subsystems/titan-task-manager).
  run                Build and launch normally (default).
  build              Build only.
  test               Build, then run CTest.
  demo               Build and launch with packaged fake THM telemetry.
  install-desktop    Build and add launcher for this user (no sudo).

Examples:
  ./run-on-arch-linux.sh ~/dev/Titan/subsystems/titan-task-manager run
  ./run-on-arch-linux.sh ~/dev/Titan/subsystems/titan-task-manager test
EOF
}

if [[ "${1:-}" == '-h' || "${1:-}" == '--help' ]]; then usage; exit 0; fi
if [[ -n "${1:-}" && -f "$1/CMakeLists.txt" ]]; then
  project_dir=$(cd -- "$1" && pwd)
  action=${2:-run}
elif [[ -f ./CMakeLists.txt ]]; then
  project_dir=$(pwd)
  action=${1:-run}
elif [[ -f ./subsystems/titan-task-manager/CMakeLists.txt ]]; then
  project_dir=$(cd ./subsystems/titan-task-manager && pwd)
  action=${1:-run}
else
  echo 'ERROR: Cannot find the TITAN Task Manager source directory.' >&2
  echo 'Extract its ZIP and pass the directory containing CMakeLists.txt.' >&2
  usage >&2
  exit 2
fi

case "$action" in run|build|test|demo|install-desktop) ;; *) echo "Unknown action: $action" >&2;usage >&2; exit 2;; esac

if [[ $(id -u) -eq 0 ]]; then
  echo 'ERROR: Build and run as your regular desktop user, not root/sudo.' >&2
  exit 1
fi
for tool in cmake ninja c++; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "ERROR: Missing $tool. Install prerequisites with:" >&2
    echo '  sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-declarative unzip' >&2
    exit 1
  fi
done

build_dir="$project_dir/build-arch"
echo "Source: $project_dir"
echo "Build:  $build_dir"
cmake -S "$project_dir" -B "$build_dir" -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build "$build_dir" --parallel "$(nproc)"

case "$action" in
  build)
    echo "Built: $build_dir/titan-task-manager"
    ;;
  test)
    ctest --test-dir "$build_dir" --output-on-failure
    ;;
  run|demo)
    if [[ -z "${XDG_RUNTIME_DIR:-}" || ! -d "${XDG_RUNTIME_DIR:-/missing}" ]]; then
      echo 'ERROR: XDG_RUNTIME_DIR is missing. Open a terminal inside a logged-in desktop session.' >&2
      exit 1
    fi
    if [[ -z "${DISPLAY:-}" && -z "${WAYLAND_DISPLAY:-}" ]]; then
      echo 'ERROR: A graphical desktop session (X11 or Wayland) is required.' >&2
      exit 1
    fi
    if [[ "$action" == demo ]]; then
      fixture="$project_dir/tests/fixtures/demo-thm-state.json"
      if [[ ! -f "$fixture" ]]; then
        echo "ERROR: Demo telemetry fixture not found: $fixture" >&2
        exit 1
      fi
      echo 'DEMO MODE: synthetic workload data, not live THM.'
      echo 'Close any existing TITAN Task Manager instance first (single-instance IPC).'
      export THM_TELEMETRY_PATH="$fixture"
      export THM_TELEMETRY_ALLOW_STALE_FIXTURE=1
    else
      echo 'ARCH LINUX MODE: CPU, RAM and process data come from Linux /proc.'
      echo 'THM workload and decision pages may be unavailable; this is expected.'
    fi
    exec "$build_dir/titan-task-manager"
    ;;
  install-desktop)
    mkdir -p "$HOME/.local/share/applications"
    # In .desktop Exec paths, a simple no-whitespace absolute path is safest.
    if [[ "$build_dir" == *' '* || "$build_dir" == *'"'* || "$build_dir" == *'\\'* ]]; then
      echo 'ERROR: Move source under a path with no spaces, quotes or backslashes for desktop launcher.' >&2
      exit 1
    fi
    dest="$HOME/.local/share/applications/titan-task-manager.desktop"
    cat > "$dest" <<DESKTOP
[Desktop Entry]
Type=Application
Name=TITAN Task Manager
Comment=Dark Linux system and process monitor (ArchTitan preview)
Exec=$build_dir/titan-task-manager
Icon=utilities-system-monitor
Terminal=false
Categories=System;Monitor;
StartupNotify=true
DESKTOP
    echo "Installed user launcher: $dest"
    echo 'Look for TITAN Task Manager in your desktop application menu.'
    ;;
esac
