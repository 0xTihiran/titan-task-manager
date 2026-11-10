#include "systemmodel.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QByteArray>
#include <QHash>
#include <algorithm>

namespace {
QByteArray readText(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return file.readAll();
}
double pressureSome(const QString& path) {
    const auto data = readText(path);
    const auto lines = data.split('\n');
    for (const auto& line: lines) {
        if (!line.startsWith("some ")) continue;
        const auto m=QRegularExpression("avg10=([0-9.]+)").match(QString::fromUtf8(line));
        if (m.hasMatch()) return m.captured(1).toDouble();
    }
    return 0;
}
void appendHistory(QVariantList& list, double value) {
    list.append(value);
    while(list.size()>48) list.removeFirst();
}
}
SystemModel::SystemModel(QObject* parent):QObject(parent) {
    connect(&m_timer,&QTimer::timeout,this,&SystemModel::refresh);
    m_timer.start(1000);
    refresh();
}
void SystemModel::refresh() {
    // /proc/stat first line: cpu user nice system idle iowait irq softirq ...
    const auto line = readText("/proc/stat").split('\n').value(0).simplified();
    const auto fields=line.split(' ');
    if (fields.size()>=5 && fields.value(0)=="cpu") {
        quint64 total=0;
        for (int i=1;i<fields.size();++i) total+=fields[i].toULongLong();
        const quint64 idle=fields[4].toULongLong() + (fields.size()>5?fields[5].toULongLong():0);
        if (m_lastTotal!=0 && total>m_lastTotal) {
            const quint64 dTotal=total-m_lastTotal;
            const quint64 dIdle=idle>=m_lastIdle?idle-m_lastIdle:0;
            m_cpu=100.0*static_cast<double>(dTotal> dIdle ? dTotal-dIdle:0)/dTotal;
        }
        m_lastTotal=total; m_lastIdle=idle; m_available=true;
    } else {
        m_available=false;
    }
    QHash<QByteArray,quint64> mem;
    const auto memlines=readText("/proc/meminfo").split('\n');
    for (const auto& item:memlines) {
        const int colon=item.indexOf(':');
        if (colon<0) continue;
        const auto val=item.mid(colon+1).simplified().split(' ').value(0).toULongLong();
        mem.insert(item.left(colon),val);
    }
    const quint64 total=mem.value("MemTotal");
    const quint64 available=mem.value("MemAvailable");
    if (total>0) {
        const double used=double(total>available?total-available:0);
        m_memory=100.0*used/double(total);
        m_memUsed=used/1048576.0;
        m_memTotal=double(total)/1048576.0;
    }
    const auto load=readText("/proc/loadavg").simplified().split(' ');
    if (!load.isEmpty()) m_load=load.first().toDouble();
    m_cpuPsi=pressureSome("/proc/pressure/cpu");
    m_memPsi=pressureSome("/proc/pressure/memory");
    m_ioPsi=pressureSome("/proc/pressure/io");
    appendHistory(m_cpuHistory,m_cpu);
    appendHistory(m_memHistory,m_memory);
    emit updated();
}
