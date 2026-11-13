#include "protectionfilter.h"

#include <QFileInfo>
#include <QSet>

namespace {
ProtectionAssessment deny(const QString& reason) {
    return {true, false, QStringLiteral("PROTECTED"), reason};
}
ProtectionAssessment unverified(const QString& reason) {
    return {false, false, QStringLiteral("UNVERIFIED"), reason};
}

// Interim conservative subset inferred from the build plan. NOT the official
// THM registry. In particular, names can be truncated in /proc/<pid>/stat.
bool protectedName(const QString& name) {
    static const QSet<QString> names{
        QStringLiteral("titan-task-manager"), QStringLiteral("titan-hwm"),
        QStringLiteral("titan-hwm-daemon"), QStringLiteral("titan-ai"),
        QStringLiteral("titan-share"), QStringLiteral("titan-mirror"),
        QStringLiteral("titan-sandboxd"), QStringLiteral("titan-sysmon"),
        QStringLiteral("titan-quicksettings"), QStringLiteral("archtitan-settings"),
        QStringLiteral("Hyprland"), QStringLiteral("pipewire"),
        QStringLiteral("wireplumber"), QStringLiteral("systemd"),
        QStringLiteral("systemd-journald"), QStringLiteral("systemd-logind"),
        QStringLiteral("dbus-daemon"), QStringLiteral("dbus-broker"),
        QStringLiteral("NetworkManager"), QStringLiteral("wpa_supplicant"),
        QStringLiteral("gdm"), QStringLiteral("sddm"), QStringLiteral("lightdm"),
        QStringLiteral("Xorg"), QStringLiteral("Xwayland"),
        QStringLiteral("firefox"), QStringLiteral("chromium"),
        QStringLiteral("chrome"), QStringLiteral("brave"),
        QStringLiteral("vivaldi"), QStringLiteral("zen")
    };
    return names.contains(name) || name.startsWith(QStringLiteral("titan-"), Qt::CaseInsensitive)
        || name.startsWith(QStringLiteral("archtitan-"), Qt::CaseInsensitive)
        || name.startsWith(QStringLiteral("firefox"), Qt::CaseInsensitive)
        || name.startsWith(QStringLiteral("chrome"), Qt::CaseInsensitive)
        || name.startsWith(QStringLiteral("chromium"), Qt::CaseInsensitive);
}

bool protectedExe(const QString& exe) {
    if (!exe.startsWith('/')) return false;
    if (exe.startsWith(QStringLiteral("/usr/lib/systemd/"))
        || exe.startsWith(QStringLiteral("/usr/libexec/"))
        || exe.startsWith(QStringLiteral("/usr/bin/titan"))
        || exe.startsWith(QStringLiteral("/usr/local/bin/titan"))
        || exe.startsWith(QStringLiteral("/usr/lib/firefox/"))) return true;
    const QString basename = QFileInfo(exe).fileName();
    return protectedName(basename);
}
}

ProtectionAssessment assessProcessProtection(const QVariantMap& process,
                                              bool telemetryAvailable,
                                              qint64 selfPid,
                                              qint64 parentPid,
                                              qint64 currentUid) {
    bool pidOk = false;
    const qint64 pid = process.value("pid").toLongLong(&pidOk);
    if (!pidOk || pid <= 1) return deny(QStringLiteral("Kernel/init or invalid process identity"));
    if (pid == selfPid || pid == parentPid)
        return deny(QStringLiteral("Task Manager or its parent process"));

    const QString name = process.value("name").toString();
    const QString exe = process.value("exe").toString();
    if (protectedName(name) || protectedExe(exe))
        return deny(QStringLiteral("Known system, ArchTitan, desktop, or browser executable (provisional registry)"));

    // A managed relationship is always protected. Do not rely solely on the
    // numeric ID: the mapper may not have an ID for malformed telemetry.
    if (process.value("assignment").toString() == QStringLiteral("THM managed")
        || process.value("workloadId").toLongLong() != 0)
        return deny(QStringLiteral("THM-managed workload: only THM controls it"));
    if (process.value("workloadObserved").toBool())
        return deny(QStringLiteral("THM observed workload"));
    static const QSet<QString> protectedTypes{
        QStringLiteral("SERVICE"), QStringLiteral("VM"),
        QStringLiteral("MEDIA"), QStringLiteral("BROWSER")
    };
    if (protectedTypes.contains(process.value("workloadType").toString().toUpper()))
        return deny(QStringLiteral("Protected THM workload type"));

    const QString cgroup = process.value("cgroup").toString();
    if (cgroup.contains(QStringLiteral("archtitan.slice"))
        || cgroup.contains(QStringLiteral("archtitan-active"))
        || cgroup.contains(QStringLiteral("archtitan-background"))
        || cgroup.contains(QStringLiteral("archtitan-idle"))
        || cgroup.contains(QStringLiteral("archtitan-reclaimable"))
        || cgroup.contains(QStringLiteral("archtitan-frozen")))
        return deny(QStringLiteral("ArchTitan-managed resource group"));

    // Distinguish a genuinely other-user process from unavailable UID data.
    if (!process.value("uidVerified").toBool())
        return unverified(QStringLiteral("/proc UID verification unavailable; termination disabled"));
    bool uidOk = false, effectiveOk = false;
    const qint64 uid = process.value("uid").toLongLong(&uidOk);
    const qint64 effective = process.value("effectiveUid").toLongLong(&effectiveOk);
    if (!uidOk || !effectiveOk || uid < 0 || effective < 0)
        return unverified(QStringLiteral("Missing or invalid real/effective UID"));
    if (uid != currentUid || effective != currentUid)
        return deny(QStringLiteral("Another user's process or elevated effective UID"));

    bool startOk = false;
    if (process.value("startTime").toULongLong(&startOk) == 0 || !startOk)
        return unverified(QStringLiteral("Missing process start time; PID reuse cannot be checked"));
    if (name.isEmpty() || exe.isEmpty() || !exe.startsWith('/'))
        return unverified(QStringLiteral("Incomplete executable identity; cannot verify protection registry"));
    if (cgroup.isEmpty())
        return unverified(QStringLiteral("Missing cgroup identity; workload membership cannot be verified"));
    if (!telemetryAvailable)
        return unverified(QStringLiteral("THM unavailable; unknown workload membership"));

    // Even a same-user unmatched process with complete metadata is NOT proven
    // unmanaged; THM root PIDs can be recycled or descendants reparented.
    return unverified(QStringLiteral("Unmatched process; official protection registry and fresh identity verification required"));
}
