#include "thmclient.h"
#include "thmstateparser.h"

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
    // A custom path is useful for real deployments and for fixture testing, but
    // it must not be labelled as the default, verified production telemetry.
    const bool customSource = m_sourcePath != QStringLiteral("/run/archtitan/thm-state.json");
    // Testing-only escape hatch is *not* allowed for the default live path.
    const bool developerFixture = customSource
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

    m_version = snapshot.version;
    m_pressureLevel = snapshot.pressureLevel;
    m_governor = snapshot.governor;
    m_lastDecision = snapshot.lastDecision;
    m_memoryUsedPct = snapshot.memoryUsedPct;
    m_workloadCount = snapshot.workloadCount;
    m_activeWorkloads = snapshot.activeWorkloads;
    m_seenProcesses = snapshot.seenProcesses;
    m_unmanagedProcesses = snapshot.unmanagedProcesses;
    m_workloads = std::move(snapshot.workloads);
    m_topUnmanaged = std::move(snapshot.topUnmanaged);
    m_available = true;
    m_sourceStatus = developerFixture ? QStringLiteral("DEMO")
                    : customSource ? QStringLiteral("OVERRIDE") : QStringLiteral("LIVE");
    emit updated();
}
