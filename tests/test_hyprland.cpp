#include "hyprlandmonitor.h"
#include <QtTest/QtTest>
#include <QLocalServer>
#include <QTemporaryDir>

class HyprlandTest : public QObject {
    Q_OBJECT
private slots:
    void parsesSupportedEvents() {
        QCOMPARE(parseHyprlandWorkspaceEvent("workspace>>3"),3);
        QCOMPARE(parseHyprlandWorkspaceEvent("workspacev2>>10,dev"),10);
        QCOMPARE(parseHyprlandWorkspaceEvent("focusedmon>>DP-1,4"),4);
        QCOMPARE(parseHyprlandWorkspaceEvent("workspace>>special:scratch"),-1);
        QCOMPARE(parseHyprlandWorkspaceEvent("openwindow>>123"),-1);
    }
    void receivesEventsAsynchronously() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path=dir.filePath("socket2.sock");
        QLocalServer server;
        QVERIFY(server.listen(path));
        connect(&server,&QLocalServer::newConnection,&server,[&server] {
            QLocalSocket* peer=server.nextPendingConnection();
            QVERIFY(peer);
            peer->write("workspacev2>>2,code\nworkspace>>7\n");
            peer->flush();
            // Keep peer alive while monitor checks focus.
            peer->setParent(&server);
        });
        HyprlandMonitor monitor(nullptr,path);
        QTRY_VERIFY_WITH_TIMEOUT(monitor.connected(),2000);
        QTRY_COMPARE_WITH_TIMEOUT(monitor.focusedWorkspace(),7,2000);
    }
};
QTEST_GUILESS_MAIN(HyprlandTest)
#include "test_hyprland.moc"
