#include <QtQuickTest/quicktest.h>
#include <QtQml>
#include <QDir>
#include <QDebug>
#include <QQmlEngine>
#include <QEventLoop>
#include <Helpers/logger.h>
#include <chillout.h>
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
    logger.log(logLine);

    if ((type == QtFatalMsg) || (type == QtWarningMsg)) {
        logger.abortFlush();
    }

    if (type == QtFatalMsg) {
        abort();
    }
}

void initCrashRecovery(Common::ISystemEnvironment &environment) {
    auto &chillout = Debug::Chillout::getInstance();

#ifdef APPVEYOR
    QString crashesDirRoot = QDir::currentPath();
#else
    QString crashesDirRoot = environment.path({Constants::CRASHES_DIR});
#endif
    QString crashesDirPath = QDir::toNativeSeparators(crashesDirRoot);

#ifdef Q_OS_WIN
    chillout.init(L"xpiks-tests-ui", crashesDirPath.toStdWString());
#else
    chillout.init("xpiks-tests-ui", crashesDirPath.toStdString());
#endif
    Helpers::Logger &logger = Helpers::Logger::getInstance();

    chillout.setBacktraceCallback([&logger](const char * const stackTrace) {
        logger.emergencyLog(stackTrace);
    });

    chillout.setCrashCallback([&logger, &chillout]() {
        chillout.backtrace();
        logger.emergencyFlush();
#ifdef Q_OS_WIN
        chillout.createCrashDump(Debug::CrashDumpFull);
#endif
    });
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

    QGuiApplication app(argc, argv);
    Q_UNUSED(app);

    UITestsEnvironment uiTestsEnvironment;

#if defined(TRAVIS_CI) || defined(APPVEYOR)
    initCrashRecovery(uiTestsEnvironment);
#endif

    qSetMessagePattern("%{time hh:mm:ss.zzz} %{type} T#%{threadid} %{function} - %{message}");
    qInstallMessageHandler(myMessageHandler);

#ifdef WITH_LOGS
    Helpers::Logger &logger = Helpers::Logger::getInstance();
    logger.setMemoryOnly(uiTestsEnvironment.getIsInMemoryOnly());
#endif
    int result = 0;
    {
        QEventLoop eventLoop;
        XpiksUITestsApp xpiksTests(uiTestsEnvironment);

        xpiksTests.startLogging();
        xpiksTests.initialize();
        xpiksTests.start();
        xpiksTests.waitInitialized();
        eventLoop.processEvents(QEventLoop::AllEvents);

        if (!xpiksTests.setupCommonFiles()) {
            return 1;
        }

        xpiksTests.setupUITests();

        TestsHost &host = TestsHost::getInstance();
        host.setApp(&xpiksTests);

        qmlRegisterSingletonType<TestsHost>("XpiksTests", 1, 0, "TestsHost", createTestsHostsQmlObject);

        result = quick_test_main(argc, argv, "xpiks_tests_ui", QUICK_TEST_SOURCE_DIR);

        xpiksTests.stop();
        QThread::sleep(1);
        eventLoop.processEvents(QEventLoop::AllEvents);
    }

    return result;
}
