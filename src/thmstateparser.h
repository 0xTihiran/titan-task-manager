#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QVariantList>

// Standalone validated parsing boundary for the documented THM v3 JSON shape.
// Replace/compare with titan-sysmon's authoritative parser when the OS repo
// is available. This parser is read-only; it never authorizes process actions.
struct ThmSnapshot {
    QString version, pressureLevel, governor, lastDecision;
    double memoryUsedPct = 0.0;
    int workloadCount = 0;
    int activeWorkloads = 0;
    int seenProcesses = 0;
    int unmanagedProcesses = 0;
    QVariantList workloads;
    QVariantList topUnmanaged;
};

// Returns false without modifying *out on malformed / unexpected input.
bool parseThmState(const QByteArray& bytes, ThmSnapshot* out, QString* error = nullptr);

// Atomic-rename telemetry should be rewritten about once/second. If the
// writer stops, its last valid payload is NOT a live process-ownership proof.
// qint64 milliseconds keeps these helpers deterministic in tests.
bool thmFileIsFresh(qint64 modifiedMs, qint64 nowMs,
                    qint64 maxAgeMs = 10000, qint64 futureSkewMs = 5000);
