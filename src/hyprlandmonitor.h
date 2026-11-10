#pragma once
#include <QObject>
#include <QLocalSocket>
#include <QTimer>
#include <QString>

// Returns the numeric workspace id of a supported Hyprland event, or -1.
int parseHyprlandWorkspaceEvent(const QByteArray& line);

// Passive, nonblocking Hyprland socket2 event subscriber. Never invokes hyprctl.
class HyprlandMonitor final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY updated)
    Q_PROPERTY(int focusedWorkspace READ focusedWorkspace NOTIFY updated)
    Q_PROPERTY(QString socketPath READ socketPath NOTIFY updated)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY updated)
public:
    explicit HyprlandMonitor(QObject* parent = nullptr,
                             const QString& socketOverride = QString());
    bool connected() const { return m_connected; }
    int focusedWorkspace() const { return m_focusedWorkspace; }
    QString socketPath() const { return m_path; }
    QString errorMessage() const { return m_error; }
    Q_INVOKABLE void reconnect();
signals:
    void updated();
private:
    QString discoverSocket() const;
    void scheduleReconnect();
    void receive();
    QLocalSocket m_socket;
    QTimer m_retry;
    QString m_override;
    QString m_path;
    QString m_error;
    QByteArray m_buffer;
    bool m_connected = false;
    int m_focusedWorkspace = -1;
    int m_backoffMs = 1000;
};
