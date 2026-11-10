# v0.4 — asynchronous THM decisions and Hyprland workspace focus

## New components

- `DecisionsModel`: connects to THM read-only UNIX stream socket and sends `decisions 50\n`. Queries asynchronously every five seconds with a 1.6-second watchdog and a 64-KiB response cap. Does not mutate THM or run external commands. Supports `THM_SOCKET_PATH` override.
- `HyprlandMonitor`: listens to `socket2.sock` for workspace focus events. No `hyprctl` polling. Discovers the preferred signature path and falls back to newest session socket. Reconnects with exponential backoff. Does not claim an initial focus until an event arrives.
- Decisions page now shows human-readable history; workspace page marks focused cards when connected.
- QtTest suites `test_decisions` and `test_hyprland` include lightweight fake UNIX sockets.

## Important boundaries

- Decisions text is **not** a structured API; keep parsing display-only.
- `workspace>>special:` events are not mapped to numeric workspaces.
- THM workload-to-PID tree matching remains best-effort; application remains read-only.
- Official `ArchTitan.TitanUI`/`TitanTheme` imports require the actual OS repository.
- No shared OS integration files were modified.
- Tests must be compiled/executed in a Linux Qt6 development environment; this packaging environment lacks Qt6.

## 0.4.1 bug fixes

The `decisions` QtTest previously failed because the test used an app-less
entry point, and `DecisionsModel` treated a `PeerClosedError` from a
read-until-EOF UNIX socket as a query failure. The implementation now handles
peer-close as successful completion when the socket was busy and preserves
its accumulated response. A socket without a listening server still reports
unavailable; oversized responses fail safely. Both async QtTest suites use
`QTEST_GUILESS_MAIN` now. Run `ctest --test-dir build --output-on-failure`.
