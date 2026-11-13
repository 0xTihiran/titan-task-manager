#include "procidentity.h"

#include <QList>
#include <limits>

bool parseProcUids(const QByteArray& status, qint64* realUid, qint64* effectiveUid) {
    if (!realUid || !effectiveUid) return false;
    for (const auto& line : status.split('\n')) {
        if (!line.startsWith("Uid:")) continue;
        const QList<QByteArray> fields = line.mid(4).simplified().split(' ');
        if (fields.size() < 4) return false;
        bool okReal = false, okEffective = false;
        const qulonglong real = fields.at(0).toULongLong(&okReal);
        const qulonglong effective = fields.at(1).toULongLong(&okEffective);
        if (!okReal || !okEffective || real > std::numeric_limits<quint32>::max()
            || effective > std::numeric_limits<quint32>::max()) return false;
        *realUid = static_cast<qint64>(real);
        *effectiveUid = static_cast<qint64>(effective);
        return true;
    }
    return false;
}

bool sameProcessIdentity(const QVariantMap& cached, const ProcStat& current) {
    if (!cached.contains("pid") || !cached.contains("startTime")) return false;
    bool okPid = false, okStart = false;
    const qint64 cachedPid = cached.value("pid").toLongLong(&okPid);
    const quint64 cachedStart = cached.value("startTime").toULongLong(&okStart);
    return okPid && okStart && cachedPid > 1 && cachedPid == current.pid
        && current.starttime != 0 && cachedStart != 0
        && cachedStart == current.starttime;
}
