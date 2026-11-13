#include "protectionfilter.h"
#include "procidentity.h"
#include <QtTest/QtTest>

namespace {
QVariantMap ownUnmatchedProcess() {
    return {{"pid", 4567LL}, {"ppid", 2345LL}, {"name", "test-editor"},
            {"exe", "/usr/bin/test-editor"}, {"cgroup", "0::/user.slice/user-1000.slice"},
            {"startTime", QVariant::fromValue(quint64(12345))},
            {"uid", 1000LL}, {"effectiveUid", 1000LL}, {"uidVerified", true},
            {"workloadId", 0}, {"assignment", "Unmatched"}};
}
ProtectionAssessment check(const QVariantMap& process, bool thm = true) {
    return assessProcessProtection(process, thm, 9000, 8999, 1000);
}
}

class ProtectionFilterTest : public QObject {
    Q_OBJECT
private slots:
    void initIsAlwaysReadOnly() {
        const auto a = check(ownUnmatchedProcess());
        QCOMPARE(a.status, QStringLiteral("UNVERIFIED"));
        QVERIFY(!a.protectedProcess);
        QVERIFY(!a.canTerminate);
        QVERIFY(!a.reason.isEmpty());
    }
    void rejectsInitAndInvalidPid() {
        for (const qint64 pid : {qint64(-1),qint64(0),qint64(1)}) {
            auto p = ownUnmatchedProcess(); p["pid"] = pid;
            const auto a = check(p);
            QVERIFY(a.protectedProcess);
            QVERIFY(!a.canTerminate);
        }
    }
    void rejectsSelfAndParent() {
        for (const qint64 pid : {qint64(8999),qint64(9000)}) {
            auto p = ownUnmatchedProcess(); p["pid"] = pid;
            QVERIFY(check(p).protectedProcess);
        }
    }
    void rejectsOtherUsersAndElevatedEffectiveUid() {
        auto p = ownUnmatchedProcess(); p["uid"] = 1001LL;
        QVERIFY(check(p).protectedProcess);
        p = ownUnmatchedProcess(); p["effectiveUid"] = 0LL;
        QVERIFY(check(p).protectedProcess);
    }
    void unknownUidFailsClosed() {
        auto p = ownUnmatchedProcess(); p["uidVerified"] = false;
        auto a = check(p);
        QCOMPARE(a.status, QStringLiteral("UNVERIFIED"));
        QVERIFY(!a.canTerminate);
    }
    void protectsArchTitanAndCompositorAndBrowser() {
        for (const QString& name : {"titan-hwm", "titan-task-mana", "Hyprland", "firefox"}) {
            auto p = ownUnmatchedProcess(); p["name"] = name;
            QVERIFY2(check(p).protectedProcess, qPrintable(name));
        }
    }
    void protectsExecutableEvenWithDifferentCommName() {
        auto p = ownUnmatchedProcess(); p["name"] = "generic";
        p["exe"] = "/usr/bin/titan-hwm";
        QVERIFY(check(p).protectedProcess);
        p["exe"] = "/usr/lib/systemd/systemd";
        QVERIFY(check(p).protectedProcess);
    }
    void protectsManagedOrObservedOrCgroupSlice() {
        auto p = ownUnmatchedProcess(); p["workloadId"] = 42;
        QVERIFY(check(p).protectedProcess);
        p = ownUnmatchedProcess(); p["assignment"] = "THM managed";
        QVERIFY(check(p).protectedProcess);
        p = ownUnmatchedProcess(); p["workloadObserved"] = true;
        QVERIFY(check(p).protectedProcess);
        p = ownUnmatchedProcess(); p["workloadType"] = "VM";
        QVERIFY(check(p).protectedProcess);
        p = ownUnmatchedProcess(); p["cgroup"] = "0::/archtitan.slice/archtitan-active.slice";
        QVERIFY(check(p).protectedProcess);
    }
    void missingIdentityNeverEnablesAction() {
        auto p = ownUnmatchedProcess(); p.remove("startTime");
        QVERIFY(!check(p).canTerminate);
        p = ownUnmatchedProcess(); p["exe"] = "";
        QVERIFY(!check(p).canTerminate);
        p = ownUnmatchedProcess(); p["cgroup"] = "";
        QVERIFY(!check(p).canTerminate);
    }
    void telemetryOfflineRemainsReadOnly() {
        auto a = check(ownUnmatchedProcess(),false);
        QCOMPARE(a.status, QStringLiteral("UNVERIFIED"));
        QVERIFY(a.reason.contains("THM"));
        QVERIFY(!a.canTerminate);
    }
    void parsesUidsFromProcStatus() {
        qint64 r = -1, e = -1;
        QVERIFY(parseProcUids("Name:\tx\nUid:\t1000\t1000\t1000\t1000\n",&r,&e));
        QCOMPARE(r,qint64(1000)); QCOMPARE(e,qint64(1000));
        QVERIFY(!parseProcUids("Name:\tx\n",&r,&e));
        QVERIFY(!parseProcUids("Uid:\tgarbled\t1000\t1000\t1000\n",&r,&e));
        QVERIFY(!parseProcUids("Uid:\t4294967296\t0\t0\t0\n",&r,&e));
    }
    void rejectsRecycledPidIdentity() {
        const auto p = ownUnmatchedProcess();
        ProcStat current; current.pid=4567; current.starttime=12345;
        QVERIFY(sameProcessIdentity(p,current));
        current.starttime=12346;
        QVERIFY(!sameProcessIdentity(p,current));
        current.starttime=12345; current.pid=4568;
        QVERIFY(!sameProcessIdentity(p,current));
    }
};
QTEST_GUILESS_MAIN(ProtectionFilterTest)
#include "test_protectionfilter.moc"
