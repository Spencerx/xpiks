#include <iostream>
#include <exception>
#include <cstdlib>

#include <QDebug>
#include <QTimer>
#include <QCoreApplication>

#include <chillout.h>

#include "Connectivity/curlinithelper.h"

#include "../../xpiks-qt/Helpers/logger.h"
#include "integrationtestsenvironment.h"
#include "xpikstestsapp.h"
#include "exiv2iohelpers.h"

#include "testshelpers.h"

#include "integrationtestbase.h"
#include "addfilesbasictest.h"
#include "autoattachvectorstest.h"
#include "savefilebasictest.h"
#include "spellcheckmultireplacetest.h"
#include "spellcheckcombinedmodeltest.h"
#include "zipartworkstest.h"
#include "spellcheckundotest.h"
#include "autocompletebasictest.h"
#include "spellingproduceswarningstest.h"
#include "undoaddwithvectorstest.h"
#include "readlegacysavedtest.h"
#include "clearmetadatatest.h"
#include "savewithemptytitletest.h"
#include "combinededitfixspellingtest.h"
#include "findandreplacemodeltest.h"
#include "addtouserdictionarytest.h"
#include "autodetachvectortest.h"
#include "removefromuserdictionarytest.h"
#include "artworkuploaderbasictest.h"
#include "plaintextedittest.h"
#include "fixspellingmarksmodifiedtest.h"
#include "presetstest.h"
#include "translatorbasictest.h"
#include "userdictedittest.h"
#include "weirdnamesreadtest.h"
#include "restoresessiontest.h"
#include "savefilelegacytest.h"
#include "locallibrarysearchtest.h"
#include "metadatacachesavetest.h"
#include "savevideobasictest.h"
#include "duplicatesearchtest.h"
#include "autocompletepresetstest.h"
#include "csvexporttest.h"
#include "unicodeiotest.h"
#include "faileduploadstest.h"
#include "undoadddirectorytest.h"
#include "undorestoresessiontest.h"
#include "masterpasswordtest.h"
#include "reimporttest.h"
#include "autoimporttest.h"
#include "importlostmetadatatest.h"
#include "warningscombinedtest.h"
#include "csvdefaultexporttest.h"
#include "loadpluginbasictest.h"
#include "stockftpautocompletetest.h"

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    QString logLine = qFormatLogMessage(type, context, msg);
#else
    QString msgType;
    switch (type) {
        case QtDebugMsg:
            msgType = "debug";
            break;
        case QtWarningMsg:
            msgType = "warning";
            break;
        case QtCriticalMsg:
            msgType = "critical";
            break;
        case QtFatalMsg:
            msgType = "fatal";
            break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 1))
        case QtInfoMsg:
            msgType = "info";
            break;
#endif
    }

    // %{time hh:mm:ss.zzz} %{type} T#%{threadid} %{function} - %{message}
    QString time = QDateTime::currentDateTimeUtc().toString("hh:mm:ss.zzz");
    QString logLine = QString("%1 %2 T#%3 %4 - %5")
                          .arg(time).arg(msgType)
                          .arg(0).arg(context.function)
                          .arg(msg);
#endif

    Helpers::Logger &logger = Helpers::Logger::getInstance();
    logger.log(type, logLine);

    if ((type == QtFatalMsg) || (type == QtWarningMsg)) {
        logger.abortFlush();
    }

    if (type == QtFatalMsg) {
        abort();
    }
}

