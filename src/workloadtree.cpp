#include "workloadtree.h"
#include "procmodel.h"
#include "thmclient.h"
#include "protectionfilter.h"
#include <unistd.h>

WorkloadTree::WorkloadTree(ThmClient* thm, ProcModel* proc, QObject* parent)
  : QObject(parent), m_thm(thm), m_proc(proc) {
    connect(thm, &ThmClient::updated, this, &WorkloadTree::refresh);
    connect(proc, &ProcModel::updated, this, &WorkloadTree::refresh);
    refresh();
}
void WorkloadTree::refresh() {
    m_snapshot = reconcileWorkloads(m_thm->workloads(), m_proc->processes(), m_thm->available());
    const qint64 selfPid = static_cast<qint64>(getpid());
    const qint64 parentPid = static_cast<qint64>(getppid());
    const qint64 uid = static_cast<qint64>(getuid());
    const bool telemetryAvailable = m_thm->available();
    for (auto& entry : m_snapshot.processes) {
        QVariantMap process = entry.toMap();
        const ProtectionAssessment assessment = assessProcessProtection(
            process, telemetryAvailable, selfPid, parentPid, uid);
        process.insert("isProtected", assessment.protectedProcess);
        process.insert("protectionStatus", assessment.status);
        process.insert("terminateReason", assessment.reason);
        process.insert("canTerminate", assessment.canTerminate); // always false
        entry = process;
    }
    // Apply identical read-only markings to workload-card process trees.
    for (auto& entry : m_snapshot.workloads) {
        QVariantMap workload = entry.toMap();
        QVariantList tree = workload.value("tree").toList();
        for (auto& child : tree) {
            QVariantMap process = child.toMap();
            process.insert("assignment", QStringLiteral("THM managed"));
            process.insert("workloadId", workload.value("id"));
            const auto assessment = assessProcessProtection(
                process, telemetryAvailable, selfPid, parentPid, uid);
            process.insert("isProtected", assessment.protectedProcess);
            process.insert("protectionStatus", assessment.status);
            process.insert("terminateReason", assessment.reason);
            process.insert("canTerminate", false);
            child = process;
        }
        workload.insert("tree", tree);
        entry = workload;
    }
    emit updated();
}
