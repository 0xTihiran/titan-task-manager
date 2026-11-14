#include "diagnosticscore.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariantMap>
#include <QJsonObject>
#include <QtTest/QtTest>

namespace {
QString statusFor(const QVariantList& checks, const QString& id) {
    for (const QVariant& check : checks) {
        const QVariantMap c = check.toMap();
        if (c.value(QStringLiteral("id")).toString() == id)
            return c.value(QStringLiteral("status")).toString();
    }
    return {};
}
}

class DiagnosticsCoreTest final : public QObject {
    Q_OBJECT
private slots:
    void completeHealthyProbeStillBlocksActions() {
        DiagnosticsInput input;
        input.thmSourceStatus = QStringLiteral("LIVE");
        input.systemAvailable = true;
        input.procfsReadable = true;
        input.cpuPsiReadable = input.memoryPsiReadable = input.ioPsiReadable = true;
        input.cgroupV2Readable = true;
        input.thmDecisionsConnected = true;
        input.hyprlandConnected = true;
        const auto checks = buildDiagnosticChecks(input);
        QCOMPARE(checks.size(), 10);
        QCOMPARE(statusFor(checks, "thm"), QStringLiteral("PASS"));
        QCOMPARE(statusFor(checks, "proc"), QStringLiteral("PASS"));
        QCOMPARE(statusFor(checks, "actions"), QStringLiteral("BLOCKED"));
        QCOMPARE(countDiagnosticWarnings(checks), 0);
        QCOMPARE(countDiagnosticBlockers(checks), 3);
    }
    void noThmOrKernelInformationNeverClaimsHealthy() {
        DiagnosticsInput input;
        input.thmSourceStatus = QStringLiteral("STALE");
        const auto checks = buildDiagnosticChecks(input);
        QCOMPARE(statusFor(checks, "thm"), QStringLiteral("WARN"));
        QCOMPARE(statusFor(checks, "system"), QStringLiteral("WARN"));
        QCOMPARE(statusFor(checks, "proc"), QStringLiteral("WARN"));
        QCOMPARE(statusFor(checks, "decisions"), QStringLiteral("WARN"));
        QVERIFY(countDiagnosticWarnings(checks) >= 4);
        QCOMPARE(statusFor(checks, "actions"), QStringLiteral("BLOCKED"));
    }
    void nonstandardPathIsNotLiveOrProductionTrust() {
        for (const QString& status : {QStringLiteral("DEMO"), QStringLiteral("OVERRIDE")}) {
            DiagnosticsInput input;
            input.thmSourceStatus = status;
            const auto checks = buildDiagnosticChecks(input);
            QCOMPARE(statusFor(checks, "thm"), QStringLiteral("INFO"));
            QCOMPARE(statusFor(checks, "actions"), QStringLiteral("BLOCKED"));
        }
    }
    void reportIncludesOnlyAllowlistedFields() {
        DiagnosticsInput input;
        input.thmSourceStatus = QStringLiteral("MISSING");
        auto checks = buildDiagnosticChecks(input);
        QVariantMap first = checks.first().toMap();
        first.insert(QStringLiteral("sensitiveCommandLine"), QStringLiteral("super-secret"));
        checks[0] = first;
        const QString report = makeDiagnosticReport(checks, "2026-11-14T00:00:00Z", input.thmSourceStatus);
        QVERIFY(!report.contains("super-secret"));
        const auto json = QJsonDocument::fromJson(report.toUtf8());
        QVERIFY(json.isObject());
        const auto obj = json.object();
        QCOMPARE(obj.value(QStringLiteral("read_only")).toBool(), true);
        QCOMPARE(obj.value(QStringLiteral("version")).toString(), QStringLiteral("0.8.0"));
        QCOMPARE(obj.value(QStringLiteral("checks")).toArray().size(), 10);
    }
};
QTEST_GUILESS_MAIN(DiagnosticsCoreTest)
#include "test_diagnosticscore.moc"
