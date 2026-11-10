#include "decisionsmodel.h"
#include <QtTest/QtTest>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTemporaryDir>
#include <QTimer>
#include <QSignalSpy>

class DecisionsModelTest : public QObject {
    Q_OBJECT
private slots:
    void stripsAnsiAndLimitsLines() {
        const auto lines = parseThmDecisionLines("\033[32mactive\033[0m\n\n background \r\n");
        QCOMPARE(lines, (QStringList{QStringLiteral("active"), QStringLiteral("background")}));
        QByteArray response;
        for (int i=0; i<90; ++i) response += QByteArray(800,'X') + '\n';
        const auto clipped = parseThmDecisionLines(response);
        QCOMPARE(clipped.size(),50);
        QCOMPARE(clipped.first().size(),512);
    }
    void readsHistoryWithoutBlockingUi() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString socketPath = directory.filePath("mock-thm.sock");
        QLocalServer server;
        QVERIFY2(server.listen(socketPath), qPrintable(server.errorString()));

        int handledQueries = 0;
        QObject::connect(&server, &QLocalServer::newConnection, &server, [&] {
            while (QLocalSocket* peer = server.nextPendingConnection()) {
                // Correctly buffer a command even if delivered in several chunks.
                QObject::connect(peer, &QLocalSocket::readyRead, peer, [peer, &handledQueries] {
                    const QByteArray incoming = peer->property("requestBuffer").toByteArray()
                                                + peer->readAll();
                    peer->setProperty("requestBuffer", incoming);
                    if (!incoming.contains('\n')) return;
                    if (incoming.startsWith("decisions 50\n")) {
                        ++handledQueries;
                        peer->write("[one] MOVE active\n[two] IDLE\n");
                        peer->flush();
                        // Allow the asynchronous transport to deliver the bytes
                        // before simulating THM's EOF-based response completion.
                        QTimer::singleShot(20, peer, [peer] {
                            peer->disconnectFromServer();
                        });
                    } else {
                        peer->disconnectFromServer();
                    }
                });
                QObject::connect(peer, &QLocalSocket::disconnected,
                                 peer, &QObject::deleteLater);
            }
        });

        DecisionsModel model(nullptr, socketPath);
        QSignalSpy updated(&model, &DecisionsModel::updated);
        QVERIFY(updated.isValid());
        QTRY_VERIFY_WITH_TIMEOUT(handledQueries >= 1, 4000);
        QTRY_VERIFY2_WITH_TIMEOUT(model.connected(),
                                  qPrintable(model.errorMessage()), 4000);
        QCOMPARE(model.lines(), (QStringList{QStringLiteral("[one] MOVE active"),
                                               QStringLiteral("[two] IDLE")}));
    }
    void reportsUnavailableSocket() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        DecisionsModel model(nullptr, directory.filePath("no-server.sock"));
        QTRY_VERIFY_WITH_TIMEOUT(!model.errorMessage().isEmpty(), 1500);
        QVERIFY(!model.connected());
        QVERIFY(model.lines().isEmpty());
    }
};
QTEST_GUILESS_MAIN(DecisionsModelTest)
#include "test_decisions.moc"
