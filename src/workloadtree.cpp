#include "workloadtree.h"
#include "procmodel.h"
#include "thmclient.h"

WorkloadTree::WorkloadTree(ThmClient* thm, ProcModel* proc, QObject* parent)
  : QObject(parent), m_thm(thm), m_proc(proc) {
    connect(thm, &ThmClient::updated, this, &WorkloadTree::refresh);
    connect(proc, &ProcModel::updated, this, &WorkloadTree::refresh);
    refresh();
}
void WorkloadTree::refresh() {
    m_snapshot = reconcileWorkloads(m_thm->workloads(), m_proc->processes(), m_thm->available());
    emit updated();
}
