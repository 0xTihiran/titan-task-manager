#include "workloadtreecore.h"
#include <QtTest/QtTest>
#include <QVariantMap>

namespace {
QVariantMap proc(qint64 pid, qint64 ppid, quint64 born) {
    return {{"pid",pid}, {"ppid",ppid}, {"name",QStringLiteral("demo")},
            {"startTime",QVariant::fromValue(born)}, {"cpu",1.0}};
}
QVariantMap workload(int id, qint64 pid) {
    return {{"id",id}, {"pid",pid}, {"name",QStringLiteral("workload")}, {"pids",3}};
}
}

class WorkloadTreeTest : public QObject {
    Q_OBJECT
private slots:
    void reconstructsParentChildTree() {
        const auto state = reconcileWorkloads({workload(7,100)},
            {proc(100,1,10),proc(101,100,11),proc(102,101,12),proc(500,1,20)},true);
        QCOMPARE(state.matched,3);
        QCOMPARE(state.unmatched,1);
        const auto w = state.workloads[0].toMap();
        QVERIFY(!w.value("stale").toBool());
        QCOMPARE(w.value("matchedCount").toInt(),3);
        const auto tree=w.value("tree").toList();
        QCOMPARE(tree[0].toMap().value("pid").toLongLong(),qint64(100));
        QCOMPARE(tree[0].toMap().value("depth").toInt(),0);
        QCOMPARE(tree[1].toMap().value("depth").toInt(),1);
        QCOMPARE(tree[2].toMap().value("depth").toInt(),2);
        QCOMPARE(state.processes[3].toMap().value("assignment").toString(),QStringLiteral("Unmatched"));
        for (const auto& p: state.processes) QVERIFY(!p.toMap().value("canTerminate").toBool());
    }
    void missingRootIsStale() {
        const auto state = reconcileWorkloads({workload(7,100)}, {proc(101,100,40)},true);
        QCOMPARE(state.matched,0);
        QCOMPARE(state.unmatched,1);
        QVERIFY(state.workloads[0].toMap().value("stale").toBool());
        QVERIFY(state.workloads[0].toMap().value("tree").toList().empty());
    }
    void nestedRootsBelongToClosestRoot() {
        const auto state = reconcileWorkloads({workload(1,100), workload(2,200)},
            {proc(100,1,10),proc(110,100,11),proc(200,110,12),proc(201,200,13)},true);
        QCOMPARE(state.matched,4);
        QCOMPARE(state.workloads[0].toMap().value("matchedCount").toInt(),2);
        QCOMPARE(state.workloads[1].toMap().value("matchedCount").toInt(),2);
        QCOMPARE(state.processes[2].toMap().value("workloadId").toInt(),2);
    }
    void rejectsOlderChildLineage() {
        const auto state = reconcileWorkloads({workload(1,100)},
            {proc(100,1,100), proc(101,100,50)},true);
        QCOMPARE(state.matched,1);
        QCOMPARE(state.unmatched,1);
    }
    void telemetryOfflineDoesNotClaimUnmanaged() {
        const auto state = reconcileWorkloads({workload(7,100)}, {proc(100,1,10)},false);
        QVERIFY(state.workloads.isEmpty());
        QCOMPARE(state.processes[0].toMap().value("assignment").toString(),QStringLiteral("THM unavailable"));
    }
};
QTEST_APPLESS_MAIN(WorkloadTreeTest)
#include "test_workloadtree.moc"
