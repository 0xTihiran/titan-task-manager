# TITAN Task Manager — Dark Edition v0.4.2 (socket-test fixes)

Read-only desktop Task Manager for ArchTitan OS in **C++20 + Qt6 + QML**. This
version extends the v0.3 dark dashboard with Linux `/proc` parsing, a workload
process tree, searchable workload/process reconciliation, and unit tests.

**Status:** Source package provided. The app has **not** been compiled/run in
this environment (Qt6 development libraries unavailable). Do not deploy to
production or merge into the ArchTitan ISO without building/testing it first.

## v0.4.2 fixes

- Corrected THM decisions response parsing after normal peer close (EOF).
- Changed socket-based QtTest entry points to `QTEST_GUILESS_MAIN`,
  which supplies a `QCoreApplication` and an event dispatcher.
- Improved the fake THM test server to buffer complete commands, and added
  a diagnostic socket-error assertion and an unavailable-socket test.
- Removed an unused C++ variable in the workload reconciliation code.
- **Not yet compiled on Arch Linux in this environment**. Build and CTest on
  your target Qt 6 environment using the commands below.

## Existing v0.4 features

- Historical THM decisions from the **read-only** `/tmp/titan_hwm.sock` (`decisions 50`), with asynchronous polling, deadline, and bounded buffer. Override with `THM_SOCKET_PATH` for tests.
- Hyprland workspace-focus events from `.socket2.sock`, with automatic reconnection and fallback session discovery. Focus is unknown until first event arrives.
- Decisions page now shows history lines; Workspaces highlights the active workspace if the socket is connected.
- Added socket-based QtTests and event parser tests; read `docs/V0_4_NOTES.md`.

## Earlier v0.3 functionality

- WorkloadTree and a **pure reconciliation function** that maps THM root PIDs to
  Linux process parent-child relationships, without modifying THM or cgroups.
- Click-to-expand cards on the **Workloads** page. Nested processes show live
  PID, CPU%, RSS, state and indentation. Shows up to 60 rows per card.
- **Missing roots** are labelled STALE; processes without a confirmed THM root
  are labelled **Unmatched**, not automatically considered safe/unmanaged.
- **Nested root** boundary: a child workload root is not consumed by its parent
  workload in the displayed process tree.
- **Processes** table is now labelled with workload assignment and has
  All/Managed/Unmatched filters and an inspector (command, path, parent, etc.).
- Dedicated, testable `/proc/<pid>/stat` parser for unusual names containing
  spaces or parentheses. Starts comparisons with starttime to avoid CPU spikes
  caused by PID reuse.
- Four QtTest suites available through CTest.

## Existing functionality from v0.2

- Dark preview token theme, navigation and responsive monitor widgets.
- Live CPU/RAM graphs, memory/CPU/IO PSI, process list and THM telemetry.
- THM-only workspace map and recent decision; read-only cgroup slice metrics.
- Single instance app: `--show`, `--hide`, `--toggle` via local IPC.
- Runs in Linux-only mode when THM telemetry is unavailable.

## Requirements (Arch Linux or ArchTitan)

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-declarative
```

## Install into existing ArchTitan source tree

Extract the ZIP at the **root of the ArchTitan repository**. Source files live
only under `subsystems/titan-task-manager/`. Review changes if that directory
already exists; **do not overwrite team members' changes blindly**.

## Build, run, and test

```bash
cd subsystems/titan-task-manager
chmod +x build.sh
./build.sh
ctest --test-dir build --output-on-failure
./build/titan-task-manager
```

Click **Workloads** and expand a THM workload card to inspect the root process
and its descendants. Select any row under **Processes** to view process info.
All actions are read-only. Use the search field at the top-right to filter.

To use *synthetic* telemetry for visual evaluation when THM is not present:

```bash
THM_TELEMETRY_PATH="$PWD/tests/fixtures/demo-thm-state.json" ./build/titan-task-manager
```

This fixture contains fake PIDs. They will appear as **STALE ROOT**, because
there are no matching real `/proc` entries; this is expected and demonstrates
correct fallback behavior. On the real ArchTitan OS, THM root PIDs may link to
live process trees. Stop any existing instance first before testing an alternate
telemetry path (a second instance only sends IPC to the first).

## Architecture notes

See `docs/ARCHITECTURE.md` for the dataflow, reconciliation limitations, and
C++ vs. QML trust boundary. The task manager is a **monitor**; THM alone manages
workloads and cgroups.

## Hard limits / still pending

- No process termination, force kill, frozen-state changes, system controller
  writes, or privilege escalation. Process action gates need the **tested**
  official `ProtectionFilter` before implementation.
- Root PID to subtree association is best-effort; THM telemetry has no member
  PIDs or root starttime. Reparented descendants cannot be identified solely
  through ancestry, and recycled roots may be misclassified in the UI.
- Must still integrate `titan-sysmon/src/thmstate.*` and the **official**
  `ArchTitan.TitanUI` / `TitanTheme` from the actual repo. `qml/Theme.qml` is a
  centralized dark **preview only**, not official theme API.
- Complete protection filter,
  and OS menu/ISO/keybinding integrations are future milestones.
- Shared OS files may be modified **only with lead approval**.

## Recommended next development step

Import and inspect the ArchTitan repository, then replace the temporary THM
parser and theme with the *actual* repo-native interfaces. Run `./run-vm.sh`
and check live THM workload root PIDs. Add unit tests using real sample telemetry.

## v0.4.2 header compatibility fix

Includes `<QList>` explicitly in `src/procparser.cpp` for Qt 6.12 / GCC 16.
Earlier source versions may fail compilation because Qt headers no longer expose
the full `QList` class transitively.
