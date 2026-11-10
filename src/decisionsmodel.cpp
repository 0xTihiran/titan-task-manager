#include "decisionsmodel.h"
#include <QRegularExpression>
#include <QtGlobal>

QStringList parseThmDecisionLines(const QByteArray& response) {
    // THM's decisions command is human-readable, not a stable JSON protocol.
    // Remove terminal control codes and unprintable characters before QML.
    const QRegularExpression ansi(QStringLiteral("\\x1B\\[[0-?]*[ -/]*[@-~]"));
    QStringList lines;
    for (const QByteArray& raw : response.split('\n')) {
        QString line = QString::fromUtf8(raw).remove(ansi).trimmed();
        line.remove(QRegularExpression(QStringLiteral("[\\x00-\\x08\\x0B-\\x1F\\x7F]")));
        if (!line.isEmpty()) lines.append(line.left(512));
        if (lines.size() >= 50) break;
    }
    return lines;
}

DecisionsModel::DecisionsModel(QObject* parent, const QString& socketPath)
  : QObject(parent), m_path(socketPath.isEmpty()
      ? qEnvironmentVariable("THM_SOCKET_PATH", "/tmp/titan_hwm.sock")
      : socketPath) {
    m_deadline.setSingleShot(true);
    connect(&m_deadline, &QTimer::timeout, this, [this] {
        fail(QStringLiteral("THM command timed out"));
    });
    connect(&m_socket, &QLocalSocket::connected, this, [this] {
        if (m_busy) { m_socket.write("decisions 50\n"); m_socket.flush(); }
    });
    connect(&m_socket, &QLocalSocket::readyRead, this, [this] {
        if (!m_busy) return;
        m_response += m_socket.readAll();
        if (m_response.size() > 65536) fail(QStringLiteral("THM response exceeded 64 KiB"));
    });
    connect(&m_socket, &QLocalSocket::disconnected, this, [this] {
        if (!m_busy) return;
        m_response += m_socket.readAll();
        succeed();
    });
    connect(&m_socket, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError) {
        if (m_busy) fail(m_socket.errorString());
    });
    connect(&m_pollTimer, &QTimer::timeout, this, &DecisionsModel::refresh);
    m_pollTimer.start(5000);
    QTimer::singleShot(0, this, &DecisionsModel::refresh);
}
void DecisionsModel::refresh() {
    if (m_busy) return;
    if (m_socket.state() != QLocalSocket::UnconnectedState) m_socket.abort();
    m_busy = true;
    m_response.clear();
    m_deadline.start(1600);
    m_socket.connectToServer(m_path);
}
void DecisionsModel::succeed() {
    m_deadline.stop();
    m_busy = false;
    m_connected = true;
    m_error.clear();
    m_lines = parseThmDecisionLines(m_response);
    emit updated();
}
void DecisionsModel::fail(const QString& reason) {
    m_deadline.stop();
    m_busy = false;
    m_connected = false;
    m_error = reason;
    m_lines.clear(); // No stale historical data should masquerade as live.
    if (m_socket.state() != QLocalSocket::UnconnectedState) m_socket.abort();
    emit updated();
}
