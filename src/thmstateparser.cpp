#include "thmstateparser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <cmath>
#include <limits>
#include <utility>

namespace {
bool reject(QString* error, const QString& message) {
    if (error) *error = message;
    return false;
}

bool nonnegativeInt(const QJsonObject& obj, const QString& key, int* result) {
    const QJsonValue v = obj.value(key);
    if (!v.isDouble()) return false;
    const double d = v.toDouble();
    if (!std::isfinite(d) || d < 0.0 ||
        d > static_cast<double>(std::numeric_limits<int>::max()) ||
        std::trunc(d) != d) return false;
    *result = static_cast<int>(d);
    return true;
}
}

bool parseThmState(const QByteArray& bytes, ThmSnapshot* out, QString* error) {
    if (error) error->clear();
    if (!out) return reject(error, QStringLiteral("Null snapshot destination"));
    constexpr qsizetype maxBytes = 4 * 1024 * 1024;
    if (bytes.isEmpty() || bytes.size() > maxBytes)
        return reject(error, QStringLiteral("THM payload is empty or exceeds 4 MiB"));

    QJsonParseError jsonError;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
        return reject(error, QStringLiteral("Malformed THM JSON: %1")
                             .arg(jsonError.errorString()));
    if (!doc.isObject())
        return reject(error, QStringLiteral("THM JSON root is not an object"));

    const QJsonObject root = doc.object();
    if (!root.value(QStringLiteral("pressure")).isObject() ||
        !root.value(QStringLiteral("workloads")).isArray() ||
        !root.value(QStringLiteral("processes")).isObject())
        return reject(error, QStringLiteral("Missing THM pressure, processes, or workloads"));

    ThmSnapshot next;
    next.version = root.value(QStringLiteral("version")).toString();
    if (next.version.isEmpty())
        return reject(error, QStringLiteral("Missing THM version"));

    const QJsonObject pressure = root.value(QStringLiteral("pressure")).toObject();
    next.pressureLevel = pressure.value(QStringLiteral("level")).toString();
    const QSet<QString> levels{
        QStringLiteral("NORMAL"), QStringLiteral("MODERATE"),
        QStringLiteral("HIGH"), QStringLiteral("CRITICAL")
    };
    if (!levels.contains(next.pressureLevel))
        return reject(error, QStringLiteral("Unrecognized THM pressure level"));

    if (!pressure.value(QStringLiteral("mem_used_pct")).isDouble())
        return reject(error, QStringLiteral("Missing or invalid memory percentage"));
    next.memoryUsedPct = pressure.value(QStringLiteral("mem_used_pct")).toDouble();
    if (!std::isfinite(next.memoryUsedPct) ||
        next.memoryUsedPct < 0.0 || next.memoryUsedPct > 100.0)
        return reject(error, QStringLiteral("Memory percentage is out of range"));

    const QJsonObject processes = root.value(QStringLiteral("processes")).toObject();
    if (!nonnegativeInt(root, QStringLiteral("workload_count"), &next.workloadCount) ||
        !nonnegativeInt(root, QStringLiteral("active_workloads"), &next.activeWorkloads) ||
        !nonnegativeInt(processes, QStringLiteral("seen"), &next.seenProcesses) ||
        !nonnegativeInt(processes, QStringLiteral("unmanaged"), &next.unmanagedProcesses))
        return reject(error, QStringLiteral("Invalid THM count fields"));
    if (next.activeWorkloads > next.workloadCount ||
        next.unmanagedProcesses > next.seenProcesses)
        return reject(error, QStringLiteral("Inconsistent THM process/workload counts"));

    const QJsonArray workloads = root.value(QStringLiteral("workloads")).toArray();
    if (workloads.size() != next.workloadCount)
        return reject(error, QStringLiteral("Workload count does not match workloads array"));
    for (const QJsonValue& value : workloads) {
        if (!value.isObject())
            return reject(error, QStringLiteral("Non-object THM workload entry"));
        const QJsonObject entry = value.toObject();
        int id = 0, pid = 0, count = 0;
        if (!nonnegativeInt(entry, QStringLiteral("id"), &id) ||
            !nonnegativeInt(entry, QStringLiteral("pid"), &pid) ||
            !nonnegativeInt(entry, QStringLiteral("pids"), &count) ||
            !entry.value(QStringLiteral("name")).isString() ||
            !entry.value(QStringLiteral("type")).isString() ||
            !entry.value(QStringLiteral("state")).isString() ||
            !entry.value(QStringLiteral("observed")).isBool())
            return reject(error, QStringLiteral("Invalid THM workload entry fields"));
        next.workloads.append(entry.toVariantMap());
    }

    const QJsonValue top = root.value(QStringLiteral("top_unmanaged"));
    if (!top.isUndefined() && !top.isNull() && !top.isArray())
        return reject(error, QStringLiteral("Invalid top_unmanaged array"));
    if (top.isArray()) {
        for (const QJsonValue& value : top.toArray()) {
            if (!value.isObject())
                return reject(error, QStringLiteral("Invalid top_unmanaged entry"));
            next.topUnmanaged.append(value.toObject().toVariantMap());
        }
    }
    next.governor = root.value(QStringLiteral("governor")).toString();
    const QJsonValue last = root.value(QStringLiteral("last_action"));
    if (!last.isUndefined() && !last.isNull() && !last.isObject())
        return reject(error, QStringLiteral("Invalid last_action object"));
    if (last.isObject()) {
        const QJsonObject a = last.toObject();
        next.lastDecision = QStringLiteral("%1  •  %2  •  PID %3")
            .arg(a.value(QStringLiteral("decision")).toString(QStringLiteral("THM decision")),
                 a.value(QStringLiteral("target")).toString(QStringLiteral("unspecified")))
            .arg(a.value(QStringLiteral("pid")).toInt());
    }

    *out = std::move(next);
    return true;
}

bool thmFileIsFresh(const qint64 modifiedMs, const qint64 nowMs,
                    const qint64 maxAgeMs, const qint64 futureSkewMs) {
    if (modifiedMs <= 0 || nowMs <= 0 || maxAgeMs <= 0 || futureSkewMs < 0)
        return false;
    const qint64 age = nowMs - modifiedMs;
    return age >= -futureSkewMs && age <= maxAgeMs;
}
