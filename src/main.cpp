#include "ipcserver.h"
#include "procmodel.h"
#include "slicemodel.h"
#include "decisionsmodel.h"
#include "hyprlandmonitor.h"
#include "systemmodel.h"
#include "thmclient.h"
#include "workloadtree.h"
#include <QDir>
#include <QGuiApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QUrl>

int main(int argc,char** argv) {
    QGuiApplication app(argc,argv);
    app.setApplicationName("titan-task-manager");
    app.setOrganizationName("ArchTitan");
    QQuickStyle::setStyle("Basic");

    QString cmd="RAISE";
    const auto args=QCoreApplication::arguments();
    if (args.contains("--hide")) cmd="HIDE";
    else if (args.contains("--toggle")) cmd="TOGGLE";
    else if (args.contains("--show")) cmd="SHOW";
    const QString runtime=qEnvironmentVariable("XDG_RUNTIME_DIR");
    if(runtime.isEmpty() || !QDir(runtime).exists()) {
        qCritical()<<"Run in a desktop session with XDG_RUNTIME_DIR";
        return 1;
    }
    const QString socketPath=QDir(runtime).filePath("titan-task-manager.sock");
    if (IpcServer::sendCommand(socketPath,cmd)) return 0;
    if (cmd=="HIDE") return 0;
    IpcServer ipc;
    if(!ipc.listen(socketPath)) {
        qCritical()<<"Could not start single-instance IPC:"<<ipc.errorString();
        return 1;
    }

    ThmClient thm;
    SystemModel system;
    ProcModel processes;
    SliceModel slices;
    DecisionsModel decisions;
    HyprlandMonitor hyprland;
    WorkloadTree workloadTree(&thm, &processes);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("thm",&thm);
    engine.rootContext()->setContextProperty("sys",&system);
    engine.rootContext()->setContextProperty("proc",&processes);
    engine.rootContext()->setContextProperty("workloadTree",&workloadTree);
    engine.rootContext()->setContextProperty("slices",&slices);
    engine.rootContext()->setContextProperty("decisions",&decisions);
    engine.rootContext()->setContextProperty("hyprland",&hyprland);
    engine.load(QUrl("qrc:/titan-task-manager/Main.qml"));
    if(engine.rootObjects().isEmpty()) {qCritical()<<"Cannot load Main.qml"; return 1;}
    auto* window=qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    if(!window) {qCritical()<<"QML root must be an ApplicationWindow"; return 1;}
    QObject::connect(&ipc,&IpcServer::commandReceived,window,[window](const QString& action) {
        if(action=="HIDE") {window->hide(); return;}
        if(action=="TOGGLE" && window->isVisible()) {window->hide(); return;}
        window->show(); window->raise(); window->requestActivate();
    });
    return app.exec();
}
