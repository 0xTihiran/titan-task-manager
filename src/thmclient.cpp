#include "thmclient.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QtGlobal>

ThmClient::ThmClient(QObject* parent): QObject(parent) {
    m_sourcePath = qEnvironmentVariable("THM_TELEMETRY_PATH", "/run/archtitan/thm-state.json");
    connect(&m_timer,&QTimer::timeout,this,&ThmClient::refresh);
    m_timer.start(1000);
    refresh();
}
void ThmClient::refresh() {
    m_available=false;
    m_error.clear(); m_version.clear(); m_pressureLevel.clear(); m_governor.clear();
    m_lastDecision.clear(); m_memoryUsedPct=0; m_workloadCount=0;
    m_activeWorkloads=0; m_seenProcesses=0; m_unmanagedProcesses=0;
    m_workloads.clear(); m_topUnmanaged.clear();
    QFile file(m_sourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_error=QStringLiteral("THM telemetry unavailable: %1").arg(file.errorString());
        emit updated(); return;
    }
    QJsonParseError pe;
    const auto doc = QJsonDocument::fromJson(file.readAll(),&pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        m_error=QStringLiteral("Malformed THM JSON: %1").arg(pe.errorString());
        emit updated(); return;
    }
    const auto obj=doc.object();
    if (!obj.value("pressure").isObject() || !obj.value("workloads").isArray()) {
        m_error=QStringLiteral("THM JSON missing pressure/workloads");
        emit updated(); return;
    }
    const auto pressure=obj.value("pressure").toObject();
    const auto proc=obj.value("processes").toObject();
    m_version=obj.value("version").toString();
    m_governor=obj.value("governor").toString();
    m_pressureLevel=pressure.value("level").toString("UNKNOWN");
    m_memoryUsedPct=pressure.value("mem_used_pct").toDouble();
    m_workloadCount=obj.value("workload_count").toInt();
    m_activeWorkloads=obj.value("active_workloads").toInt();
    m_seenProcesses=proc.value("seen").toInt();
    m_unmanagedProcesses=proc.value("unmanaged").toInt();
    for (const auto& v: obj.value("workloads").toArray()) {
        if (v.isObject()) m_workloads.append(v.toObject().toVariantMap());
    }
    for (const auto& v: obj.value("top_unmanaged").toArray()) {
        if (v.isObject()) m_topUnmanaged.append(v.toObject().toVariantMap());
    }
    const auto action=obj.value("last_action");
    if (action.isObject()) {
        const auto a=action.toObject();
        m_lastDecision=QStringLiteral("%1  •  %2  •  PID %3")
             .arg(a.value("decision").toString("THM decision"),
                  a.value("target").toString("unspecified"))
             .arg(a.value("pid").toInt());
    }
    m_available=true;
    emit updated();
}
