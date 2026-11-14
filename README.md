# TITAN Task Manager v0.8 — ArchTitan OS native integration (read-only)

This source package was updated using the **actual uploaded ArchTitan OS
repository** and the previous **v0.7** Task Manager project. It keeps the
modern UI while adopting real repository-owned THM and TitanUI interfaces.

### Implemented in v0.8 source

- Compile existing `subsystems/titan-sysmon/src/thmstate.cpp` directly
  (do not fork or copy it into this subsystem).
- Validate and compare canonical THM state; expose `memMb/cpuPct` AND
  `mem_mb/cpu_pct` for UI compatibility.
- Import native `ArchTitan.TitanUI.TitanTheme` semantic colors, typography,
  and radii (no preview RGB tokens).
- Use shared `TitanButton` and `TitanSidebarItem` for actions/navigation.
- Read official THM static protected-name/executable-prefix and browser sets,
  keeping additional conservative restrictions.
- Retain all 8 previous QtTest suites and add a standalone static-registry
  C++ smoke test.
- Keep all process action reviews **simulation-only**. Never signal a process.

### Build in the full ArchTitan OS repository

Extract this ZIP at the OS repository root to merge files into
`subsystems/titan-task-manager/`. **Review diffs** before overwriting anything.
The ZIP is an overlay, not the complete OS repository.

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-declarative
cd subsystems/titan-task-manager
chmod +x build.sh run-dev.sh
./build.sh
ctest --test-dir build --output-on-failure
./run-dev.sh
```

**Run-time dependency:** ArchTitan's `ArchTitan.TitanUI` QML module, either
installed in the OS or built from sibling `titan-ui/`. If you build its module
into the default `titan-ui/build`, `run-dev.sh` configures the import path.

To exercise synthetic THM telemetry, stop any running instance and use:

```bash
THM_TELEMETRY_ALLOW_STALE_FIXTURE=1 \
THM_TELEMETRY_PATH="$PWD/tests/fixtures/demo-thm-state.json" \
./run-dev.sh
```

The example PIDs are fictional and should display as missing roots.

**Read `docs/V0_8_NATIVE_INTEGRATION.md`** for exact native source paths,
limitations, test details and the remaining OS team approval gates.

**Validation:** project/file checks and non-Qt native-registry smoke test can
be run in the authoring environment. Qt6 compilation, ctest and the GUI are
not verified there and must be tested inside ArchTitan.
