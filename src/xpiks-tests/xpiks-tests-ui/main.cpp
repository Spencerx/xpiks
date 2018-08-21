#include <QtQuickTest/quicktest.h>
#include <QtQml>
#include <QDir>
#include <QDebug>
#include <QQmlEngine>
#include <Helpers/clipboardhelper.h>
#include <QMLExtensions/triangleelement.h>
#include <Helpers/logger.h>
#include "testshost.h"
#include "xpiksuitestsapp.h"
#include "uitestsenvironment.h"

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define QML2_IMPORT_PATH_VAR "QML2_IMPORT_PATH"

static QObject *createTestsHostsQmlObject(QQmlEngine *engine, QJSEngine *scriptEngine) {
    Q_UNUSED(scriptEngine);

    TestsHost &host = TestsHost::getInstance();
    host.qmlEngineCallback(engine);

    QObject *object = &host;
    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    return object;
}

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    QString logLine = qFormatLogMessage(type, context, msg);

    Helpers::Logger &logger = Helpers::Logger::getInstance();
    logger.log(type, logLine);

    if ((type == QtFatalMsg) || (type == QtWarningMsg)) {
        logger.abortFlush();
    }

    if (type == QtFatalMsg) {
        abort();
    }
}

int main(int argc, char **argv) {
    // hack to overcome URI warning when loading Stubs plugin
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    QString importPath = QString::fromLatin1(qgetenv(QML2_IMPORT_PATH_VAR));
    if (!importPath.isEmpty()) { importPath += ";"; }
    importPath += QDir::toNativeSeparators(STRINGIZE(PLUGIN_STUB_IMPORT_DIR));
    qDebug() << "Setting QML2_IMPORT_PATH path to" << importPath;
    qputenv(QML2_IMPORT_PATH_VAR, importPath.toUtf8());
#endif

    qSetMessagePattern("%{time hh:mm:ss.zzz} %{type} T#%{threadid} %{function} - %{message}");
    qInstallMessageHandler(myMessageHandler);

    UITestsEnvironment uiTestsEnvironment;

#ifdef WITH_LOGS
    Helpers::Logger &logger = Helpers::Logger::getInstance();
    logger.setMemoryOnly(uiTestsEnvironment.getIsInMemoryOnly());
#endif

    XpiksUITestsApp xpiksTests(uiTestsEnvironment);

    xpiksTests.startLogging();
    xpiksTests.initialize();
    xpiksTests.start();
    xpiksTests.waitInitialized();

    TestsHost &host = TestsHost::getInstance();
    host.setApp(&xpiksTests);

    qmlRegisterType<Helpers::ClipboardHelper>("xpiks", 1, 0, "ClipboardHelper");
    qmlRegisterType<QMLExtensions::TriangleElement>("xpiks", 1, 0, "TriangleElement");

    qmlRegisterSingletonType<TestsHost>("XpiksTests", 1, 0, "TestsHost", createTestsHostsQmlObject);

    return quick_test_main(argc, argv, "xpiks_tests_ui", QUICK_TEST_SOURCE_DIR);
}
