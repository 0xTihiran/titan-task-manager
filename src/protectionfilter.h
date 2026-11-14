#pragma once
#include <QString>
#include <QVariantMap>
#include <QtGlobal>

// Conservative, review-only protection classification. IMPORTANT:
// this is NOT the official titan-hwm protected_registry.hpp, which must be
// imported from the ArchTitan OS repo before any action can be enabled.
struct ProtectionAssessment {
    bool protectedProcess = false;
    bool canTerminate = false; // ALWAYS false in v0.5
    QString status;            // "PROTECTED" or "UNVERIFIED"
    QString reason;
};

// Pure classifier: explicit caller identity keeps it deterministic for QtTest.
// A positive result never gives the UI permission to signal a process.
ProtectionAssessment assessProcessProtection(const QVariantMap& process,
                                              bool telemetryAvailable,
                                              qint64 selfPid,
                                              qint64 parentPid,
                                              qint64 currentUid);
