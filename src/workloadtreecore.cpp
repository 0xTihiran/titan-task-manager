#include "workloadtreecore.h"

#include <QHash>
#include <QSet>
#include <QVariantMap>
#include <algorithm>

namespace {
qint64 pidOf(const QVariantMap& p) { return p.value("pid").toLongLong(); }
}

WorkloadSnapshot reconcileWorkloads(const QVariantList& thmWorkloads,
                                    const QVariantList& processes,
                                    const bool telemetryAvailable) {
    WorkloadSnapshot result;
    QHash<qint64, QVariantMap> byPid;
    QHash<qint64, QList<qint64>> children;
    for (const QVariant& entry : processes) {
        const QVariantMap p = entry.toMap();
        const qint64 pid = pidOf(p);
        if (pid <= 0) continue;
        byPid.insert(pid, p);
        children[p.value("ppid").toLongLong()].append(pid);
    }
    for (auto it = children.begin(); it != children.end(); ++it)
        std::sort(it.value().begin(), it.value().end());

    // Distinct THM roots are boundaries. The closer root owns a nested subtree.
    QSet<qint64> roots;
    for (const QVariant& entry : thmWorkloads) {
        const qint64 pid = entry.toMap().value("pid").toLongLong();
        if (pid > 0) roots.insert(pid);
    }
    QHash<qint64, int> owner;
    QHash<qint64, int> depths;
    QHash<qint64, qint64> visibleParent;

    // Breadth-first per root avoids cycles and unbounded recursion in odd snapshots.
    if (telemetryAvailable) {
        for (const QVariant& entry : thmWorkloads) {
            QVariantMap workload = entry.toMap();
            const qint64 root = workload.value("pid").toLongLong();
            const bool hasRoot = root > 0 && byPid.contains(root);
            workload.insert("stale", !hasRoot);
            workload.insert("rootPresent", hasRoot);
            workload.insert("matchedCount", 0);
            workload.insert("tree", QVariantList{});
            result.workloads.append(workload);
        }
        // Walk *every* root independently, stopping at another THM root.
        // One process can never be claimed by two workloads.
        for (int w = 0; w < result.workloads.size(); ++w) {
            const QVariantMap workload = result.workloads.at(w).toMap();
            const qint64 root = workload.value("pid").toLongLong();
            if (!byPid.contains(root) || owner.contains(root)) continue;
            QList<qint64> pending{root};
            QHash<qint64,int> localDepth;
            QSet<qint64> visited;
            localDepth.insert(root,0);
            for (qsizetype index = 0; index < pending.size(); ++index) {
                const qint64 current = pending.at(index);
                if (visited.contains(current)) continue;
                visited.insert(current);
                if (current != root && roots.contains(current)) continue;
                if (owner.contains(current)) continue;
                if (!byPid.contains(current)) continue;
                owner.insert(current,w);
                depths.insert(current,localDepth.value(current));
                for (const qint64 child : children.value(current)) {
                    if (child == current || visited.contains(child)) continue;
                    // Avoid following a child whose process snapshot is older
                    // than its parent (protection against impossible lineage).
                    const quint64 parentStart = byPid.value(current).value("startTime").toULongLong();
                    const quint64 childStart = byPid.value(child).value("startTime").toULongLong();
                    if (parentStart && childStart && childStart < parentStart) continue;
                    if (!localDepth.contains(child)) localDepth.insert(child,localDepth.value(current)+1);
                    if (!visibleParent.contains(child)) visibleParent.insert(child,current);
                    pending.append(child);
                }
            }
        }
        // Build tree in stable parent-first order; depth is a UI indentation aid.
        for (int w = 0; w < result.workloads.size(); ++w) {
            QVariantMap workload = result.workloads[w].toMap();
            const qint64 root = workload.value("pid").toLongLong();
            QVariantList tree;
            if (owner.value(root,-1) == w) {
                QList<qint64> queue{root};
                QSet<qint64> emitted;
                for (qsizetype i=0; i < queue.size(); ++i) {
                    const qint64 pid = queue.at(i);
                    if (emitted.contains(pid) || owner.value(pid,-1) != w) continue;
                    emitted.insert(pid);
                    QVariantMap process = byPid.value(pid);
                    process.insert("depth",depths.value(pid,0));
                    process.insert("isRoot",pid == root);
                    tree.append(process);
                    for (const qint64 child: children.value(pid)) {
                        if (owner.value(child,-1)==w && visibleParent.value(child,-1)==pid)
                            queue.append(child);
                    }
                }
            }
            workload.insert("tree",tree);
            workload.insert("matchedCount",tree.size());
            result.workloads[w] = workload;
        }
    }
    for (const QVariant& entry : processes) {
        QVariantMap proc = entry.toMap();
        const qint64 pid = pidOf(proc);
        const int index = owner.value(pid, -1);
        const bool matched = telemetryAvailable && index >= 0;
        if (matched) {
            const QVariantMap workload = result.workloads[index].toMap();
            proc.insert("workloadId", workload.value("id"));
            proc.insert("workloadName", workload.value("name"));
            proc.insert("workloadType", workload.value("type"));
            proc.insert("workloadObserved", workload.value("observed"));
            proc.insert("assignment", "THM managed");
            ++result.matched;
        } else {
            proc.insert("workloadId", 0);
            proc.insert("workloadName", "");
            proc.insert("workloadType", "");
            proc.insert("workloadObserved", false);
            // Never claim a process is safely unmanaged based solely on ancestry.
            proc.insert("assignment", telemetryAvailable ? "Unmatched" : "THM unavailable");
            ++result.unmatched;
        }
        proc.insert("canTerminate",false);
        proc.insert("terminateReason","Read-only: a complete protection check is required");
        result.processes.append(proc);
    }
    return result;
}
