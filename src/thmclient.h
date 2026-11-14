#pragma once
#include <QObject>
#include <QTimer>
#include <QVariantList>

// Read-only, fail-closed THM file adapter. Official titan-sysmon parser and
// ArchTitan.TitanUI are still awaiting the full ArchTitan repository.
class ThmClient final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY updated)
    Q_PROPERTY(QString sourcePath READ sourcePath CONSTANT)
    Q_PROPERTY(QString sourceStatus READ sourceStatus NOTIFY updated)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY updated)
    Q_PROPERTY(int telemetryAgeSeconds READ telemetryAgeSeconds NOTIFY updated)
    Q_PROPERTY(QString version READ version NOTIFY updated)
    Q_PROPERTY(QString pressureLevel READ pressureLevel NOTIFY updated)
    Q_PROPERTY(double memoryUsedPct READ memoryUsedPct NOTIFY updated)
    Q_PROPERTY(int workloadCount READ workloadCount NOTIFY updated)
    Q_PROPERTY(int activeWorkloads READ activeWorkloads NOTIFY updated)
    Q_PROPERTY(int seenProcesses READ seenProcesses NOTIFY updated)
    Q_PROPERTY(int unmanagedProcesses READ unmanagedProcesses NOTIFY updated)
    Q_PROPERTY(QString governor READ governor NOTIFY updated)
    Q_PROPERTY(QVariantList workloads READ workloads NOTIFY updated)
    Q_PROPERTY(QVariantList topUnmanaged READ topUnmanaged NOTIFY updated)
    Q_PROPERTY(QString lastDecision READ lastDecision NOTIFY updated)
public:
    explicit ThmClient(QObject* parent=nullptr);
    bool available() const { return m_available; }
    QString sourcePath() const { return m_sourcePath; }
    QString sourceStatus() const { return m_sourceStatus; }
    QString errorMessage() const { return m_error; }
    int telemetryAgeSeconds() const { return m_ageSeconds; }
    QString version() const { return m_version; }
    QString pressureLevel() const { return m_pressureLevel; }
    double memoryUsedPct() const { return m_memoryUsedPct; }
    int workloadCount() const { return m_workloadCount; }
    int activeWorkloads() const { return m_activeWorkloads; }
    int seenProcesses() const { return m_seenProcesses; }
    int unmanagedProcesses() const { return m_unmanagedProcesses; }
    QString governor() const { return m_governor; }
    QVariantList workloads() const { return m_workloads; }
    QVariantList topUnmanaged() const { return m_topUnmanaged; }
    QString lastDecision() const { return m_lastDecision; }
    Q_INVOKABLE void refresh();
signals:
    void updated();
private:
    void clearSnapshot();
    QString m_sourcePath;
    QTimer m_timer;
    bool m_available = false;
    QString m_sourceStatus = QStringLiteral("MISSING");
    QString m_error, m_version, m_pressureLevel, m_governor, m_lastDecision;
    int m_ageSeconds = -1;
    double m_memoryUsedPct = 0.0;
    int m_workloadCount = 0, m_activeWorkloads = 0;
    int m_seenProcesses = 0, m_unmanagedProcesses = 0;
    QVariantList m_workloads, m_topUnmanaged;
};
