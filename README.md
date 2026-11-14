# TITAN Task Manager — Dark Edition v0.6 (action review preview)

C++20 + Qt6/QML read-only ArchTitan task manager. This version **does not** terminate
processes. It adds fail-closed preflight verification, audit logging and two-step
confirmation *preview* UX built on the user's v0.5 project.

## v0.6 additions

- `ActionController::reviewTermination(pid, force)` reads fresh `/proc` data,
  checks PID/starttime, executable, cgroup and UID against the current cached
  snapshot, then refuses the action with a specific reason.
- `ActionPreflight` is pure and independently unit-tested. No code path may
  send a signal: `actionsEnabled` is always false; `maySignal` is always false.
- Inspector has **Review Terminate** and **Review Force Kill** previews.
  Force review has a second confirmation. Both produce **no signal**.
- Review attempts are logged as JSON lines under
  `$XDG_STATE_HOME/titan-task-manager/actions.log` or
  `~/.local/state/titan-task-manager/actions.log`.
  Log entries are append-only in the app; this is NOT an immutable/tamper-proof audit system.
- Six QtTest suites including `actionpreflight`.
- Dark preview theme and six pages retained.

## Important deployment limitation

The real ArchTitan sources for `titan-hwm-v3/core/protected_registry.hpp`,
`subsystems/titan-sysmon/src/thmstate.*` and `ArchTitan.TitanUI` have **not**
been provided. The v0.5 protection list is provisional; THM telemetry only
provides root PIDs and cannot conclusively prove all process membership.
**Do not enable real SIGTERM/SIGKILL, merge to OS integration or deploy in an ISO**
until the official registry is imported, runtime checks are revalidated and
VM tests pass. Shared ArchTitan files were not modified.

## Build and test on Arch Linux / ArchTitan

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-declarative
cd subsystems/titan-task-manager
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
./build/titan-task-manager
```

On the **Processes** page, select a process and click `Review Terminate` or
`Review Force Kill`. Confirm to receive a blocked preflight result; no process
signal is sent. Logs go to the path displayed in the dialog.

To test synthetic THM sample data, stop any running task manager first:

```bash
THM_TELEMETRY_PATH="$PWD/tests/fixtures/demo-thm-state.json" ./build/titan-task-manager
```

## Verification status

The source archive and project wiring can be validated in the authoring
container, but Qt6 development packages are unavailable here so compilation,
CTest, and GUI runtime are **not verified**. Run these on the ArchTitan target.
See `docs/V0_6_NOTES.md` for test cases and subsequent work.
