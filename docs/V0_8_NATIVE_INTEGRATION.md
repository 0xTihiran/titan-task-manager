# v0.8 — Native ArchTitan OS integration

Source verified against user-provided `ArchTitan-OS-main(1).zip` (2026-11-14).
The previous Task Manager baseline is v0.7 (read-only telemetry reliability).
This is **only** a Task Manager subsystem overlay, NOT a modified OS archive.

## Verified dependencies from ArchTitan OS source

| Dependency | Actual repository path | Integration |
|---|---|---|
| THM reader | `subsystems/titan-sysmon/src/thmstate.hpp` + `.cpp` | Compiled directly by Task Manager CMake target |
| THM publisher | `titan-hwm-v3/ipc/telemetry_publisher.cpp` | Used to confirm fields, no copy |
| Protection sets | `titan-hwm-v3/core/protected_registry.hpp` | Imported directly for static names/prefixes |
| Titan UI | `titan-ui/qml/style/TitanTheme.qml` | Imported as `ArchTitan.TitanUI` singleton |
| UI plugin | `airootfs/usr/lib/qt6/qml/ArchTitan/TitanUI/qmldir` | Matches module URI and controls |
| Controls | `titan-ui/qml/components/TitanButton.qml` and `TitanSidebarItem.qml` | Used in the existing dark UI |

## Telemetry compatibility

The canonical native `ThmReader::read(path)` returns `ThmState` with workload
keys `memMb` and `cpuPct` rather than publisher/UI `mem_mb` and `cpu_pct`.

The v0.8 adapter first uses v0.7's bounded, strict JSON validation and 10s
freshness checks. Then it calls the existing native `ThmReader::read(path)`.
On mismatch between the guarded and native snapshots, it clears data and
reports `INVALID`. (Concurrent atomic publication can briefly cause this;
the next 1Hz poll retries.) It normalizes both naming forms into the Task
Manager's public map while retaining native values for pressure, counts, etc.

**`LIVE` means the telemetry is structurally valid and recently written; it
is not a trusted guarantee of process membership.** `DEMO` is still a
synthetic, read-only status.

## Official theme, no invented tokens

The `qml/Theme.qml` bridge now maps exactly to exported `TitanTheme` properties:

- `bg0`, `bg1`, `bg3`, `bg4`, `stateHover`
- `textHigh`, `textMid`, `textLow`, `accent`, `accentSoft`
- `borderSubtle`, `success`, `warning`, `danger`
- `radiusCard`, `fontFamily`, `alphaColor()`

`ActionButton` now wraps `TitanButton` and sidebar navigation uses
`TitanSidebarItem`. The rest of the design stays compatible with the dark
UI, with semantic colors following ArchTitan's current global theme.
The application does **not** force the global OS theme into dark mode.

## Protection: what the registry can and cannot prove

`ProtectionFilter` uses the **actual** `thm::protected_names()`,
`thm::protected_exe_prefixes()`, `thm::browser_names()` and
`thm::browser_exe_prefixes()` static registries, plus some stricter local
fail-closed rules. **It does not instantiate a daemon registry**: the daemon
keeps dynamically registered protected PIDs privately, and the read-only
socket does not expose a fully authoritative membership/permissions service.
All actions remain review-only (`ActionController.actionsEnabled == false`).
No process signal or cgroup write is implemented.

## Build inside the complete OS repository

Copy this overlay so all files are in `ArchTitan-OS-main/subsystems/titan-task-manager/`.

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-declarative
cd ArchTitan-OS-main/subsystems/titan-task-manager
./build.sh
ctest --test-dir build --output-on-failure
```

For a live ArchTitan OS session with `ArchTitan.TitanUI` installed:

```bash
./build/titan-task-manager
```

For development outside the OS image, build the `titan-ui` sibling module
first (see `titan-ui/CMakeLists.txt` and its README), then launch:

```bash
./run-dev.sh
```

`run-dev.sh` searches `titan-ui/build/qml/ArchTitan/TitanUI` and exposes it as
`ARCHTITAN_QML_IMPORT_PATH`. If the module is installed system-wide, it uses
Qt's normal import path. For a custom TitanUI build location, set the variable
manually. The app will log QML import diagnostics if the module is unavailable.

Demo fixture: stop the existing instance, then run:

```bash
THM_TELEMETRY_ALLOW_STALE_FIXTURE=1 \
THM_TELEMETRY_PATH="$PWD/tests/fixtures/demo-thm-state.json" \
./run-dev.sh
```

The demo fixture is not suitable to verify live process-tree matching because
its PIDs are synthetic.

## Tests and boundaries

Eight existing QtTest suites are retained, plus `native_registry` which
can also compile by itself using a C++20 compiler. The native THM reader
now builds in `titan-task-manager` and `test_thmclient` from the *sibling*
source file. A new assertion checks both native `memMb/cpuPct` and UI
`mem_mb/cpu_pct` aliases.

- `ctest` and QML runtime must be verified on ArchTitan; Qt6 dev packages
  were not available in the authoring environment.
- `test_native_registry` was compile/run checked using GNU C++20 against the
  shipped `protected_registry.hpp` without Qt.
- No `airootfs`, `profiledef.sh`, Hyprland, menu, launcher or Spotlight files
  have been changed; obtain lead approval before those integrations.
- The archived OS tree currently contains a **scaffold-only**
  `subsystems/titan-task-manager/`, so review and merge the overlay instead
  of automatically replacing others' changes.
