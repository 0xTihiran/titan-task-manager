#include "procparser.h"
#include <QtTest/QtTest>

class ProcParserTest : public QObject {
    Q_OBJECT
private:
    static QByteArray makeStat(const QByteArray& name, const QByteArray& pid = "123") {
        // Fields starting at state(3) through rss(24); 22 values.
        QList<QByteArray> fields;
        fields << "S" << "10";
        for (int i = 5; i <= 13; ++i) fields << "0";
        fields << "200" << "100"; // utime 14, stime 15
        for (int i = 16; i <= 21; ++i) fields << "0";
        fields << "999" << "0" << "55"; // starttime 22, vsize 23, rss 24
        return pid + " (" + name + ") " + fields.join(' ') + "\n";
    }
private slots:
    void acceptsNestedNames() {
        ProcStat info;
        QVERIFY(parseProcStat(makeStat("compiler (worker) name"), &info));
        QCOMPARE(info.pid, qint64(123));
        QCOMPARE(info.ppid, qint64(10));
        QCOMPARE(info.comm, QStringLiteral("compiler (worker) name"));
        QCOMPARE(info.utime, quint64(200));
        QCOMPARE(info.stime, quint64(100));
        QCOMPARE(info.starttime, quint64(999));
        QCOMPARE(info.rssPages, qint64(55));
    }
    void rejectsMissingFields() {
        ProcStat info;
        QVERIFY(!parseProcStat("123 (test) S 2 3\n", &info));
        QVERIFY(!parseProcStat("xx (test) S 2 3\n", &info));
        QVERIFY(!parseProcStat("123 (test S 2 3\n", &info));
        QVERIFY(!parseProcStat(makeStat("sample"), nullptr));
    }
    void rejectsNonNumericFields() {
        ProcStat info;
        QByteArray stat = makeStat("test").replace("999", "oops");
        QVERIFY(!parseProcStat(stat, &info));
    }
};
QTEST_APPLESS_MAIN(ProcParserTest)
#include "test_procparser.moc"
