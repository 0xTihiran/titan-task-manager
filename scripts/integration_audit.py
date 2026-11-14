#!/usr/bin/env python3
"""Read-only readiness report for the ArchTitan OS integration dependencies."""
from pathlib import Path
import argparse


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("repo", type=Path, help="Path to extracted ArchTitan OS repository root")
    args = parser.parse_args()
    repo = args.repo.expanduser().resolve()
    if not repo.is_dir():
        parser.error(f"Not a directory: {repo}")

    checks = {
        "THM telemetry parser (hpp)": ["subsystems/titan-sysmon/src/thmstate.hpp"],
        "THM telemetry parser (cpp)": ["subsystems/titan-sysmon/src/thmstate.cpp"],
        "THM protected registry": ["titan-hwm-v3/core/protected_registry.hpp",
                                   "subsystems/titan-hwm-v3/core/protected_registry.hpp"],
        "THM telemetry publisher": ["titan-hwm-v3/ipc/telemetry_publisher.cpp",
                                    "subsystems/titan-hwm-v3/ipc/telemetry_publisher.cpp"],
        "ArchTitan TitanUI module": ["airootfs/usr/local/lib/qt6/qml/ArchTitan/TitanUI/qmldir",
                                     "subsystems/titan-ui/qmldir"],
    }
    missing = []
    print(f"ArchTitan integration report for: {repo}")
    for name, candidates in checks.items():
        found = [repo / r for r in candidates if (repo / r).is_file()]
        if found:
            print(f"[FOUND] {name}: {found[0].relative_to(repo)}")
        else:
            print(f"[REVIEW] {name}: not found at plan-specified/typical paths")
            missing.append(name)

    themes = list(repo.rglob("TitanTheme.qml"))[:4]
    if themes:
        for candidate in themes:
            print(f"[FOUND] Theme candidate: {candidate.relative_to(repo)}")
    else:
        print("[REVIEW] TitanTheme.qml not found by name; theme may be a C++ singleton")

    print("\nThis is discovery only. No OS files were changed.")
    print("Theme integration requires inspecting the real exported token names, not guessing.")
    print("Protected registry must be reviewed against the daemon before signals are enabled.")
    return 0 if not missing else 2


if __name__ == "__main__":
    raise SystemExit(main())
