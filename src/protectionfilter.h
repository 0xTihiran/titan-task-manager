#pragma once
#include <QString>
#include <QVariantMap>
#include <QtGlobal>

// Conservative, review-only protection classification. Uses the official
// repo-owned THM *static* names and executable prefixes, with additional
// conservative rules. Dynamic protected PID registrations remain daemon-only;
// no process action may be authorized by this classifier.
struct ProtectionAssessment {
    bool protectedProcess = false;
    bool canTerminate = false; // ALWAYS false in v0.8
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
