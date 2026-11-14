#pragma once
#include <QObject>
#include <QVariantMap>

class WorkloadTree;
class ThmClient;

// v0.6: user-confirmed action REQUEST REVIEW only. No kill(), pidfd or SIGTERM.
// Every path fails closed until official THM registry can be imported/tested.
class ActionController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool actionsEnabled READ actionsEnabled CONSTANT)
    Q_PROPERTY(QString auditLogPath READ auditLogPath CONSTANT)
public:
    ActionController(WorkloadTree* workloads, ThmClient* thm, QObject* parent = nullptr);
    bool actionsEnabled() const { return false; }
    QString auditLogPath() const;
    Q_INVOKABLE QVariantMap reviewTermination(qint64 pid, bool force);
private:
    QVariantMap readLiveIdentity(qint64 pid) const;
    bool appendAudit(const QVariantMap& audit) const;
    WorkloadTree* m_workloads;
    ThmClient* m_thm;
};
