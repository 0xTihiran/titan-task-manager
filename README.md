# TITAN Task Manager — Dark Edition v0.8 (diagnostics)

This version builds directly on the supplied v0.7 Qt6/C++20 project.
It adds safe, read-only environment diagnostics; official ArchTitan source
has not been supplied.

## v0.8 additions

- System Diagnostics popup in the header (keeps six navigation sections).
- Shows standard-path versus custom-path THM status, CPU/RAM, PSI, cgroup v2,
  read-only THM decision socket, and Hyprland event availability.
- Shows explicit blockers for official TitanTheme, THM protected registry,
  and process-action activation; all remain unimplemented.
- One-click copy of a **limited JSON status report**, without any process
  names, PIDs, command lines, or filesystem paths.
- Ninth QtTest suite: `diagnosticscore`; `thmclient` tests updated for `OVERRIDE`.
- Preserves all v0.7 monitoring, dark-theme preview, and disabled action reviews.
- **NO process signalling.** No shared ArchTitan files modified.

See `docs/V0_8_NOTES.md` for details.

## Build

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-declarative
cd subsystems/titan-task-manager
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
./build/titan-task-manager
```

## Synthetic-data preview

Stop any existing instance first. This fixture will be labelled **DEMO**.

```bash
THM_TELEMETRY_ALLOW_STALE_FIXTURE=1 \
THM_TELEMETRY_PATH="$PWD/tests/fixtures/demo-thm-state.json" \
./build/titan-task-manager
```

## Still waiting on the real OS source

- `subsystems/titan-sysmon/src/thmstate.{hpp,cpp}`
- `titan-hwm-v3/core/protected_registry.hpp`
- `ArchTitan.TitanUI` and `TitanTheme` exported definitions

The existing `qml/Theme.qml` is a modern **dark preview**; it does not yet use
official TitanTheme tokens. Termination remains intentionally impossible.

## Validation

The archive, JSON fixtures and source references were checked in the authoring
environment. Qt6 development libraries were not available, so **CMake
compilation, the nine CTest suites, and live GUI behavior are unverified**.
