#include "thmclient.h"

#include <QDateTime>
#include <QFile>
#include <QFileDevice>
#include <QTemporaryDir>
#include <QtTest/QtTest>

namespace {
struct ScopedEnv {
    const QByteArray pathValue = qgetenv("THM_TELEMETRY_PATH");
    const bool hadPath = qEnvironmentVariableIsSet("THM_TELEMETRY_PATH");
    const QByteArray demoValue = qgetenv("THM_TELEMETRY_ALLOW_STALE_FIXTURE");
    const bool hadDemo = qEnvironmentVariableIsSet("THM_TELEMETRY_ALLOW_STALE_FIXTURE");
    ~ScopedEnv() {
        if (hadPath) qputenv("THM_TELEMETRY_PATH", pathValue);
        else qunsetenv("THM_TELEMETRY_PATH");
        if (hadDemo) qputenv("THM_TELEMETRY_ALLOW_STALE_FIXTURE", demoValue);
        else qunsetenv("THM_TELEMETRY_ALLOW_STALE_FIXTURE");
    }
};
}

class ThmClientTest final : public QObject {
    Q_OBJECT
private slots:
    void transitionsFailClosed() {
        ScopedEnv environment;
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        const QString target = tmp.filePath("current.json");
        QVERIFY(QFile::copy(QStringLiteral(THM_FIXTURE_PATH), target));
        {
            QFile fresh(target);
            QVERIFY(fresh.open(QIODevice::ReadWrite));
            QVERIFY(fresh.setFileTime(QDateTime::currentDateTimeUtc(),
                                      QFileDevice::FileModificationTime));
        }
        QVERIFY(qputenv("THM_TELEMETRY_PATH", target.toLocal8Bit()));
        qunsetenv("THM_TELEMETRY_ALLOW_STALE_FIXTURE");
        ThmClient client;
        QVERIFY2(client.available(), qPrintable(client.errorMessage()));
        QCOMPARE(client.sourceStatus(), QStringLiteral("OVERRIDE"));
        QCOMPARE(client.workloadCount(), 5);

        {
            QFile file(target);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("{"), qint64(1));
        }
        client.refresh();
        QVERIFY(!client.available());
        QCOMPARE(client.sourceStatus(), QStringLiteral("INVALID"));
        QCOMPARE(client.workloadCount(), 0);
        QVERIFY(client.workloads().isEmpty());

        QVERIFY(QFile::remove(target));
        QVERIFY(QFile::copy(QStringLiteral(THM_FIXTURE_PATH), target));
        {
            QFile fresh(target);
            QVERIFY(fresh.open(QIODevice::ReadWrite));
            QVERIFY(fresh.setFileTime(QDateTime::currentDateTimeUtc(),
                                      QFileDevice::FileModificationTime));
        }
        client.refresh();
        QVERIFY(client.available());
        QCOMPARE(client.sourceStatus(), QStringLiteral("OVERRIDE"));
        {
            QFile file(target);
            QVERIFY(file.open(QIODevice::ReadWrite));
            QVERIFY(file.setFileTime(QDateTime::currentDateTimeUtc().addSecs(-65),
                                     QFileDevice::FileModificationTime));
        }
        client.refresh();
        QVERIFY(!client.available());
        QCOMPARE(client.sourceStatus(), QStringLiteral("STALE"));
        QVERIFY(client.workloads().isEmpty());

        // Explicit preview mode is marked DEMO, never LIVE.
        QVERIFY(qputenv("THM_TELEMETRY_ALLOW_STALE_FIXTURE", "1"));
        client.refresh();
        QVERIFY(client.available());
        QCOMPARE(client.sourceStatus(), QStringLiteral("DEMO"));

        QVERIFY(QFile::remove(target));
        client.refresh();
        QVERIFY(!client.available());
        QCOMPARE(client.sourceStatus(), QStringLiteral("MISSING"));
        QCOMPARE(client.workloadCount(), 0);
    }
};
QTEST_GUILESS_MAIN(ThmClientTest)
#include "test_thmclient.moc"
