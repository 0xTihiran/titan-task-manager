# TITAN Task Manager — Dark Edition v0.7 (telemetry reliability)

This release builds on the supplied v0.6 dark Qt6/C++20 project. Its **next
verified step** is read-only telemetry reliability and native integration
preparation; official ArchTitan source has not been supplied.

## v0.7 additions

- Typed, strict THM JSON validation in `src/thmstateparser.*`.
- Telemetry-file freshness checks: stale / missing / malformed payloads clear
  all previous THM workload data and present an explicit status in the UI.
- Unit tests for parser and client status transitions (total: eight test suites).
- Optional discovery script `scripts/integration_audit.py` for the full OS checkout.
- Existing dark preview UI, workload/process tree, Decisions, Slices,
  Hyprland, protection diagnostics and review-only dialogs remain unchanged.
- **NO process signalling.** No shared ArchTitan files modified.

See `docs/V0_7_NOTES.md` for detailed behavior, test procedures, and scope.

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

Archive integrity, data fixtures, and project references can be checked in the
authoring environment. Qt6 development libraries are not installed here, so
**CMake compile, all CTests, and live GUI behavior have not been verified**.
