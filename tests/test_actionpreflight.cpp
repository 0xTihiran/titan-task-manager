#include "actionpreflight.h"
#include <QtTest/QtTest>

namespace {
QVariantMap sample() {
    return {{"pid", 4567LL}, {"ppid", 1234LL}, {"name", "test-editor"},
            {"exe", "/usr/bin/test-editor"}, {"cgroup", "0::/user.slice/user-1000.slice"},
            {"startTime", QVariant::fromValue(quint64(12345))},
            {"uid", 1000LL}, {"effectiveUid", 1000LL}, {"uidVerified", true},
            {"workloadId", 0}, {"assignment", "Unmatched"}};
}
ActionPreflight review(const QVariantMap& cached, const QVariantMap& current, bool thm = true) {
    return evaluateActionPreflight(cached, current, thm, 9000, 8999, 1000);
}
}

class ActionPreflightTest final : public QObject {
    Q_OBJECT
private slots:
    void noSnapshotIsBlocked() {
        auto v = review({}, sample());
        QCOMPARE(v.code, QStringLiteral("NO_SNAPSHOT"));
        QVERIFY(!v.maySignal);
    }
    void missingLiveProcessIsBlocked() {
        auto v = review(sample(), {});
        QCOMPARE(v.code, QStringLiteral("PROCESS_EXITED"));
        QVERIFY(!v.maySignal);
    }
    void reusedPidIsBlocked() {
        auto changed = sample(); changed["startTime"] = QVariant::fromValue(quint64(12346));
        auto v = review(sample(), changed);
        QCOMPARE(v.code, QStringLiteral("STALE_PID"));
        QVERIFY(!v.maySignal);
    }
    void changedExeAndCgroupAreBlocked() {
        auto changed = sample(); changed["exe"] = "/tmp/other";
        QCOMPARE(review(sample(), changed).code, QStringLiteral("IDENTITY_CHANGED"));
        changed = sample(); changed["cgroup"] = "0::/other.slice";
        QCOMPARE(review(sample(), changed).code, QStringLiteral("IDENTITY_CHANGED"));
    }
    void uidMismatchAndMissingUidAreBlocked() {
        auto changed = sample(); changed["effectiveUid"] = 0LL;
        QCOMPARE(review(sample(), changed).code, QStringLiteral("UID_UNVERIFIED"));
        changed = sample(); changed["uidVerified"] = false;
        QCOMPARE(review(sample(), changed).code, QStringLiteral("UID_UNVERIFIED"));
    }
    void managedAndProtectedAreBlocked() {
        auto owned = sample(); owned["assignment"] = "THM managed";
        QCOMPARE(review(owned, sample()).code, QStringLiteral("PROTECTED"));
        auto protectedProcess = sample(); protectedProcess["name"] = "titan-hwm";
        QCOMPARE(review(protectedProcess, protectedProcess).code, QStringLiteral("PROTECTED"));
    }
    void offlineThmIsBlocked() {
        auto v = review(sample(), sample(), false);
        QCOMPARE(v.code, QStringLiteral("THM_OFFLINE"));
        QVERIFY(!v.maySignal);
    }
    void perfectlyMatchingUnmatchedProcessStillCannotSignal() {
        auto v = review(sample(), sample());
        QCOMPARE(v.code, QStringLiteral("REGISTRY_UNVERIFIED"));
        QVERIFY(!v.maySignal);
    }
};
QTEST_GUILESS_MAIN(ActionPreflightTest)
#include "test_actionpreflight.moc"
