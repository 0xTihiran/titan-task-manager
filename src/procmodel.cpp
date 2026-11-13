#include "procmodel.h"
#include "procparser.h"
#include "procidentity.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QVariantMap>
#include <algorithm>
#include <utility>
#include <unistd.h>

namespace {
QString readLimitedText(const QString& path, qint64 limit = 2048) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QString::fromLocal8Bit(file.read(limit)).trimmed();
}
}

ProcModel::ProcModel(QObject* parent): QObject(parent) {
    m_clock.start();
    connect(&m_timer, &QTimer::timeout, this, &ProcModel::refresh);
    m_timer.start(1500);
    refresh();
}

void ProcModel::refresh() {
    const qint64 nowMs = m_clock.elapsed();
    const double elapsed = double(nowMs - m_lastMs) / 1000.0;
    m_lastMs = nowMs;
    const long hz = sysconf(_SC_CLK_TCK);
    const long pageSize = sysconf(_SC_PAGESIZE);
    QVariantList found;
    QHash<qint64, Sample> next;

    const auto dirs = QDir("/proc").entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& dir : dirs) {
        bool validPid = false;
        const qint64 pid = dir.toLongLong(&validPid);
        if (!validPid || pid <= 0) continue;
        const QString path = QStringLiteral("/proc/") + dir;
        QFile statFile(path + QStringLiteral("/stat"));
        if (!statFile.open(QIODevice::ReadOnly)) continue;
        ProcStat stat;
        if (!parseProcStat(statFile.readAll(), &stat) || stat.pid != pid) continue;

        const quint64 ticks = stat.utime + stat.stime;
        const auto old = m_previous.constFind(pid);
        double cpu = 0.0;
        if (old != m_previous.cend() && old->startTime == stat.starttime
            && ticks >= old->ticks && hz > 0 && elapsed > 0) {
            cpu = 100.0 * (double(ticks - old->ticks) / double(hz)) / elapsed;
        }
        next.insert(pid, {ticks, stat.starttime});
        const double memory = pageSize > 0 && stat.rssPages > 0
            ? double(stat.rssPages) * double(pageSize) / 1048576.0 : 0.0;
        const QString command = readLimitedText(path + QStringLiteral("/cmdline"), 4096).replace(QChar::Null, ' ');
        QVariantMap item;
        item.insert("pid", pid);
        item.insert("ppid", stat.ppid);
        item.insert("name", stat.comm);
        item.insert("state", stat.state);
        item.insert("cpu", cpu);
        item.insert("memoryMiB", memory);
        item.insert("rssKb", pageSize > 0 && stat.rssPages > 0 ? stat.rssPages * (pageSize / 1024) : 0);
        qint64 realUid = -1;
        qint64 effectiveUid = -1;
        QFile statusFile(path + QStringLiteral("/status"));
        const bool uidVerified = statusFile.open(QIODevice::ReadOnly)
            && parseProcUids(statusFile.read(16384), &realUid, &effectiveUid);
        item.insert("uid", realUid);
        item.insert("effectiveUid", effectiveUid);
        item.insert("uidVerified", uidVerified);
        item.insert("startTime", QVariant::fromValue(stat.starttime));
        item.insert("exe", QFileInfo(path + QStringLiteral("/exe")).symLinkTarget());
        item.insert("cwd", QFileInfo(path + QStringLiteral("/cwd")).symLinkTarget());
        item.insert("cmdline", command);
        item.insert("cgroup", readLimitedText(path + QStringLiteral("/cgroup"), 2048));
        found.append(item);
    }
    std::sort(found.begin(), found.end(), [](const QVariant& a, const QVariant& b) {
        const auto ma = a.toMap(), mb = b.toMap();
        if (ma.value("cpu").toDouble() != mb.value("cpu").toDouble())
            return ma.value("cpu").toDouble() > mb.value("cpu").toDouble();
        return ma.value("memoryMiB").toDouble() > mb.value("memoryMiB").toDouble();
    });
    m_count = found.size();
    m_processes = std::move(found);
    m_previous = std::move(next);
    emit updated();
}
