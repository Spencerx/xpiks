#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "restartmodel.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QStringList arguments = app.arguments();
    if (arguments.first().contains(app.applicationName())) {
        arguments.removeFirst();
    }

    RestartModel restartModel(arguments, &app);

    QQmlApplicationEngine engine;
    QQmlContext *rootContext = engine.rootContext();
    rootContext->setContextProperty("restartModel", &restartModel);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
