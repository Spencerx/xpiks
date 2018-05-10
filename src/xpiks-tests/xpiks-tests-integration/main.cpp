#include <iostream>
#include <exception>
#include <cstdlib>

#include <QDebug>
#include <QTimer>
#include <QCoreApplication>

#include "../../xpiks-qt/SpellCheck/spellchecksuggestionmodel.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "../../xpiks-qt/QMLExtensions/imagecachingservice.h"
#include "../../xpiks-qt/QMLExtensions/videocachingservice.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/AutoComplete/autocompleteservice.h"
#include "../../xpiks-qt/QMLExtensions/artworksupdatehub.h"
#include "../../xpiks-qt/Models/deletekeywordsviewmodel.h"
#include "../../xpiks-qt/Connectivity/analyticsuserevent.h"
#include "../../xpiks-qt/SpellCheck/spellcheckerservice.h"
#include "../../xpiks-qt/AutoComplete/keywordsautocompletemodel.h"
#include "../../xpiks-qt/Translation/translationmanager.h"
#include "../../xpiks-qt/Translation/translationservice.h"
#include "../../xpiks-qt/Models/recentdirectoriesmodel.h"
#include "../../xpiks-qt/Models/recentfilesmodel.h"
#include "../../xpiks-qt/MetadataIO/metadataioservice.h"
#include "../../xpiks-qt/Suggestion/keywordssuggestor.h"
#include "../../xpiks-qt/Models/combinedartworksmodel.h"
#include "../../xpiks-qt/Connectivity/telemetryservice.h"
#include "../../xpiks-qt/Helpers/globalimageprovider.h"
#include "../../xpiks-qt/Models/uploadinforepository.h"
#include <ftpcoordinator.h>
#include "../../xpiks-qt/Models/findandreplacemodel.h"
#include "../../xpiks-qt/Connectivity/curlinithelper.h"
#include "../../xpiks-qt/Helpers/helpersqmlwrapper.h"
#include "../../xpiks-qt/Connectivity/updateservice.h"
#include "../../xpiks-qt/Encryption/secretsmanager.h"
#include "../../xpiks-qt/Models/artworksrepository.h"
#include "../../xpiks-qt/QMLExtensions/colorsmodel.h"
#include "../../xpiks-qt/Warnings/warningsservice.h"
#include "../../xpiks-qt/Models/artworkproxymodel.h"
#include "../../xpiks-qt/UndoRedo/undoredomanager.h"
#include "../../xpiks-qt/Helpers/clipboardhelper.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/QuickBuffer/quickbuffer.h"
#include "../../xpiks-qt/Models/artworkuploader.h"
#include "../../xpiks-qt/Warnings/warningsmodel.h"
#include "../../xpiks-qt/Plugins/pluginmanager.h"
#include "../../xpiks-qt/Helpers/loggingworker.h"
#include "../../xpiks-qt/Models/languagesmodel.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/ziparchiver.h"
#include "../../xpiks-qt/Models/sessionmanager.h"
#include "../../xpiks-qt/Helpers/constants.h"
#include "../../xpiks-qt/Helpers/runguard.h"
#include "../../xpiks-qt/Models/logsmodel.h"
#include "../../xpiks-qt/Helpers/logger.h"
#include "../../xpiks-qt/Common/version.h"
#include "../../xpiks-qt/Common/defines.h"
#include "../../xpiks-qt/Storage/databasemanager.h"
#include "../../xpiks-qt/KeywordsPresets/presetkeywordsmodel.h"
#include "../../xpiks-qt/Maintenance/maintenanceservice.h"
#include "../../xpiks-qt/Connectivity/requestsservice.h"
#include "../../xpiks-qt/SpellCheck/duplicatesreviewmodel.h"
#include "../../xpiks-qt/MetadataIO/csvexportmodel.h"
#include "../../xpiks-qt/Models/switchermodel.h"
#include "../../xpiks-qt/Helpers/filehelpers.h"
#include "../../xpiks-qt/Models/uimanager.h"
#include "../../xpiks-qt/Microstocks/microstockapiclients.h"
#include "../../xpiks-qt/Encryption/isecretsstorage.h"
#include <apisecretsstorage.h>
#include "integrationtestsenvironment.h"

#include "exiv2iohelpers.h"

#ifdef Q_OS_WIN
#include "windowscrashhandler.h"
#endif

#ifdef Q_OS_LINUX
#include <signal.h>
#include <exception>
#include <cstdlib>
#endif

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
        logger.emergencyFlush();
    }

    if (type == QtFatalMsg) {
        abort();
    }
}

#if defined(Q_OS_LINUX) && defined(QT_DEBUG)
void linuxAbortHandler(int signalNumber) {
    Helpers::Logger &logger = Helpers::Logger::getInstance();
    logger.emergencyFlush();
}
#endif

