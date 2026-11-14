#include "actionpreflight.h"
#include "protectionfilter.h"

namespace {
ActionPreflight block(const QString& code, const QString& reason) {
    return {code, reason, false};
}
}

ActionPreflight evaluateActionPreflight(const QVariantMap& cached,
                                        const QVariantMap& fresh,
                                        const bool telemetryAvailable,
                                        const qint64 selfPid,
                                        const qint64 parentPid,
                                        const qint64 currentUid) {
    if (cached.isEmpty())
        return block(QStringLiteral("NO_SNAPSHOT"), QStringLiteral("PID not found in the current Task Manager snapshot"));
    if (fresh.isEmpty())
        return block(QStringLiteral("PROCESS_EXITED"), QStringLiteral("Cannot re-read current process identity from /proc"));

    bool cachedOk = false, freshOk = false, cachedStartOk = false, freshStartOk = false;
    const qint64 cachedPid = cached.value("pid").toLongLong(&cachedOk);
    const qint64 freshPid = fresh.value("pid").toLongLong(&freshOk);
    const quint64 cachedStart = cached.value("startTime").toULongLong(&cachedStartOk);
    const quint64 freshStart = fresh.value("startTime").toULongLong(&freshStartOk);
    if (!cachedOk || !freshOk || cachedPid <= 1 || freshPid != cachedPid
        || !cachedStartOk || !freshStartOk || !cachedStart || !freshStart
        || cachedStart != freshStart)
        return block(QStringLiteral("STALE_PID"), QStringLiteral("Process PID/starttime changed, or identity is incomplete"));

    const auto cachedExe = cached.value("exe").toString();
    const auto freshExe = fresh.value("exe").toString();
    const auto cachedCgroup = cached.value("cgroup").toString();
    const auto freshCgroup = fresh.value("cgroup").toString();
    if (cachedExe.isEmpty() || freshExe.isEmpty() || cachedExe != freshExe
        || cachedCgroup.isEmpty() || freshCgroup.isEmpty() || cachedCgroup != freshCgroup)
        return block(QStringLiteral("IDENTITY_CHANGED"), QStringLiteral("Executable or cgroup changed, or could not be verified"));

    if (!cached.value("uidVerified").toBool() || !fresh.value("uidVerified").toBool()
        || cached.value("uid") != fresh.value("uid")
        || cached.value("effectiveUid") != fresh.value("effectiveUid"))
        return block(QStringLiteral("UID_UNVERIFIED"), QStringLiteral("User identity changed or could not be revalidated"));

    // Retain THM assignment from cached mapping, but update identity fields
    // from /proc. Note that root-only THM telemetry CANNOT prove membership.
    QVariantMap checked = cached;
    for (const QString& key : {QStringLiteral("pid"), QStringLiteral("name"), QStringLiteral("exe"),
                               QStringLiteral("cgroup"), QStringLiteral("uid"),
                               QStringLiteral("effectiveUid"), QStringLiteral("uidVerified"),
                               QStringLiteral("startTime")}) {
        checked.insert(key, fresh.value(key));
    }
    const auto assessment = assessProcessProtection(checked, telemetryAvailable,
                                                    selfPid, parentPid, currentUid);
    if (assessment.protectedProcess)
        return block(QStringLiteral("PROTECTED"), assessment.reason);
    if (!telemetryAvailable)
        return block(QStringLiteral("THM_OFFLINE"), QStringLiteral("THM telemetry is missing; workload ownership cannot be checked"));

    // Critical: provisional protected-name lists and root-PID heuristics never
    // become implicit authorization. No compile switch or environment variable
    // can turn this release into a process-signal sender.
    return block(QStringLiteral("REGISTRY_UNVERIFIED"),
                 QStringLiteral("Official THM registry and fresh workload membership verification are not integrated"));
}
