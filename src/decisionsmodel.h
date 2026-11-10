#pragma once
#include <QObject>
#include <QLocalSocket>
#include <QStringList>
#include <QTimer>

// Retains sanitized nonempty response lines (max 50, bounded length).
QStringList parseThmDecisionLines(const QByteArray& response);

// Queries only the documented *read-only* THM socket command "decisions 50".
// Every connection is asynchronous and has a timeout: the UI never blocks.
class DecisionsModel final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY updated)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY updated)
    Q_PROPERTY(QString socketPath READ socketPath CONSTANT)
    Q_PROPERTY(QStringList lines READ lines NOTIFY updated)
public:
    explicit DecisionsModel(QObject* parent = nullptr,
                            const QString& socketPath = QString());
    bool connected() const { return m_connected; }
    QString errorMessage() const { return m_error; }
    QString socketPath() const { return m_path; }
    QStringList lines() const { return m_lines; }
    Q_INVOKABLE void refresh();
signals:
    void updated();
private:
    void succeed();
    void fail(const QString& reason);
    QLocalSocket m_socket;
    QTimer m_pollTimer;
    QTimer m_deadline;
    QString m_path;
    QByteArray m_response;
    QStringList m_lines;
    QString m_error;
    bool m_busy = false;
    bool m_connected = false;
};
