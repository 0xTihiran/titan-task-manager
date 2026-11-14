# Arch Linux build and test

1. Install requirements (once):

   ```bash
   sudo pacman -S --needed base-devel cmake qt6-base qt6-declarative
   ```

2. In `subsystems/titan-task-manager` run:

   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
   cmake --build build -j"$(nproc)"
   ctest --test-dir build --output-on-failure
   ./build/titan-task-manager
   ```

3. If `decisions` fails, print its precise reason and event flow:

   ```bash
   ctest --test-dir build -R '^decisions$' -V
   ```

4. To test a standalone dark UI with synthetic THM data, stop any
   running instance and use the explicit fixture mode:

   ```bash
   THM_TELEMETRY_ALLOW_STALE_FIXTURE=1 \
   THM_TELEMETRY_PATH="$PWD/tests/fixtures/demo-thm-state.json" \
   ./build/titan-task-manager
   ```

5. To inspect full-OS integration requirements without changing the repo:

   ```bash
   python3 scripts/integration_audit.py /path/to/ArchTitan-OS-main
   ```

6. Open **Diagnostics** in the header. Check THM status, kernel interfaces,
   THM decisions service, Hyprland connection and the explicit integration
   blockers. Use **Copy report** to share check statuses without exposing
   process details. Run the new test independently if needed:

   ```bash
   ctest --test-dir build -R '^diagnosticscore$' --output-on-failure
   ```

The app is still a read-only preview; safe termination and official
`ArchTitan.TitanUI` integration are NOT implemented.
