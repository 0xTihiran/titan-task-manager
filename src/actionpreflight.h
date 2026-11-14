#pragma once

#include <QString>
#include <QVariantMap>
#include <QtGlobal>

struct ActionPreflight {
    QString code;
    QString reason;
    bool maySignal = false; // Must remain false without the official THM registry.
};

// Pure, testable policy: compare fresh /proc observations with the cached
// workload-mapped process. NEVER authorizes a signal in this preview release.
ActionPreflight evaluateActionPreflight(const QVariantMap& cached,
                                        const QVariantMap& fresh,
                                        bool telemetryAvailable,
                                        qint64 selfPid,
                                        qint64 parentPid,
                                        qint64 currentUid);