void initCrashRecovery(Common::ISystemEnvironment &environment) {
    auto &chillout = Debug::Chillout::getInstance();
    QString crashesDirPath = QDir::toNativeSeparators(environment.path({Constants::CRASHES_DIR}));
#ifdef Q_OS_WIN
    chillout.init(L"xpiks-tests-integration", crashesDirPath.toStdWString());
#else
    chillout.init("xpiks-tests-integration", crashesDirPath.toStdString());
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

void initQSettings() {
    QCoreApplication::setOrganizationName(Constants::ORGANIZATION_NAME);
    QCoreApplication::setApplicationName(Constants::APPLICATION_NAME);
}

int main(int argc, char *argv[]) {
    initQSettings();

    std::cout << "Started integration tests" << std::endl;
    std::cout << "Current working directory: " << QDir::currentPath().toStdString() << std::endl;

    // will call curl_global_init and cleanup
    Connectivity::CurlInitHelper curlInitHelper;
    Q_UNUSED(curlInitHelper);

    Exiv2InitHelper exiv2InitHelper;
    Q_UNUSED(exiv2InitHelper);

    qSetMessagePattern("%{time hh:mm:ss.zzz} %{type} T#%{threadid} %{function} - %{message}");
    qInstallMessageHandler(myMessageHandler);
    qRegisterMetaType<Common::SpellCheckFlags>("Common::SpellCheckFlags");

    // -----------------------------------------------
    QCoreApplication app(argc, argv);
    IntegrationTestsEnvironment environment(app.arguments());
    environment.ensureSystemDirectoriesExist();
    initCrashRecovery(environment);
    // -----------------------------------------------
    std::cout << "Initialized application" << std::endl;

#ifdef WITH_LOGS
    {
        QString time = QDateTime::currentDateTimeUtc().toString("ddMMyyyy-hhmmss-zzz");
        QString logFilename = QString("xpiks-qt-%1.log").arg(time);
        QString logFilePath = environment.path({Constants::LOGS_DIR, logFilename});
        Helpers::Logger &logger = Helpers::Logger::getInstance();
        logger.setLogFilePath(logFilePath);
        logger.setMemoryOnly(environment.getIsInMemoryOnly());
    }
#endif

    XpiksTestsApp xpiksTests(environment);
    xpiksTests.startLogging();
    xpiksTests.initialize();
    xpiksTests.start();

    xpiksTests.waitInitialized();

    auto &commandManager = xpiksTests.getCommandManager();
    int result = 0;

    QVector<IntegrationTestBase*> integrationTests;

    // always the first one
    integrationTests.append(new MetadataCacheSaveTest(environment, &commandManager));
    // and all others
    integrationTests.append(new AddFilesBasicTest(environment, &commandManager));
    integrationTests.append(new AutoAttachVectorsTest(environment, &commandManager));
    integrationTests.append(new SaveFileBasicTest(environment, &commandManager));
    integrationTests.append(new SaveFileLegacyTest(environment, &commandManager));
#ifndef TRAVIS_CI
    integrationTests.append(new SaveVideoBasicTest(environment, &commandManager));
#endif
    integrationTests.append(new FailedUploadsTest(environment, &commandManager));
    integrationTests.append(new SpellCheckMultireplaceTest(environment, &commandManager));
    integrationTests.append(new SpellCheckCombinedModelTest(environment, &commandManager));
    integrationTests.append(new ZipArtworksTest(environment, &commandManager));
    integrationTests.append(new SpellCheckUndoTest(environment, &commandManager));
    integrationTests.append(new AutoCompleteBasicTest(environment, &commandManager));
    integrationTests.append(new SpellingProducesWarningsTest(environment, &commandManager));
    integrationTests.append(new UndoAddWithVectorsTest(environment, &commandManager));
    integrationTests.append(new ReadLegacySavedTest(environment, &commandManager));
    integrationTests.append(new ClearMetadataTest(environment, &commandManager));
    integrationTests.append(new SaveWithEmptyTitleTest(environment, &commandManager));
    integrationTests.append(new CombinedEditFixSpellingTest(environment, &commandManager));
    integrationTests.append(new FindAndReplaceModelTest(environment, &commandManager));
    integrationTests.append(new AddToUserDictionaryTest(environment, &commandManager));
    integrationTests.append(new AutoDetachVectorTest(environment, &commandManager));
    integrationTests.append(new RemoveFromUserDictionaryTest(environment, &commandManager));
    integrationTests.append(new ArtworkUploaderBasicTest(environment, &commandManager));
    integrationTests.append(new PlainTextEditTest(environment, &commandManager));
    integrationTests.append(new FixSpellingMarksModifiedTest(environment, &commandManager));
    integrationTests.append(new PresetsTest(environment, &commandManager));
    integrationTests.append(new TranslatorBasicTest(environment, &commandManager));
    integrationTests.append(new UserDictEditTest(environment, &commandManager));
    integrationTests.append(new WeirdNamesReadTest(environment, &commandManager));
    integrationTests.append(new RestoreSessionTest(environment, &commandManager));
    integrationTests.append(new DuplicateSearchTest(environment, &commandManager));
    integrationTests.append(new AutoCompletePresetsTest(environment, &commandManager));
    integrationTests.append(new CsvExportTest(environment, &commandManager));
    integrationTests.append(new UnicodeIoTest(environment, &commandManager));
    integrationTests.append(new UndoAddDirectoryTest(environment, &commandManager));
    integrationTests.append(new UndoRestoreSessionTest(environment, &commandManager));
    integrationTests.append(new MasterPasswordTest(environment, &commandManager));
    integrationTests.append(new ReimportTest(environment, &commandManager));
    integrationTests.append(new AutoImportTest(environment, &commandManager));
    integrationTests.append(new ImportLostMetadataTest(environment, &commandManager));
    integrationTests.append(new WarningsCombinedTest(environment, &commandManager));
    integrationTests.append(new CsvDefaultExportTest(environment, &commandManager));
    integrationTests.append(new LoadPluginBasicTest(environment, &commandManager));
    integrationTests.append(new StockFtpAutoCompleteTest(environment, &commandManager));
    // always the last one. insert new tests above
    integrationTests.append(new LocalLibrarySearchTest(environment, &commandManager));


    qInfo("\n");
    int succeededTestsCount = 0, failedTestsCount = 0;
    QStringList failedTests;

    foreach (IntegrationTestBase *test, integrationTests) {
        QThread::msleep(500);

        qInfo("---------------------------------------------------------");
        qInfo("Running test: %s", test->testName().toStdString().c_str());

        test->setup();
        const int testResult = test->doTest();
        test->teardown();

        result += testResult;
        if (testResult == 0) {
            succeededTestsCount++;
            qInfo("Test %s PASSED", test->testName().toStdString().c_str());
        } else {
            failedTestsCount++;
            qInfo("Test %s FAILED", test->testName().toStdString().c_str());
            failedTests.append(test->testName());
        }

        qInfo("\n");
    }

    qDeleteAll(integrationTests);

    qInfo() << "--------------------------";
    qInfo() << "Integration Tests Results:" << succeededTestsCount << "succeeded," << failedTestsCount << "failed";
    qInfo() << "Tests return code" << result;

    if (!failedTests.empty()) {
        qInfo() << "FAILED TESTS:" << failedTests.join(", ");
    }

    qInfo() << "--------------------------";

    QTimer debuggingTimer;
    debuggingTimer.setSingleShot(true);
    QObject::connect(&debuggingTimer, &QTimer::timeout,
                     []() {
        Q_ASSERT(false);
    });
    // assert in 1 minute to see hung threads
    debuggingTimer.start(60*1000);
    qInfo() << "--";
    qInfo() << "-----> Crash timer started...";
    qInfo() << "--";

    xpiksTests.stop();

    // for the logs to appear
    app.processEvents();

    QThread::sleep(1);

    std::cout << "Integration tests finished" << std::endl;

    return result;
}

