#pragma once
#include "procparser.h"
#include <QByteArray>
#include <QVariantMap>
#include <QtGlobal>

// Parse the real and effective UID from /proc/<pid>/status.
// Missing or malformed fields cannot authorize any future action.
bool parseProcUids(const QByteArray& status, qint64* realUid, qint64* effectiveUid);

// Before any future action, read /proc/<pid>/stat AGAIN and check the
// PID/starttime pair. This is only an identity helper; no signals are sent.
bool sameProcessIdentity(const QVariantMap& cached, const ProcStat& current);
