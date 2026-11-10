#pragma once
#include <QObject>
#include <QHash>
#include <QTimer>
#include <QVariantList>
#include <QElapsedTimer>

// Read-only Linux process snapshot; no actions, signals or privilege escalation.
class ProcModel final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList processes READ processes NOTIFY updated)
    Q_PROPERTY(int count READ count NOTIFY updated)
public:
    explicit ProcModel(QObject* parent=nullptr);
    QVariantList processes() const {return m_processes;}
    int count() const {return m_count;}
    Q_INVOKABLE void refresh();
signals:
    void updated();
private:
    struct Sample {quint64 ticks=0, startTime=0;};
    QHash<qint64,Sample> m_previous;
    QVariantList m_processes;
    QElapsedTimer m_clock;
    QTimer m_timer;
    qint64 m_lastMs=0;
    int m_count=0;
};
