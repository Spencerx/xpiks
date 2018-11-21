#include <cstdlib>

#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QtGlobal>
#include <QtQml>
#include <QtQuickTest/quicktest.h>

#include <vendors/chillout/src/chillout/chillout.h>

#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Helpers/constants.h"
#include "Helpers/logger.h"

#include "testshost.h"
#include "uitestsenvironment.h"
#include "xpiksuitestsapp.h"

class QJSEngine;
class QObject;

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define QML2_IMPORT_PATH_VAR "QML2_IMPORT_PATH"

static QObject *createTestsHostsQmlObject(QQmlEngine *engine, QJSEngine *scriptEngine) {
    Q_UNUSED(scriptEngine);

    TestsHost &host = TestsHost::getInstance();
    host.qmlEngineCallback(engine);
    LOG_INFO << "QML import paths:" << engine->importPathList();

    QObject *object = &host;
    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    return object;
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
/*#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    QString importPath = QString::fromLatin1(qgetenv(QML2_IMPORT_PATH_VAR));
    if (!importPath.isEmpty()) { importPath += ";"; }
    importPath += QDir::toNativeSeparators(STRINGIZE(PLUGIN_STUB_IMPORT_DIR));
    qDebug() << "Setting QML2_IMPORT_PATH path to" << importPath;
    qputenv(QML2_IMPORT_PATH_VAR, importPath.toUtf8());
#endif*/

    QGuiApplication app(argc, argv);
    Q_UNUSED(app);

    UITestsEnvironment uiTestsEnvironment;

#if defined(APPVEYOR)
    initCrashRecovery(uiTestsEnvironment);
#endif

#ifdef WITH_LOGS
    Helpers::Logger &logger = Helpers::Logger::getInstance();
    logger.setMemoryOnly(uiTestsEnvironment.getIsInMemoryOnly());
#endif
    int result = 0;
    {
        QEventLoop eventLoop;
        XpiksUITestsApp xpiksTests(uiTestsEnvironment,
                                   QStringList() << QDir::toNativeSeparators(STRINGIZE(DIALOGS_DIR)));

        xpiksTests.startLogging();
        xpiksTests.initialize();
        xpiksTests.start();
        xpiksTests.waitInitialized();
        eventLoop.processEvents(QEventLoop::AllEvents);

        if (!xpiksTests.setupCommonFiles()) {
            return 1;
        }

        TestsHost &host = TestsHost::getInstance();
        host.setApp(&xpiksTests);

        qmlRegisterSingletonType<TestsHost>("XpiksTests", 1, 0, "TestsHost", createTestsHostsQmlObject);

        LOG_DEBUG << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
        LOG_DEBUG << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
        LOG_DEBUG << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";

        result = quick_test_main(argc, argv, "xpiks_tests_ui", QUICK_TEST_SOURCE_DIR);

        LOG_DEBUG << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
        LOG_DEBUG << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
        LOG_DEBUG << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";

        xpiksTests.stop();
        QThread::sleep(1);
        eventLoop.processEvents(QEventLoop::AllEvents);
        xpiksTests.quit();
    }
    return result;
}
