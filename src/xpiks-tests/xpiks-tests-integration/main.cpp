#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QObject>
#include <QString>  // IWYU pragma: keep
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QtGlobal>

#include <vendors/chillout/src/chillout/chillout.h>

#include "Common/isystemenvironment.h"
#include "Connectivity/curlinithelper.h"
#include "Helpers/constants.h"
#include "Helpers/logger.h"

#include "exiv2iohelpers.h"
#include "integrationtestsenvironment.h"  // IWYU pragma: keep
#include "integrationtestbase.h"
#include "xpikstestsapp.h"

#include "addfilesbasictest.h"
#include "addtouserdictionarytest.h"
#include "artworkuploaderbasictest.h"
#include "autoattachvectorstest.h"
#include "autocompletebasictest.h"
#include "autocompletepresetstest.h"
#include "autodetachvectortest.h"
#include "autoimporttest.h"
#include "clearmetadatatest.h"
#include "combinededitfixspellingtest.h"
#include "csvdefaultexporttest.h"
#include "csvexporttest.h"
#include "duplicatesearchtest.h"
#include "faileduploadstest.h"
#include "findandreplacemodeltest.h"
#include "fixspellingmarksmodifiedtest.h"
#include "importlostmetadatatest.h"
#include "loadpluginbasictest.h"
#include "locallibrarysearchtest.h"
#include "masterpasswordtest.h"
#include "metadatacachesavetest.h"
#include "plaintextedittest.h"
#include "presetstest.h"
#include "readlegacysavedtest.h"
#include "reimporttest.h"
#include "removedirsavessessiontest.h"
#include "removefromuserdictionarytest.h"
#include "restoresessiontest.h"
#include "savefilebasictest.h"
#include "savefilelegacytest.h"
#include "savevideobasictest.h"
#include "savewithemptytitletest.h"
#include "spellcheckcombinedmodeltest.h"
#include "spellcheckmultireplacetest.h"
#include "spellcheckundotest.h"
#include "spellingproduceswarningstest.h"
#include "stockftpautocompletetest.h"
#include "translatorbasictest.h"
#include "unavailablefilestest.h"
#include "undoadddirectorytest.h"
#include "undoaddwithvectorstest.h"
#include "undorestoresessiontest.h"
#include "unicodeiotest.h"
#include "userdictedittest.h"
#include "warningscombinedtest.h"
#include "weirdnamesreadtest.h"
#include "zipartworkstest.h"

void initCrashRecovery(Common::ISystemEnvironment &environment) {
#ifdef TRAVIS_CI
    return;
#endif
    auto &chillout = Debug::Chillout::getInstance();

#ifdef APPVEYOR
    QString crashesDirRoot = QDir::currentPath();
#else
    QString crashesDirRoot = environment.path({Constants::CRASHES_DIR});
#endif
    QString crashesDirPath = QDir::toNativeSeparators(crashesDirRoot);

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

#ifndef NO_EXIV2
    Exiv2InitHelper exiv2InitHelper;
    Q_UNUSED(exiv2InitHelper);
#endif

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

    int result = 0;
    std::vector<std::shared_ptr<IntegrationTestBase>> integrationTests;

    // always the first one
    integrationTests.emplace_back(std::make_shared<MetadataCacheSaveTest>(environment, xpiksTests));
    // and all others
    integrationTests.emplace_back(std::make_shared<AddFilesBasicTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<AutoAttachVectorsTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<SaveFileBasicTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<SaveFileLegacyTest>(environment, xpiksTests));
#ifndef TRAVIS_CI
    integrationTests.emplace_back(std::make_shared<SaveVideoBasicTest>(environment, xpiksTests));
#endif
    integrationTests.emplace_back(std::make_shared<FailedUploadsTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<SpellCheckMultireplaceTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<SpellCheckCombinedModelTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<ZipArtworksTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<SpellCheckUndoTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<AutoCompleteBasicTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<SpellingProducesWarningsTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<UndoAddWithVectorsTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<ReadLegacySavedTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<ClearMetadataTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<SaveWithEmptyTitleTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<CombinedEditFixSpellingTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<FindAndReplaceModelTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<AddToUserDictionaryTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<AutoDetachVectorTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<RemoveFromUserDictionaryTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<ArtworkUploaderBasicTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<PlainTextEditTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<FixSpellingMarksModifiedTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<PresetsTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<TranslatorBasicTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<UserDictEditTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<WeirdNamesReadTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<RestoreSessionTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<DuplicateSearchTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<AutoCompletePresetsTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<CsvExportTest>(environment, xpiksTests));
#ifndef NO_EXIV2
    integrationTests.emplace_back(std::make_shared<UnicodeIoTest>(environment, xpiksTests));
#endif
    integrationTests.emplace_back(std::make_shared<UndoAddDirectoryTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<UndoRestoreSessionTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<MasterPasswordTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<ReimportTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<AutoImportTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<ImportLostMetadataTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<WarningsCombinedTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<CsvDefaultExportTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<LoadPluginBasicTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<StockFtpAutoCompleteTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<UnavailableFilesTest>(environment, xpiksTests));
    integrationTests.emplace_back(std::make_shared<RemoveDirSavesSessionTest>(environment, xpiksTests));
    // always the last one. insert new tests above
    integrationTests.emplace_back(std::make_shared<LocalLibrarySearchTest>(environment, xpiksTests));

    qInfo("\n");
    int succeededTestsCount = 0, failedTestsCount = 0;
    QStringList failedTests;

    for (auto &test: integrationTests) {
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

    qInfo() << "--------------------------";
    qInfo() << "In memory run:" << environment.getIsInMemoryOnly();
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
    xpiksTests.waitFinalized();

    std::cout << "Integration tests finished" << std::endl;

    return result;
}
