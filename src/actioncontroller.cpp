#include "actioncontroller.h"
#include "actionpreflight.h"
#include "procidentity.h"
#include "procparser.h"
#include "thmclient.h"
#include "workloadtree.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtGlobal>
#include <unistd.h>

ActionController::ActionController(WorkloadTree* workloads, ThmClient* thm, QObject* parent)
    : QObject(parent), m_workloads(workloads), m_thm(thm) {}

QString ActionController::auditLogPath() const {
    QString base = qEnvironmentVariable("XDG_STATE_HOME");
    if (base.isEmpty() || !QDir::isAbsolutePath(base))
        base = QDir::homePath() + QStringLiteral("/.local/state");
    return QDir(base).filePath(QStringLiteral("titan-task-manager/actions.log"));
}

QVariantMap ActionController::readLiveIdentity(qint64 pid) const {
    if (pid <= 1) return {};
    const QString prefix = QStringLiteral("/proc/") + QString::number(pid) + '/';
    QFile statFile(prefix + QStringLiteral("stat"));
    if (!statFile.open(QIODevice::ReadOnly)) return {};
    ProcStat stat;
    if (!parseProcStat(statFile.readAll(), &stat) || stat.pid != pid) return {};

    QFile statusFile(prefix + QStringLiteral("status"));
    qint64 uid = -1, effective = -1;
    const bool uidVerified = statusFile.open(QIODevice::ReadOnly)
        && parseProcUids(statusFile.read(16384), &uid, &effective);
    QFile cgroupFile(prefix + QStringLiteral("cgroup"));
    QString cgroup;
    if (cgroupFile.open(QIODevice::ReadOnly))
        cgroup = QString::fromLocal8Bit(cgroupFile.read(8192)).trimmed();
    return {{"pid", pid}, {"name", stat.comm},
            {"startTime", QVariant::fromValue(stat.starttime)},
            {"exe", QFileInfo(prefix + QStringLiteral("exe")).symLinkTarget()},
            {"cgroup", cgroup}, {"uid", uid},
            {"effectiveUid", effective}, {"uidVerified", uidVerified}};
}

bool ActionController::appendAudit(const QVariantMap& audit) const {
    const QString path = auditLogPath();
    const QFileInfo info(path);
    // Do not follow a pre-existing log symlink.
    if (info.isSymLink()) return false;
    if (!QDir().mkpath(info.absolutePath())) return false;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) return false;
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    const QByteArray record = QJsonDocument(QJsonObject::fromVariantMap(audit)).toJson(QJsonDocument::Compact) + '\n';
    return file.write(record) == record.size() && file.flush();
}

QVariantMap ActionController::reviewTermination(const qint64 pid, const bool force) {
    QVariantMap cached;
    for (const QVariant& item : m_workloads->processes()) {
        const QVariantMap candidate = item.toMap();
        if (candidate.value("pid").toLongLong() == pid) {
            cached = candidate;
            break;
        }
    }
    const QVariantMap fresh = readLiveIdentity(pid);
    const ActionPreflight verdict = evaluateActionPreflight(cached, fresh, m_thm->available(),
                                    static_cast<qint64>(getpid()),
                                    static_cast<qint64>(getppid()),
                                    static_cast<qint64>(getuid()));
    const QString signalName = force ? QStringLiteral("SIGKILL") : QStringLiteral("SIGTERM");
    QVariantMap result{{"pid", pid}, {"signal", signalName},
                       {"code", verdict.code}, {"reason", verdict.reason},
                       {"executed", false}, {"allowed", false}};
    const QVariantMap audit{{"timestamp_utc", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
                            {"pid", pid}, {"name", cached.value("name").toString()},
                            {"uid", cached.value("uid")}, {"requested_signal", signalName},
                            {"outcome", verdict.code}, {"reason", verdict.reason},
                            {"executed", false}, {"version", QStringLiteral("0.6.0-preview")}};
    const bool logged = appendAudit(audit);
    result.insert("logged", logged);
    result.insert("auditPath", auditLogPath());
    if (!logged)
        result.insert("auditWarning", QStringLiteral("Audit log could not be written"));
    return result;
}
