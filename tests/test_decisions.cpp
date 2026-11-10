#include "decisionsmodel.h"
#include <QtTest/QtTest>
#include <QLocalServer>
#include <QTemporaryDir>

class DecisionsModelTest : public QObject {
    Q_OBJECT
private slots:
    void stripsAnsiAndLimitsLines() {
        const auto lines = parseThmDecisionLines("\033[32mactive\033[0m\n\n background \r\n");
        QCOMPARE(lines, (QStringList{QStringLiteral("active"), QStringLiteral("background")}));
        QByteArray longResponse;
        for (int i=0; i<90; ++i) longResponse += QByteArray(800,'X') + '\n';
        const auto clipped = parseThmDecisionLines(longResponse);
        QCOMPARE(clipped.size(),50);
        QCOMPARE(clipped.first().size(),512);
    }
    void readsHistoryWithoutBlockingUi() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path=dir.filePath("thm.sock");
        QLocalServer server;
        QVERIFY(server.listen(path));
        connect(&server, &QLocalServer::newConnection, &server, [&server] {
            auto* peer=server.nextPendingConnection();
            QVERIFY(peer);
            QObject::connect(peer,&QLocalSocket::readyRead,peer,[peer] {
                const QByteArray command=peer->readAll();
                if (!command.contains("decisions 50\n")) return;
                peer->write("[one] MOVE active\n[two] IDLE\n");
                peer->flush();
                peer->disconnectFromServer();
            });
            QObject::connect(peer,&QLocalSocket::disconnected,peer,&QObject::deleteLater);
        });
        DecisionsModel model(nullptr,path);
        QTRY_VERIFY_WITH_TIMEOUT(model.connected(),2000);
        QCOMPARE(model.lines().size(),2);
        QCOMPARE(model.lines().first(),QStringLiteral("[one] MOVE active"));
    }
};
QTEST_APPLESS_MAIN(DecisionsModelTest)
#include "test_decisions.moc"
