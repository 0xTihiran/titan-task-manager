#pragma once

#include <QObject>
#include <QVariantList>
#include "workloadtreecore.h"

class ThmClient;
class ProcModel;

class WorkloadTree final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList workloads READ workloads NOTIFY updated)
    Q_PROPERTY(QVariantList processes READ processes NOTIFY updated)
    Q_PROPERTY(int matchedCount READ matchedCount NOTIFY updated)
    Q_PROPERTY(int unmatchedCount READ unmatchedCount NOTIFY updated)
public:
    WorkloadTree(ThmClient* thm, ProcModel* proc, QObject* parent = nullptr);
    QVariantList workloads() const {return m_snapshot.workloads;}
    QVariantList processes() const {return m_snapshot.processes;}
    int matchedCount() const {return m_snapshot.matched;}
    int unmatchedCount() const {return m_snapshot.unmatched;}
public slots:
    void refresh();
signals:
    void updated();
private:
    ThmClient* m_thm;
    ProcModel* m_proc;
    WorkloadSnapshot m_snapshot;
};
