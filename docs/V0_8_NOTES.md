# v0.8 — Read-only diagnostics and integration readiness

This update is based on the **v0.7 telemetry-reliability source ZIP**.

## Additions

- **System Diagnostics** popup, accessible from the dashboard header, without adding
  a seventh navigation page. Dark styling uses the existing isolated preview `Theme.qml`.
- `src/diagnosticscore.{h,cpp}` holds deterministic, pure status classification.
- `src/diagnosticsmodel.{h,cpp}` reads only availability signals, kernel-file
  readability, and two existing asynchronous connection states. No external
  process commands are launched; no OS or user files are changed.
- **Copy report** puts a JSON status report on the clipboard only when clicked.
  It includes check IDs, titles, statuses, timestamp, and THM source status.
  It intentionally does NOT include PIDs, process names, commands, cgroup paths,
  usernames, home paths, socket paths, or other detailed process information.
- Added `tests/test_diagnosticscore.cpp` (ninth CTest suite).
- `THM_TELEMETRY_PATH` that differs from the standard production path is
  classified as `OVERRIDE` when recent and parseable, *not* `LIVE`. Explicit
  `THM_TELEMETRY_ALLOW_STALE_FIXTURE=1` custom-source mode is still `DEMO`.
  `LIVE` only means the standard path is fresh and structurally valid; it is
  **not proof of producer authenticity**. See `tests/test_thmclient.cpp`.

## Scope and security

- **No process signals**, no cgroup writes, and no elevation. Review-only
  process dialogs remain unchanged and denied by C++ backend.
- **Three integration blockers** are shown: official `TitanTheme`, official
  `titan-hwm-v3/core/protected_registry.hpp`, and process action enablement.
- The actual ArchTitan OS source checkout has not been provided, so no claim
  of native integration is made. The source-only discovery utility is still
  `scripts/integration_audit.py <ArchTitan OS repo root>`.
- A socket diagnosis reporting `PASS` means the existing asynchronous THM
  **decisions query** connected and returned successfully, not THM authority
  or future permission for process actions.

## How to test in ArchTitan

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-declarative
cd subsystems/titan-task-manager
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
./build/titan-task-manager
```

Open **Diagnostics** in the title bar. Confirm offline/online state changes,
copy the limited JSON report, and check `thmclient` and `diagnosticscore` CTests.

## Remaining gates

1. Obtain the real ArchTitan repository source and inspect the exported QML theme API.
2. Reuse and verify the official `titan-sysmon` THM parser.
3. Import and unit-test the official `titan-hwm` protected registry.
4. Verify workload ownership and PID identity in a dedicated integration test.
5. Acquire owner approval for editing shared OS launcher / airootfs files.
6. Run actual Qt6 compilation, CTest, GUI and ArchTitan VM checks.

This package is a developer preview until those steps are complete.
