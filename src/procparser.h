#pragma once

#include <QByteArray>
#include <QString>
#include <QtGlobal>

struct ProcStat {
    qint64 pid = 0;
    qint64 ppid = 0;
    QString comm;
    QString state;
    quint64 utime = 0;
    quint64 stime = 0;
    quint64 starttime = 0;
    qint64 rssPages = 0;
};

// Linux /proc/<pid>/stat parser. Handles command names with spaces and ')' chars.
bool parseProcStat(const QByteArray& raw, ProcStat* result);
