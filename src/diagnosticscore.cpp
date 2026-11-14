#include "diagnosticscore.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

namespace {
void add(QVariantList& list, const QString& id, const QString& title,
         const QString& status, const QString& detail) {
    list.append(QVariantMap{{QStringLiteral("id"), id},
                            {QStringLiteral("title"), title},
                            {QStringLiteral("status"), status},
                            {QStringLiteral("detail"), detail}});
}
}

QVariantList buildDiagnosticChecks(const DiagnosticsInput& input) {
    QVariantList checks;
    const QString status = input.thmSourceStatus.trimmed().toUpper();
    if (status == QStringLiteral("LIVE")) {
        add(checks, QStringLiteral("thm"), QStringLiteral("THM telemetry"),
            QStringLiteral("PASS"),
            QStringLiteral("Recent, parseable THM telemetry at the standard path (not authentication)"));
    } else if (status == QStringLiteral("OVERRIDE")) {
        add(checks, QStringLiteral("thm"), QStringLiteral("THM telemetry"),
            QStringLiteral("INFO"),
            QStringLiteral("Recent telemetry from a custom path; source identity unverified"));
    } else if (status == QStringLiteral("DEMO")) {
        add(checks, QStringLiteral("thm"), QStringLiteral("THM telemetry"),
            QStringLiteral("INFO"),
            QStringLiteral("Synthetic fixture mode; not valid evidence of a running THM daemon"));
    } else {
        add(checks, QStringLiteral("thm"), QStringLiteral("THM telemetry"),
            QStringLiteral("WARN"),
            QStringLiteral("No usable THM data: ") + (status.isEmpty() ? QStringLiteral("UNKNOWN") : status));
    }

    add(checks, QStringLiteral("system"), QStringLiteral("CPU and memory collector"),
        input.systemAvailable ? QStringLiteral("PASS") : QStringLiteral("WARN"),
        input.systemAvailable ? QStringLiteral("Linux system metrics are available")
                              : QStringLiteral("CPU/memory metrics currently unavailable"));
    add(checks, QStringLiteral("proc"), QStringLiteral("Linux /proc"),
        input.procfsReadable ? QStringLiteral("PASS") : QStringLiteral("WARN"),
        input.procfsReadable ? QStringLiteral("Process information can be inspected read-only")
                             : QStringLiteral("Required /proc files are not readable"));

    const int readablePsi = int(input.cpuPsiReadable) + int(input.memoryPsiReadable)
                            + int(input.ioPsiReadable);
    add(checks, QStringLiteral("psi"), QStringLiteral("Kernel pressure metrics"),
        readablePsi == 3 ? QStringLiteral("PASS") : QStringLiteral("INFO"),
        readablePsi == 3 ? QStringLiteral("CPU, memory, and I/O PSI are readable")
                         : QStringLiteral("Only %1 of 3 PSI interfaces readable; kernel support varies").arg(readablePsi));

    add(checks, QStringLiteral("cgroup"), QStringLiteral("Cgroup v2"),
        input.cgroupV2Readable ? QStringLiteral("PASS") : QStringLiteral("INFO"),
        input.cgroupV2Readable ? QStringLiteral("Unified cgroup controller file is readable")
                               : QStringLiteral("Unified cgroup controller file unavailable"));
    add(checks, QStringLiteral("decisions"), QStringLiteral("THM decision socket"),
        input.thmDecisionsConnected ? QStringLiteral("PASS") : QStringLiteral("WARN"),
        input.thmDecisionsConnected ? QStringLiteral("Read-only THM decision query has succeeded")
                                    : QStringLiteral("THM decision service is not currently responding"));
    add(checks, QStringLiteral("hyprland"), QStringLiteral("Hyprland workspace events"),
        input.hyprlandConnected ? QStringLiteral("PASS") : QStringLiteral("INFO"),
        input.hyprlandConnected ? QStringLiteral("Workspace event subscription connected")
                                : QStringLiteral("Not connected; some sessions do not use Hyprland"));
    add(checks, QStringLiteral("theme"), QStringLiteral("Official ArchTitan theme"),
        input.officialThemeIntegrated ? QStringLiteral("PASS") : QStringLiteral("BLOCKED"),
        input.officialThemeIntegrated ? QStringLiteral("Official TitanTheme adapter confirmed")
                                      : QStringLiteral("Using a dark preview; official ArchTitan.TitanUI source required"));
    add(checks, QStringLiteral("registry"), QStringLiteral("THM protection registry"),
        input.officialRegistryIntegrated ? QStringLiteral("PASS") : QStringLiteral("BLOCKED"),
        input.officialRegistryIntegrated ? QStringLiteral("Registry integration confirmed")
                                         : QStringLiteral("Official THM registry not imported or verified"));
    // Even an integrated registry is not sufficient to activate signalling:
    // independently reviewed action logic is a separate release gate.
    add(checks, QStringLiteral("actions"), QStringLiteral("Process actions"),
        QStringLiteral("BLOCKED"),
        QStringLiteral("Review-only; no process termination code is enabled"));
    return checks;
}

int countDiagnosticWarnings(const QVariantList& checks) {
    int n = 0;
    for (const QVariant& entry : checks)
        if (entry.toMap().value(QStringLiteral("status")).toString() == QStringLiteral("WARN")) ++n;
    return n;
}

int countDiagnosticBlockers(const QVariantList& checks) {
    int n = 0;
    for (const QVariant& entry : checks)
        if (entry.toMap().value(QStringLiteral("status")).toString() == QStringLiteral("BLOCKED")) ++n;
    return n;
}

QString makeDiagnosticReport(const QVariantList& checks,
                             const QString& generatedAtUtc,
                             const QString& sourceStatus) {
    QJsonArray items;
    for (const QVariant& entry : checks) {
        const QVariantMap check = entry.toMap();
        // Explicit allowlist of fields: never serialize user-supplied paths or
        // process details into a diagnostics support report.
        items.append(QJsonObject{{QStringLiteral("id"), check.value(QStringLiteral("id")).toString()},
                                 {QStringLiteral("title"), check.value(QStringLiteral("title")).toString()},
                                 {QStringLiteral("status"), check.value(QStringLiteral("status")).toString()}});
    }
    QJsonObject report{{QStringLiteral("application"), QStringLiteral("titan-task-manager")},
                       {QStringLiteral("version"), QStringLiteral("0.8.0")},
                       {QStringLiteral("generated_at_utc"), generatedAtUtc},
                       {QStringLiteral("thm_source_status"), sourceStatus},
                       {QStringLiteral("read_only"), true},
                       {QStringLiteral("checks"), items}};
    return QString::fromUtf8(QJsonDocument(report).toJson(QJsonDocument::Indented));
}
