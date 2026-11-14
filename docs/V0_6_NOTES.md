# v0.6 — action preflight preview, NOT process termination

## Trust boundary

QML supplies only a requested PID and an action type. The C++ `ActionController`
fetches the current WorkloadTree process snapshot and re-reads `/proc/<pid>/stat`,
`/proc/<pid>/status`, `/proc/<pid>/exe` and `/proc/<pid>/cgroup`. The pure
`ActionPreflight` compares cached and fresh identities, checks provisional
protection, and always returns `maySignal=false`. C++ cannot send SIGTERM or
SIGKILL in this release because there is no signal-sending implementation.

**This is an intentionally blocked action-review simulation.**

## Test cases

- No cached process / exited process: denied.
- PID starttime mismatch: denied (PID reuse).
- Executable or cgroup changes: denied.
- Missing or changed UID: denied.
- THM-managed or protected: denied.
- Missing THM telemetry: denied.
- Same-user, unmatched, seemingly healthy process: **still denied**, because
  official THM protection registry and conclusive membership proof are absent.
- Confirmations for force review require two successive dialogues.
- Every confirmed review is written as a JSON audit attempt, if permissions
  allow; a log error is shown if the path is not writable.

## Next integration work

1. Import `titan-hwm-v3/core/protected_registry.hpp` and write tests against
   exact name/path registry semantics.
2. Validate real process snapshots against the official THM daemon in a VM.
3. Resolve reparented descendants and stale root PID attribution limitations.
4. Add pidfd-based authorized actions *only after* proven runtime gates and
   identity checks exist; keep existing default fail-closed behavior.
5. Integrate official `ArchTitan.TitanUI` theme and obtain lead approval before
   shared menu, launcher or ISO changes.

No `kill()`, `pidfd_send_signal()`, `SIGTERM`, `SIGKILL` system calls or cgroup
writes occur in the compiled Task Manager source for this milestone.
