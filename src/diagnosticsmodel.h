#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>

class ThmClient;
class SystemModel;
class DecisionsModel;
class HyprlandMonitor;

// Read-only diagnostics for the existing six-page UI. No daemon control,
// commands, process details, or filesystem writes.
class DiagnosticsModel final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList checks READ checks NOTIFY updated)
    Q_PROPERTY(int warningCount READ warningCount NOTIFY updated)
    Q_PROPERTY(int blockerCount READ blockerCount NOTIFY updated)
    Q_PROPERTY(QString lastCheckedUtc READ lastCheckedUtc NOTIFY updated)
    Q_PROPERTY(QString thmSourceStatus READ thmSourceStatus NOTIFY updated)
public:
    DiagnosticsModel(ThmClient* thm, SystemModel* system,
                     DecisionsModel* decisions, HyprlandMonitor* hyprland,
                     QObject* parent = nullptr);
    QVariantList checks() const { return m_checks; }
    int warningCount() const { return m_warnings; }
    int blockerCount() const { return m_blockers; }
    QString lastCheckedUtc() const { return m_lastCheckedUtc; }
    QString thmSourceStatus() const { return m_sourceStatus; }
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool copyReportToClipboard() const;
signals:
    void updated();
private:
    ThmClient* m_thm = nullptr;
    SystemModel* m_system = nullptr;
    DecisionsModel* m_decisions = nullptr;
    HyprlandMonitor* m_hyprland = nullptr;
    QTimer m_pollTimer;
    QVariantList m_checks;
    int m_warnings = 0;
    int m_blockers = 0;
    QString m_lastCheckedUtc;
    QString m_sourceStatus;
};
