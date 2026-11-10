#pragma once
#include <QObject>
#include <QTimer>
#include <QVariantList>

// Linux read-only /proc-based live system metrics.
class SystemModel final : public QObject {
    Q_OBJECT
    Q_PROPERTY(double cpuPercent READ cpuPercent NOTIFY updated)
    Q_PROPERTY(double memoryPercent READ memoryPercent NOTIFY updated)
    Q_PROPERTY(double memoryUsedGiB READ memoryUsedGiB NOTIFY updated)
    Q_PROPERTY(double memoryTotalGiB READ memoryTotalGiB NOTIFY updated)
    Q_PROPERTY(double loadAverage READ loadAverage NOTIFY updated)
    Q_PROPERTY(double cpuPsi READ cpuPsi NOTIFY updated)
    Q_PROPERTY(double memoryPsi READ memoryPsi NOTIFY updated)
    Q_PROPERTY(double ioPsi READ ioPsi NOTIFY updated)
    Q_PROPERTY(QVariantList cpuHistory READ cpuHistory NOTIFY updated)
    Q_PROPERTY(QVariantList memoryHistory READ memoryHistory NOTIFY updated)
    Q_PROPERTY(bool available READ available NOTIFY updated)
public:
    explicit SystemModel(QObject* parent=nullptr);
    double cpuPercent() const {return m_cpu;}
    double memoryPercent() const {return m_memory;}
    double memoryUsedGiB() const {return m_memUsed;}
    double memoryTotalGiB() const {return m_memTotal;}
    double loadAverage() const {return m_load;}
    double cpuPsi() const {return m_cpuPsi;}
    double memoryPsi() const {return m_memPsi;}
    double ioPsi() const {return m_ioPsi;}
    QVariantList cpuHistory() const {return m_cpuHistory;}
    QVariantList memoryHistory() const {return m_memHistory;}
    bool available() const {return m_available;}
    Q_INVOKABLE void refresh();
signals:
    void updated();
private:
    QTimer m_timer;
    quint64 m_lastTotal=0, m_lastIdle=0;
    double m_cpu=0,m_memory=0,m_memUsed=0,m_memTotal=0,m_load=0;
    double m_cpuPsi=0,m_memPsi=0,m_ioPsi=0;
    bool m_available=false;
    QVariantList m_cpuHistory,m_memHistory;
};
