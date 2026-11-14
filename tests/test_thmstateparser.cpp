#include "thmstateparser.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest/QtTest>
#include <functional>

namespace {
QByteArray fixture() {
    QFile file(QStringLiteral(THM_FIXTURE_PATH));
    if (!file.open(QIODevice::ReadOnly)) return {};
    return file.readAll();
}
QByteArray modify(const std::function<void(QJsonObject&)>& change) {
    QJsonObject root = QJsonDocument::fromJson(fixture()).object();
    change(root);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}
}

class ThmStateParserTest final : public QObject {
    Q_OBJECT
private slots:
    void documentedDemoSchema() {
        ThmSnapshot state;
        QString error;
        QVERIFY2(parseThmState(fixture(), &state, &error), qPrintable(error));
        QCOMPARE(state.version, QStringLiteral("3.1.0"));
        QCOMPARE(state.pressureLevel, QStringLiteral("NORMAL"));
        QCOMPARE(state.memoryUsedPct, 32.5);
        QCOMPARE(state.workloadCount, 5);
        QCOMPARE(state.activeWorkloads, 2);
        QCOMPARE(state.workloads.size(), 5);
        QCOMPARE(state.topUnmanaged.size(), 1);
        QVERIFY(state.lastDecision.contains(QStringLiteral("PROMOTE")));
    }
    void truncatedJsonRejectedAndSnapshotPreserved() {
        ThmSnapshot state;
        state.version = QStringLiteral("unchanged");
        QString error;
        QVERIFY(!parseThmState(QByteArray("{\"pressure\":"), &state, &error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(state.version, QStringLiteral("unchanged"));
    }
    void emptyAndOversizedRejected() {
        ThmSnapshot state;
        QVERIFY(!parseThmState({}, &state));
        QVERIFY(!parseThmState(QByteArray(4 * 1024 * 1024 + 1, 'x'), &state));
    }
    void missingRequiredFieldsRejected() {
        const QByteArray json = modify([](QJsonObject& root) { root.remove("workloads"); });
        ThmSnapshot state;
        QVERIFY(!parseThmState(json, &state));
    }
    void invalidPressureRejected() {
        const QByteArray json = modify([](QJsonObject& root) {
            auto pressure = root.value("pressure").toObject();
            pressure.insert("level", "UNKNOWN");
            root.insert("pressure", pressure);
        });
        ThmSnapshot state;
        QVERIFY(!parseThmState(json, &state));
    }
    void impossibleMemoryAndCountersRejected() {
        ThmSnapshot state;
        const QByteArray badRam = modify([](QJsonObject& root) {
            auto pressure = root.value("pressure").toObject();
            pressure.insert("mem_used_pct", 145.0); root.insert("pressure", pressure);
        });
        QVERIFY(!parseThmState(badRam, &state));
        const QByteArray badCount = modify([](QJsonObject& root) {
            root.insert("active_workloads", 200);
        });
        QVERIFY(!parseThmState(badCount, &state));
    }
    void invalidWorkloadEntryRejected() {
        const QByteArray json = modify([](QJsonObject& root) {
            auto workloads = root.value("workloads").toArray();
            workloads.replace(0, 42);
            root.insert("workloads", workloads);
        });
        ThmSnapshot state;
        QVERIFY(!parseThmState(json, &state));
    }
    void nullLastActionAccepted() {
        const QByteArray json = modify([](QJsonObject& root) {
            root.insert("last_action", QJsonValue::Null);
        });
        ThmSnapshot state;
        QString error;
        QVERIFY2(parseThmState(json, &state, &error), qPrintable(error));
        QVERIFY(state.lastDecision.isEmpty());
    }
    void freshnessBoundaries() {
        const qint64 now = 2000000;
        QVERIFY(thmFileIsFresh(now - 9999, now));
        QVERIFY(thmFileIsFresh(now - 10000, now));
        QVERIFY(!thmFileIsFresh(now - 10001, now));
        QVERIFY(thmFileIsFresh(now + 5000, now));
        QVERIFY(!thmFileIsFresh(now + 5001, now));
        QVERIFY(!thmFileIsFresh(0, now));
    }
};
QTEST_GUILESS_MAIN(ThmStateParserTest)
#include "test_thmstateparser.moc"
