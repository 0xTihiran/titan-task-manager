#pragma once
#include <QObject>
#include <QLocalServer>
#include <QString>

class IpcServer final : public QObject {
    Q_OBJECT
public:
    explicit IpcServer(QObject* parent=nullptr);
    bool listen(const QString& path);
    QString errorString() const;
    static bool sendCommand(const QString& path, const QString& command);
signals:
    void commandReceived(const QString& command);
private:
    QLocalServer m_server;
};
