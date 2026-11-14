#include "diagnosticsmodel.h"
#include "diagnosticscore.h"
#include "decisionsmodel.h"
#include "hyprlandmonitor.h"
#include "systemmodel.h"
#include "thmclient.h"

#include <QClipboard>
#include <QDateTime>
#include <QFileInfo>
#include <QGuiApplication>

DiagnosticsModel::DiagnosticsModel(ThmClient* thm, SystemModel* system,
                                   DecisionsModel* decisions, HyprlandMonitor* hyprland,
                                   QObject* parent)
    : QObject(parent), m_thm(thm), m_system(system),
      m_decisions(decisions), m_hyprland(hyprland) {
    connect(m_thm, &ThmClient::updated, this, &DiagnosticsModel::refresh);
    connect(m_system, &SystemModel::updated, this, &DiagnosticsModel::refresh);
    connect(m_decisions, &DecisionsModel::updated, this, &DiagnosticsModel::refresh);
    connect(m_hyprland, &HyprlandMonitor::updated, this, &DiagnosticsModel::refresh);
    connect(&m_pollTimer, &QTimer::timeout, this, &DiagnosticsModel::refresh);
    m_pollTimer.start(8000);
    refresh();
}

void DiagnosticsModel::refresh() {
    const auto readable = [](const QString& path) {
        return QFileInfo(path).isReadable();
    };
    DiagnosticsInput input;
    input.thmSourceStatus = m_thm->sourceStatus();
    input.systemAvailable = m_system->available();
    input.procfsReadable = readable(QStringLiteral("/proc/stat"))
                           && readable(QStringLiteral("/proc/self/status"));
    input.cpuPsiReadable = readable(QStringLiteral("/proc/pressure/cpu"));
    input.memoryPsiReadable = readable(QStringLiteral("/proc/pressure/memory"));
    input.ioPsiReadable = readable(QStringLiteral("/proc/pressure/io"));
    input.cgroupV2Readable = readable(QStringLiteral("/sys/fs/cgroup/cgroup.controllers"));
    input.thmDecisionsConnected = m_decisions->connected();
    input.hyprlandConnected = m_hyprland->connected();
    // Do NOT claim official integration before inspecting the real OS repo.
    input.officialThemeIntegrated = false;
    input.officialRegistryIntegrated = false;

    m_checks = buildDiagnosticChecks(input);
    m_warnings = countDiagnosticWarnings(m_checks);
    m_blockers = countDiagnosticBlockers(m_checks);
    m_sourceStatus = input.thmSourceStatus;
    m_lastCheckedUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    emit updated();
}

bool DiagnosticsModel::copyReportToClipboard() const {
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (!clipboard) return false;
    clipboard->setText(makeDiagnosticReport(m_checks, m_lastCheckedUtc, m_sourceStatus));
    return true;
}
