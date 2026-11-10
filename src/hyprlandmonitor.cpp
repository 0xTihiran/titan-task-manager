#include "hyprlandmonitor.h"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QtGlobal>
#include <algorithm>

int parseHyprlandWorkspaceEvent(const QByteArray& line) {
    QByteArray payload;
    if (line.startsWith("workspacev2>>")) payload = line.mid(sizeof("workspacev2>>")-1);
    else if (line.startsWith("workspace>>")) payload = line.mid(sizeof("workspace>>")-1);
    else if (line.startsWith("focusedmon>>")) {
        const QByteArray details = line.mid(sizeof("focusedmon>>")-1);
        const int comma = details.indexOf(',');
        if (comma < 0) return -1;
        payload = details.mid(comma+1);
    } else return -1;
    // workspacev2 uses "id,name", workspace uses "name" which may be numeric.
    const QByteArray number = payload.trimmed().split(',').value(0);
    bool ok = false;
    const int id = number.toInt(&ok);
    return ok && id > 0 ? id : -1;
}

HyprlandMonitor::HyprlandMonitor(QObject* parent, const QString& socketOverride)
  : QObject(parent), m_override(socketOverride) {
    m_retry.setSingleShot(true);
    connect(&m_retry, &QTimer::timeout, this, &HyprlandMonitor::reconnect);
    connect(&m_socket, &QLocalSocket::connected, this, [this] {
        m_connected = true;
        m_error.clear();
        m_backoffMs = 1000;
        emit updated();
    });
    connect(&m_socket, &QLocalSocket::readyRead, this, &HyprlandMonitor::receive);
    connect(&m_socket, &QLocalSocket::disconnected, this, [this] {
        m_connected = false;
        m_focusedWorkspace = -1; // Don't imply a stale focused workspace.
        m_error = QStringLiteral("Hyprland socket disconnected");
        emit updated();
        scheduleReconnect();
    });
    connect(&m_socket, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError) {
        m_connected = false;
        m_focusedWorkspace = -1;
        m_error = m_socket.errorString();
        emit updated();
        scheduleReconnect();
    });
    QTimer::singleShot(0, this, &HyprlandMonitor::reconnect);
}
QString HyprlandMonitor::discoverSocket() const {
    if (!m_override.isEmpty()) return m_override;
    const QString runtime = qEnvironmentVariable("XDG_RUNTIME_DIR");
    const QString signature = qEnvironmentVariable("HYPRLAND_INSTANCE_SIGNATURE");
    if (runtime.isEmpty()) return {};
    const QString hyprDir = QDir(runtime).filePath("hypr");
    if (!signature.isEmpty()) {
        const QString preferred = QDir(hyprDir).filePath(signature + "/.socket2.sock");
        if (QFileInfo::exists(preferred)) return preferred;
    }
    // May be multiple sessions; select newest socket as the documented fallback.
    const QDir root(hyprDir);
    QString newest;
    QDateTime newestTime;
    for (const QString& entry: root.entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
        const QString file = root.filePath(entry + "/.socket2.sock");
        const QFileInfo info(file);
        if (!info.exists()) continue;
        if (newest.isEmpty() || info.lastModified() > newestTime) {
            newest = file;
            newestTime = info.lastModified();
        }
    }
    return newest;
}
void HyprlandMonitor::reconnect() {
    m_retry.stop();
    if (m_socket.state() == QLocalSocket::ConnectedState ||
        m_socket.state() == QLocalSocket::ConnectingState) return;
    m_socket.abort();
    m_buffer.clear();
    m_path = discoverSocket();
    if (m_path.isEmpty()) {
        m_error = QStringLiteral("Hyprland event socket not found");
        m_connected = false;
        emit updated();
        scheduleReconnect();
        return;
    }
    m_socket.connectToServer(m_path);
    emit updated();
}
void HyprlandMonitor::scheduleReconnect() {
    if (m_retry.isActive()) return;
    m_retry.start(m_backoffMs);
    m_backoffMs = std::min(m_backoffMs * 2, 16000);
}
void HyprlandMonitor::receive() {
    m_buffer += m_socket.readAll();
    if (m_buffer.size() > 16384) { m_buffer.clear(); return; } // Bound memory.
    int split;
    while ((split = m_buffer.indexOf('\n')) >= 0) {
        const QByteArray line = m_buffer.left(split).trimmed();
        m_buffer.remove(0, split+1);
        const int focused = parseHyprlandWorkspaceEvent(line);
        if (focused > 0 && focused != m_focusedWorkspace) {
            m_focusedWorkspace = focused;
            emit updated();
        }
    }
}
