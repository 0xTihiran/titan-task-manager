#include "slicemodel.h"
#include <QDir>
#include <QFile>
#include <QVariantMap>

namespace {
QString content(const QString& path) {
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly)) return "—";
    return QString::fromUtf8(f.readAll()).trimmed();
}
}
SliceModel::SliceModel(QObject* parent):QObject(parent) {
    connect(&m_timer,&QTimer::timeout,this,&SliceModel::refresh);
    m_timer.start(2000);
    refresh();
}
void SliceModel::refresh() {
    m_slices.clear();
    const QString base="/sys/fs/cgroup/archtitan.slice/";
    const QStringList names={"archtitan-active","archtitan-background","archtitan-idle","archtitan-reclaimable"};
    for (const auto& name:names) {
        QString path=base+name+".slice/";
        if(name=="archtitan-reclaimable" && !QDir(path).exists()) {
            const QString fallback=base+"archtitan-frozen.slice/";
            if(QDir(fallback).exists()) path=fallback;
        }
        const bool exists=QDir(path).exists();
        QVariantMap s;
        s.insert("name",name);
        s.insert("available",exists);
        s.insert("cpuWeight",exists?content(path+"cpu.weight"):"—");
        s.insert("cpuMax",exists?content(path+"cpu.max"):"—");
        s.insert("memoryCurrent",exists?content(path+"memory.current"):"—");
        s.insert("memoryHigh",exists?content(path+"memory.high"):"—");
        s.insert("pidsCurrent",exists?content(path+"pids.current"):"—");
        s.insert("frozen",exists?content(path+"cgroup.freeze"):"—");
        s.insert("populated",exists?content(path+"cgroup.populated"):"—");
        m_slices.append(s);
    }
    emit updated();
}
