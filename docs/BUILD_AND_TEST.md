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
   running instance before setting `THM_TELEMETRY_PATH`.

The app is still a read-only preview; safe termination and official
`ArchTitan.TitanUI` integration are NOT implemented.