void initQSettings() {
    QCoreApplication::setOrganizationName(Constants::ORGANIZATION_NAME);
    QCoreApplication::setApplicationName(Constants::APPLICATION_NAME);
}

int main(int argc, char *argv[]) {
    initQSettings();

#if defined(Q_OS_WIN) && defined(APPVEYOR)
    WindowsCrashHandler crashHandler;
    crashHandler.SetProcessExceptionHandlers();
    crashHandler.SetThreadExceptionHandlers();
#endif

#if defined(Q_OS_LINUX) && defined(QT_DEBUG)
    signal(SIGABRT, &linuxAbortHandler);
    std::set_terminate([](){
        Helpers::Logger &logger = Helpers::Logger::getInstance();
        logger.emergencyFlush();
        std::abort();
    });
#endif

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

    Models::LogsModel logsModel;
    logsModel.startLogging();

    Models::SettingsModel settingsModel(environment);
    settingsModel.initializeConfigs();
    settingsModel.retrieveAllValues();

    QMLExtensions::ColorsModel colorsModel;

    Models::ArtworksRepository artworkRepository;
    Models::ArtItemsModel artItemsModel;
    Models::CombinedArtworksModel combinedArtworksModel;
    Models::UploadInfoRepository uploadInfoRepository(environment);
    KeywordsPresets::PresetKeywordsModel presetsModel(environment);
    Warnings::WarningsService warningsService(environment);
    Encryption::SecretsManager secretsManager;
    UndoRedo::UndoRedoManager undoRedoManager;
    Models::ZipArchiver zipArchiver;
    Storage::DatabaseManager databaseManager(environment);
    std::shared_ptr<Encryption::ISecretsStorage> secretsStorage(new libxpks::microstocks::APISecretsStorage());
    Microstocks::MicrostockAPIClients apiClients(secretsStorage.get());
    Connectivity::RequestsService requestsService(settingsModel.getProxySettings());
    Suggestion::KeywordsSuggestor keywordsSuggestor(apiClients, requestsService, environment);
    Models::FilteredArtItemsProxyModel filteredArtItemsModel;
    filteredArtItemsModel.setSourceModel(&artItemsModel);
    Models::RecentDirectoriesModel recentDirectorieModel(environment);
    Models::RecentFilesModel recentFileModel(environment);
    libxpks::net::FtpCoordinator *ftpCoordinator = new libxpks::net::FtpCoordinator(settingsModel.getMaxParallelUploads());
    Models::ArtworkUploader artworkUploader(environment, ftpCoordinator);
    SpellCheck::SpellCheckerService spellCheckerService(environment, &settingsModel);
    SpellCheck::SpellCheckSuggestionModel spellCheckSuggestionModel;
    MetadataIO::MetadataIOService metadataIOService(&databaseManager);
    Warnings::WarningsModel warningsModel;
    warningsModel.setSourceModel(&artItemsModel);
    Models::LanguagesModel languagesModel;
    AutoComplete::KeywordsAutoCompleteModel autoCompleteModel;
    AutoComplete::AutoCompleteService autoCompleteService(&autoCompleteModel, &presetsModel, &settingsModel);
    QMLExtensions::ImageCachingService imageCachingService(environment, &databaseManager);
    Models::FindAndReplaceModel findAndReplaceModel(&colorsModel);
    Models::DeleteKeywordsViewModel deleteKeywordsModel;
    Translation::TranslationManager translationManager(environment);
    Translation::TranslationService translationService(translationManager);
    Models::ArtworkProxyModel artworkProxy;
    Models::UIManager uiManager(environment, &settingsModel);
    Models::SessionManager sessionManager(environment);
    // intentional memory leak to beat spellcheck lock stuff
    QuickBuffer::QuickBuffer quickBuffer;
    Maintenance::MaintenanceService maintenanceService(environment);

    QMLExtensions::VideoCachingService videoCachingService(environment, &databaseManager);
    QMLExtensions::ArtworksUpdateHub artworksUpdateHub;
    artworksUpdateHub.setStandardRoles(artItemsModel.getArtworkStandardRoles());
    Models::SwitcherModel switcherModel(environment);
    Connectivity::UpdateService updateService(environment, &settingsModel, &switcherModel, &maintenanceService);

    MetadataIO::MetadataIOCoordinator metadataIOCoordinator;
    Connectivity::TelemetryService telemetryService("1234567890", false);
    Plugins::PluginManager pluginManager(environment, &databaseManager, requestsService, apiClients);
    SpellCheck::DuplicatesReviewModel duplicatesModel(&colorsModel);
    MetadataIO::CsvExportModel csvExportModel(environment);    

    auto *uiProvider = pluginManager.getUIProvider();
    uiProvider->setUIManager(&uiManager);

    Commands::CommandManager commandManager;
    commandManager.InjectDependency(&artworkRepository);
    commandManager.InjectDependency(&artItemsModel);
    commandManager.InjectDependency(&filteredArtItemsModel);
    commandManager.InjectDependency(&combinedArtworksModel);
    commandManager.InjectDependency(&artworkUploader);
    commandManager.InjectDependency(&uploadInfoRepository);
    commandManager.InjectDependency(&warningsService);
    commandManager.InjectDependency(&secretsManager);
    commandManager.InjectDependency(&undoRedoManager);
    commandManager.InjectDependency(&zipArchiver);
    commandManager.InjectDependency(&keywordsSuggestor);
    commandManager.InjectDependency(&settingsModel);
    commandManager.InjectDependency(&recentDirectorieModel);
    commandManager.InjectDependency(&recentFileModel);
    commandManager.InjectDependency(&spellCheckerService);
    commandManager.InjectDependency(&spellCheckSuggestionModel);
    commandManager.InjectDependency(&metadataIOService);
    commandManager.InjectDependency(&telemetryService);
    commandManager.InjectDependency(&updateService);
    commandManager.InjectDependency(&logsModel);
    commandManager.InjectDependency(&metadataIOCoordinator);
    commandManager.InjectDependency(&pluginManager);
    commandManager.InjectDependency(&languagesModel);
    commandManager.InjectDependency(&colorsModel);
    commandManager.InjectDependency(&autoCompleteService);
    commandManager.InjectDependency(&autoCompleteModel);
    commandManager.InjectDependency(&imageCachingService);
    commandManager.InjectDependency(&findAndReplaceModel);
    commandManager.InjectDependency(&deleteKeywordsModel);
    commandManager.InjectDependency(&presetsModel);
    commandManager.InjectDependency(&translationManager);
    commandManager.InjectDependency(&translationService);
    commandManager.InjectDependency(&artworkProxy);
    commandManager.InjectDependency(&sessionManager);
    commandManager.InjectDependency(&warningsModel);
    commandManager.InjectDependency(&quickBuffer);
    commandManager.InjectDependency(&maintenanceService);
    commandManager.InjectDependency(&videoCachingService);
    commandManager.InjectDependency(&switcherModel);
    commandManager.InjectDependency(&requestsService);
    commandManager.InjectDependency(&artworksUpdateHub);
    commandManager.InjectDependency(&databaseManager);
    commandManager.InjectDependency(&duplicatesModel);
    commandManager.InjectDependency(&csvExportModel);

    commandManager.ensureDependenciesInjected();

    secretsManager.setMasterPasswordHash(settingsModel.getMasterPasswordHash());
    recentDirectorieModel.initialize();
    recentFileModel.initialize();

#if defined(Q_OS_WIN)
    #if defined(APPVEYOR)
        settingsModel.setExifToolPath("c:/projects/xpiks-deps/windows-3rd-party-bin/exiftool.exe");
    #else
        settingsModel.setExifToolPath(findFullPathForTests("xpiks-qt/deps/exiftool.exe"));
    #endif
#elif defined(Q_OS_MAC)
    settingsModel.setExifToolPath(findFullPathForTests("xpiks-qt/deps/exiftool/exiftool"));
#endif

    switcherModel.setRemoteConfigOverride(findFullPathForTests("configs-for-tests/tests_switches.json"));
    QString csvExportPlansPath;
    if (!tryFindFullPathForTests("api/v1/csv_export_plans.json", csvExportPlansPath)) {
        if (!tryFindFullPathForTests("xpiks-api/api/v1/csv_export_plans.json", csvExportPlansPath)) {
            // fallback to copy-pasted and probably not enough frequently updated just for the sake of tests
            tryFindFullPathForTests("configs-for-tests/csv_export_plans.json", csvExportPlansPath);
        }
    }
    csvExportModel.setRemoteConfigOverride(csvExportPlansPath);

    QString stocksFtpPath;
    if (!tryFindFullPathForTests("api/v1/stocks_ftp.json", stocksFtpPath)) {
        if (!tryFindFullPathForTests("xpiks-api/api/v1/stocks_ftp.json", stocksFtpPath)) {
            // fallback to copy-pasted and probably not enough frequently updated just for the sake of tests
            tryFindFullPathForTests("configs-for-tests/stocks_ftp.json", stocksFtpPath);
        }
    }
    artworkUploader.accessStocksList().setRemoteOverride(stocksFtpPath);

    commandManager.connectEntitiesSignalsSlots();
    commandManager.afterConstructionCallback();

    // process signals from construction
    QCoreApplication::processEvents();

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

    commandManager.beforeDestructionCallback();

    // for the logs to appear
    app.processEvents();

    QThread::sleep(1);

    std::cout << "Integration tests finished" << std::endl;

    return result;
}

