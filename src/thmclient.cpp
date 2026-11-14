#include "thmclient.h"
#include "thmstateparser.h"
#include "thmstate.hpp" // From the repository-owned titan-sysmon subsystem

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QtGlobal>
#include <limits>
#include <utility>

ThmClient::ThmClient(QObject* parent) : QObject(parent) {
    m_sourcePath = qEnvironmentVariable("THM_TELEMETRY_PATH", "/run/archtitan/thm-state.json");
    connect(&m_timer, &QTimer::timeout, this, &ThmClient::refresh);
    m_timer.start(1000);
    refresh();
}

void ThmClient::clearSnapshot() {
    m_available = false;
    m_version.clear(); m_pressureLevel.clear(); m_governor.clear();
    m_lastDecision.clear(); m_memoryUsedPct = 0.0;
    m_workloadCount = 0; m_activeWorkloads = 0;
    m_seenProcesses = 0; m_unmanagedProcesses = 0;
    m_workloads.clear(); m_topUnmanaged.clear();
}

void ThmClient::refresh() {
    clearSnapshot();
    m_error.clear();
    m_sourceStatus = QStringLiteral("MISSING");
    m_ageSeconds = -1;

    QFile file(m_sourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_error = QStringLiteral("THM telemetry unavailable: %1").arg(file.errorString());
        emit updated();
        return;
    }

    const QFileInfo info(file);
    const qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    const qint64 modified = info.lastModified().toUTC().toMSecsSinceEpoch();
    const qint64 age = now - modified;
    if (modified > 0 && age >= 0)
        m_ageSeconds = static_cast<int>(qMin(age / 1000, qint64(std::numeric_limits<int>::max())));

    // File age is independent of the timestamp_ms payload's clock convention.
    // Testing-only escape hatch is *not* allowed for the default live path.
    const bool developerFixture = m_sourcePath != QStringLiteral("/run/archtitan/thm-state.json")
        && qEnvironmentVariable("THM_TELEMETRY_ALLOW_STALE_FIXTURE") == QStringLiteral("1");
    if (!thmFileIsFresh(modified, now) && !developerFixture) {
        m_sourceStatus = QStringLiteral("STALE");
        m_error = QStringLiteral("THM telemetry file is stale (more than 10 seconds old or clock skew detected)");
        emit updated();
        return;
    }

    constexpr qint64 maxPayloadSize = 4 * 1024 * 1024;
    const QByteArray bytes = file.read(maxPayloadSize + 1);
    ThmSnapshot snapshot;
    QString parseError;
    if (!parseThmState(bytes, &snapshot, &parseError)) {
        m_sourceStatus = QStringLiteral("INVALID");
        m_error = parseError;
        emit updated();
        return;
    }

    // Reuse the exact ArchTitan sysmon reader. THM writes via atomic rename:
    // a rare file replacement between the guarded and native reads must fail
    // closed, not combine mismatched generations.
    const ThmState native = ThmReader::read(m_sourcePath);
    if (!native.available || native.version != snapshot.version ||
        native.workloadCount != snapshot.workloadCount ||
        native.activeWorkloads != snapshot.activeWorkloads ||
        native.workloads.size() != snapshot.workloads.size()) {
        m_sourceStatus = QStringLiteral("INVALID");
        m_error = QStringLiteral("THM snapshot changed or native parser disagrees; retrying");
        emit updated();
        return;
    }

    QVariantList mapped;
    mapped.reserve(native.workloads.size());
    for (qsizetype i=0; i<native.workloads.size(); ++i) {
        const QVariantMap item = native.workloads.at(i).toMap();
        const QVariantMap validated = snapshot.workloads.at(i).toMap();
        if (item.value(QStringLiteral("id")).toInt() != validated.value(QStringLiteral("id")).toInt() ||
            item.value(QStringLiteral("pid")).toInt() != validated.value(QStringLiteral("pid")).toInt()) {
            m_sourceStatus = QStringLiteral("INVALID");
            m_error = QStringLiteral("Native and guarded workload identity differs; retrying");
            emit updated();
            return;
        }
        QVariantMap normalized = item;
        // titan-sysmon exports camelCase, but the Task Manager pages and THM
        // publisher use snake_case. Preserve *both* without guessing fields.
        normalized.insert(QStringLiteral("mem_mb"), item.value(QStringLiteral("memMb")));
        normalized.insert(QStringLiteral("cpu_pct"), item.value(QStringLiteral("cpuPct")));
        mapped.append(normalized);
    }
    m_version = native.version;
    m_pressureLevel = native.pressureLevel;
    m_governor = native.governor;
    m_lastDecision = snapshot.lastDecision;
    m_memoryUsedPct = native.memUsedPct;
    m_workloadCount = native.workloadCount;
    m_activeWorkloads = native.activeWorkloads;
    m_seenProcesses = native.processesSeen;
    m_unmanagedProcesses = native.processesUnmanaged;
    m_workloads = std::move(mapped);
    m_topUnmanaged = std::move(snapshot.topUnmanaged);
    m_available = true;
    m_sourceStatus = developerFixture ? QStringLiteral("DEMO") : QStringLiteral("LIVE");
    emit updated();
}
