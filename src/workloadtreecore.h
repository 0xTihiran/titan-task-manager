#pragma once

#include <QVariantList>

struct WorkloadSnapshot {
    QVariantList workloads;
    QVariantList processes;
    int matched = 0;
    int unmatched = 0;
};

// Pure, deterministic reconciliation: no OS writes, no assumptions that
// unmatched == safe to terminate, no fabricated children for missing roots.
WorkloadSnapshot reconcileWorkloads(const QVariantList& thmWorkloads,
                                    const QVariantList& processes,
                                    bool telemetryAvailable);
