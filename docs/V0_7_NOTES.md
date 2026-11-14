# v0.7 — THM Telemetry Hardening and Native Integration Readiness

This version is built from the **user's v0.6 source package**. Its exact
baseline was `TITAN_Task_Manager_v0.6_Safety_Review_Dark_Source.zip`.

## What changed

- Extracted the documented THM v3 JSON parsing into pure
  `src/thmstateparser.h/.cpp` for deterministic testing.
- Validation rejects truncated/oversized payloads, unexpected count fields,
  impossible RAM percentages, non-object workloads, inconsistent array counts.
- If the THM telemetry file has not been rewritten within 10 seconds, or
  its modification time is too far into the future, it is marked **STALE**.
  No stale workloads/process assignment data remains exposed.
- UI distinguishes `LIVE`, `STALE`, `INVALID`, `MISSING`, `DEMO`.
- Added `thmstateparser` and `thmclient` QtTest suites, including live → invalid
  → live → stale → demo → missing transitions.
- Added `scripts/integration_audit.py` to locate repo-native source modules
  **without modifying shared ArchTitan files**.

## Source-status meanings

| Status | Meaning | `thm.available` |
| --- | --- | --- |
| LIVE | Structurally valid JSON file with recent modification time | true |
| STALE | File older than 10s (or clock-skew anomaly) | false |
| INVALID | Recent file but malformed/invalid schema | false |
| MISSING | No access, unreadable or missing file | false |
| DEMO | Explicitly allowed synthetic fixture; never production proof | true, preview only |

**Note:** `LIVE` is a file freshness/shape check, **not** cryptographic
verification, official parser compatibility, or proven workload membership.
This version **never sends process signals**. A fixture status of `DEMO` must
never be used to authorize any future action.

## Live tests

```bash
cd subsystems/titan-task-manager
./build.sh
ctest --test-dir build --output-on-failure
./build/titan-task-manager
```

## Demo fixture

The packaged example fixture is committed to source control, so its modification
time will likely exceed the production freshness threshold. To use it in an
explicit development-only preview, stop any running instance and run:

```bash
THM_TELEMETRY_ALLOW_STALE_FIXTURE=1 \
THM_TELEMETRY_PATH="$PWD/tests/fixtures/demo-thm-state.json" \
./build/titan-task-manager
```

The UI identifies this as **DEMO**, not LIVE. Using the real THM path with the
fixture override will **not** bypass freshness checks.

## Integration inspection (needs the official OS checkout)

```bash
python3 scripts/integration_audit.py /path/to/ArchTitan-OS-main
```

The user has not supplied the ArchTitan OS repository in this conversation.
Consequently this version cannot truthfully claim full `TitanTheme` or native
`titan-sysmon` integration. The existing modern dark `qml/Theme.qml` remains
an isolated **preview** token implementation, not an official theme singleton.

### Official integration gate

1. Obtain and review `subsystems/titan-sysmon/src/thmstate.{hpp,cpp}`.
2. Verify JSON schema with real samples and THM daemon publisher.
3. Obtain and review `titan-hwm-v3/core/protected_registry.hpp`.
4. Map the preview theme tokens to *existing*, not invented, TitanUI tokens.
5. Run all tests in ArchTitan VM; check startup, stale file, stopped daemon,
   malformed JSON, Wayland compositor reconnects.
6. Get lead approval before changes to `airootfs`, Hyprland configs, launcher,
   Spotlight, menu, or `profiledef.sh`.
7. Do **not** activate termination until registry and membership guarantees
   are independently verified and reviewed.

## Test limitations

Project references and archive can be statically validated here, but Qt6
libraries are not installed in the authoring environment. Compilation,
`ctest`, and GUI runtime must be verified on the target Arch Linux machine.
