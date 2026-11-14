#pragma once

#include <QString>
#include <QVariantList>

// Inputs intentionally contain only state booleans and broad status labels.
// No process names, command lines, PIDs, file paths, or private data are
// collected by the diagnostics report.
struct DiagnosticsInput {
    QString thmSourceStatus; // LIVE, OVERRIDE, DEMO, STALE, INVALID, MISSING
    bool systemAvailable = false;
    bool procfsReadable = false;
    bool cpuPsiReadable = false;
    bool memoryPsiReadable = false;
    bool ioPsiReadable = false;
    bool cgroupV2Readable = false;
    bool thmDecisionsConnected = false;
    bool hyprlandConnected = false;
    bool officialThemeIntegrated = false;
    bool officialRegistryIntegrated = false;
};

// Each check: {id, title, status: PASS|INFO|WARN|BLOCKED, detail}.
QVariantList buildDiagnosticChecks(const DiagnosticsInput& input);
int countDiagnosticWarnings(const QVariantList& checks);
int countDiagnosticBlockers(const QVariantList& checks);
QString makeDiagnosticReport(const QVariantList& checks,
                             const QString& generatedAtUtc,
                             const QString& sourceStatus);
