# v0.5 — read-only protection diagnostics

This milestone updates the supplied **v0.4.2 Compile Fix** project. It is
**not** the official ArchTitan protection subsystem and does **not** implement
process termination.

## Additions

- `src/protectionfilter.{h,cpp}`: conservative and deterministic classifier.
  Labels system/THM/cgroup-protected processes as `PROTECTED`, other unknown
  processes as `UNVERIFIED`. **Every result returns `canTerminate=false`.**
- `src/procidentity.{h,cpp}`: parses real/effective UID from `/proc/<pid>/status`
  and checks a cached PID/starttime against a new parsed `ProcStat` sample.
  The identity check is for unit tests and later use, not a complete action gate.
- `ProcModel`: captures UIDs from the actual status file; missing data is
  marked `uidVerified=false` rather than assuming the owner.
- `WorkloadTree`: annotates process rows and workload tree entries with
  `protectionStatus`, `isProtected`, and an explanation. Still read-only.
- `ProcessesPage.qml`: ACCESS column, Protected filter and explanation in inspector.
- `tests/test_protectionfilter.cpp`: unit tests for denial, identity uncertainty,
  browser/compositor/system names, UID mismatch, cgroups, missing THM and PID reuse.

## Explicit limitations

- The **real** `titan-hwm-v3/core/protected_registry.hpp` was not available.
  Interim names and executable prefixes are intentionally conservative; their
  completeness is **not** assured. They MUST NOT become authorization logic.
- Telemetry only reports workload roots; ancestry matches can miss reparented
  processes and can be wrong when root PIDs are recycled.
- This version has no `ActionController`, `kill`, `pidfd_send_signal`, or
  confirmation UI. Nothing can be terminated from the application.
- Official `ArchTitan.TitanUI` / `TitanTheme` is still unavailable; the existing
  modern dark preview tokens are kept intact. Do not merge into the ISO until
  the real design system is integrated and the app is verified in a VM.
- On a future action, identity and protection checks must be re-evaluated at
  the moment of action; a cached label is never sufficient.

## Test and run

```bash
cd subsystems/titan-task-manager
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
./build/titan-task-manager
```

For the new test only:

```bash
ctest --test-dir build -R '^protectionfilter$' -V
```

Compiled/tested on target Linux: **not yet**; the authoring environment does
not have Qt6 development libraries. Do not claim runtime correctness yet.

## Next phase

1. Import the exact ArchTitan repository sources for `protected_registry.hpp`,
   `titan-sysmon/src/thmstate.*`, and `ArchTitan.TitanUI`.
2. Reconcile the classifier with the authoritative registry and test with real
   non-sensitive process snapshots on ArchTitan.
3. Add runtime safety proof (fresh UID, PID/starttime, cgroup + THM membership,
   official registry), comprehensive tests and confirmation UX **before** any
   action support is considered.
4. Shared OS files remain untouched until team-lead approval.
