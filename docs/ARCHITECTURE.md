# TITAN Task Manager architecture — v0.3 (read-only prototype)

```text
                    Dark Qt6/QML UI
   Overview     Workloads [expand]    Processes [inspector]
       |                |                    |
       +----SystemModel |    +---------------+
                         v    v
                       WorkloadTree
                         |    |
                 ThmClient   ProcModel
                   |            |
              THM JSON      Linux /proc
                 (1Hz)         (~0.67Hz)
                         |
                  WorkloadTreeCore
                (pure reconciliation)

Other pages:
  Workspaces <- THM JSON workspace field (no Hyprland IPC yet)
  Decisions  <- THM last_action field (history pending)
  Slices     <- SliceModel -> cgroup2 sysfs (read only)

Single instance: IPC server under $XDG_RUNTIME_DIR
```

## Data model and security

`ProcModel` extracts per-process data from `/proc/<pid>/stat` with a separate
`parseProcStat()` helper, plus command, exe/cwd links, cgroup and owner UID when
readable. Snapshots include starttime, ppid, CPU%, RSS and name. The CPU delta
is one-core-normalized; multithreaded processes may exceed 100%.

`ThmClient` reads THM JSON from `/run/archtitan/thm-state.json` (or the optional
`THM_TELEMETRY_PATH` override). It is a lightweight provisional parser: the
real `titan-sysmon/src/thmstate.*` needs to replace it on repository integration.

`WorkloadTreeCore` is a pure function taking THM root records plus Linux process
records and returning annotated records. It:

1. Indexes processes by PID and their children by PPID.
2. Detects all THM root PIDs as boundaries, including nested roots.
3. BFS-traverses each available root. Rejects impossible parent/child chronology
   when their `startTime` ticks indicate child predates parent.
4. Avoids cycles and duplicate ownership.
5. Sets per-workload `tree`, `matchedCount`, `rootPresent`, `stale`.
6. Sets per-process `assignment`, `workloadId`, `workloadName`, and always
   `canTerminate=false`.

**Important:** THM does *not* publish process member PIDs or starttimes.
Inferring lineage is useful for display but isn't authoritative: reparented
processes may be unmatched and a recycled root PID may be associated with the
wrong group. This must not be used as an authorization decision.

**Safety invariant:** The entire v0.3 app remains read-only. Unmatched does not
mean unmanaged/safe to kill. The required process protection and signal
pipeline is out of scope until the official `ProtectionFilter` is implemented,
security-reviewed, and tested. QML never provides authorization.

## Design system integration

`qml/Theme.qml` is a self-contained dark palette used only as a development
preview. It exposes named tokens for surfaces, text, accent, states and chart
colors. The user-supplied plan requires **real `ArchTitan.TitanUI` components**
and **real `TitanTheme` semantic tokens** in the finished application. The
actual module source wasn't supplied, so its API must not be invented.

## Testing

`test_procparser` exercises tricky `comm` contents and invalid stat fields.
`test_workloadtree` tests descendant reconciliation, stale roots, nested roots,
chronology rejection, and telemetry-offline behavior. Run with:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```

The above tests require the ArchTitan/Arch Linux Qt6 development environment.


## v0.5 protection diagnostics (read-only)

`ProcModel` adds verified real/effective UID properties from `/proc/[pid]/status`.
`WorkloadTree` passes each reconciled process to pure `assessProcessProtection`.
Known protected or managed processes are marked `PROTECTED`; all others stay
`UNVERIFIED` until the official THM registry and runtime proof are available.
**`canTerminate=false` for every process.** No signals, cgroup writes, or
privilege escalation are implemented. See `docs/V0_5_NOTES.md`.
