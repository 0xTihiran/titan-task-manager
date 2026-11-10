#include "procparser.h"
#include <QList>

bool parseProcStat(const QByteArray& raw, ProcStat* result) {
    if (!result) return false;
    const QByteArray line = raw.trimmed();
    const qsizetype open = line.indexOf('(');
    const qsizetype close = line.lastIndexOf(')');
    if (open <= 0 || close <= open || close + 2 >= line.size()) return false;
    bool ok = false;
    const qint64 pid = line.left(open).trimmed().toLongLong(&ok);
    if (!ok || pid <= 0) return false;
    const QByteArray fields = line.mid(close + 1).trimmed().simplified();
    const QList<QByteArray> parts = fields.split(' ');
    // state is field 3 -> parts[0], rss is field 24 -> parts[21].
    if (parts.size() < 22) return false;
    ProcStat parsed;
    parsed.pid = pid;
    parsed.comm = QString::fromLocal8Bit(line.mid(open + 1, close - open - 1));
    parsed.state = QString::fromLatin1(parts.at(0));
    parsed.ppid = parts.at(1).toLongLong(&ok);
    if (!ok || parsed.ppid < 0) return false;
    parsed.utime = parts.at(11).toULongLong(&ok);
    if (!ok) return false;
    parsed.stime = parts.at(12).toULongLong(&ok);
    if (!ok) return false;
    parsed.starttime = parts.at(19).toULongLong(&ok);
    if (!ok) return false;
    parsed.rssPages = parts.at(21).toLongLong(&ok);
    if (!ok) return false;
    *result = parsed;
    return true;
}
