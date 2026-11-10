#include "ipcserver.h"
#include <QAbstractSocket>
#include <QByteArray>
#include <QLocalSocket>

IpcServer::IpcServer(QObject* parent): QObject(parent) {
    m_server.setSocketOptions(QLocalServer::UserAccessOption);
    connect(&m_server, &QLocalServer::newConnection, this, [this] {
        while (QLocalSocket* client = m_server.nextPendingConnection()) {
            connect(client, &QLocalSocket::readyRead, client, [this, client] {
                while (client->canReadLine()) {
                    const QString cmd=QString::fromUtf8(client->readLine()).trimmed().toUpper();
                    if (cmd == "SHOW" || cmd == "HIDE" || cmd == "TOGGLE" || cmd == "RAISE")
                        emit commandReceived(cmd);
                }
            });
            connect(client, &QLocalSocket::disconnected, client, &QObject::deleteLater);
        }
    });
}
bool IpcServer::listen(const QString& path) {
    if (m_server.listen(path)) return true;
    if (m_server.serverError() != QAbstractSocket::AddressInUseError) return false;
    QLocalSocket probe;
    probe.connectToServer(path);
    if (probe.waitForConnected(800)) return false; // Another instance owns it.
    // Stale socket left behind by a crashed instance; private runtime directory only.
    if (!QLocalServer::removeServer(path)) return false;
    return m_server.listen(path);
}
QString IpcServer::errorString() const { return m_server.errorString(); }
bool IpcServer::sendCommand(const QString& path, const QString& cmd) {
    QLocalSocket socket;
    socket.connectToServer(path);
    if (!socket.waitForConnected(500)) return false;
    socket.write((cmd + "\n").toUtf8());
    return socket.waitForBytesWritten(500);
}
