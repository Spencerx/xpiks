/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#include <QUrl>
#include <QDir>
#include <QtQml>
#include <QFile>
#include <QUuid>
#include <QScreen>
#include <QtDebug>
#include <QDateTime>
#include <QSettings>
#include <QTextStream>
#include <QQmlContext>
#include <QApplication>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QQmlApplicationEngine>
#include <QDesktopWidget>
// -------------------------------------
#include "AutoComplete/keywordsautocompletemodel.h"
#include "SpellCheck/spellchecksuggestionmodel.h"
#include "QMLExtensions/cachingimageprovider.h"
#include "Models/filteredartitemsproxymodel.h"
#include "QMLExtensions/imagecachingservice.h"
#include "MetadataIO/metadataiocoordinator.h"
#include "AutoComplete/autocompleteservice.h"
#include "Connectivity/analyticsuserevent.h"
#include "SpellCheck/spellcheckerservice.h"
#include "Models/deletekeywordsviewmodel.h"
#include "Translation/translationmanager.h"
#include "Translation/translationservice.h"
#include "Models/recentdirectoriesmodel.h"
#include "QMLExtensions/triangleelement.h"
#include "Connectivity/telemetryservice.h"
#include "MetadataIO/metadataioservice.h"
#include "Suggestion/keywordssuggestor.h"
#include "SpellCheck/userdicteditmodel.h"
#include "Models/combinedartworksmodel.h"
#include "QMLExtensions/folderelement.h"
#include "Helpers/globalimageprovider.h"
#include "Models/uploadinforepository.h"
#include "Connectivity/curlinithelper.h"
#include "Connectivity/updateservice.h"
#include "Helpers/helpersqmlwrapper.h"
#include "Encryption/secretsmanager.h"
#include "Models/artworksrepository.h"
#include "QMLExtensions/colorsmodel.h"
#include "Models/artworkproxymodel.h"
#include "Warnings/warningsservice.h"
#include "UndoRedo/undoredomanager.h"
#include "Models/recentfilesmodel.h"
#include "QuickBuffer/quickbuffer.h"
#include "Helpers/clipboardhelper.h"
#include "Commands/commandmanager.h"
#include "QMLExtensions/tabsmodel.h"
#include "Models/artworkuploader.h"
#include "Warnings/warningsmodel.h"
#include "Plugins/pluginmanager.h"
#include "Helpers/loggingworker.h"
#include "Models/languagesmodel.h"
#include "Models/sessionmanager.h"
#include "Models/artitemsmodel.h"
#include "Models/settingsmodel.h"
#include "Models/ziparchiver.h"
#include "Helpers/constants.h"
#include "Helpers/runguard.h"
#include "Models/logsmodel.h"
#include "Models/uimanager.h"
#include "Helpers/database.h"
#include "Helpers/logger.h"
#include "Common/version.h"
#include "Common/defines.h"
#include "Models/proxysettings.h"
#include "Models/findandreplacemodel.h"
#include "Models/previewartworkelement.h"
#include "Maintenance/maintenanceservice.h"
#include "QMLExtensions/artworksupdatehub.h"
#include "QMLExtensions/videocachingservice.h"
#include "KeywordsPresets/presetkeywordsmodel.h"
#include "Models/switchermodel.h"
#include "Connectivity/requestsservice.h"
#include "SpellCheck/duplicatesreviewmodel.h"
#include "MetadataIO/csvexportmodel.h"
#include "KeywordsPresets/presetgroupsmodel.h"
#include <ftpcoordinator.h>
#include "Helpers/filehelpers.h"
#include "Common/systemenvironment.h"

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
    logger.log(logLine);

    if (type == QtFatalMsg) {
        logger.flush();
        abort();
    }
}

void initQSettings() {
    QCoreApplication::setOrganizationName(Constants::ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(Constants::ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(Constants::APPLICATION_NAME);
    QString appVersion(STRINGIZE(BUILDNUMBER));
    QCoreApplication::setApplicationVersion(XPIKS_VERSION_STRING " " STRINGIZE(XPIKS_VERSION_SUFFIX) " - " +
                                            appVersion.left(10));
}

void ensureUserIdExists(Models::SettingsModel *settings) {
    QString userID = settings->getUserAgentId();
    QUuid latest(userID);
    if (userID.isEmpty()
            || (latest.isNull())
            || (latest.version() == QUuid::VerUnknown)) {
        QUuid uuid = QUuid::createUuid();
        settings->setUserAgentId(uuid.toString());
    }
}

void setHighDpiEnvironmentVariable() {
#ifdef Q_OS_WIN
    static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
    if (!qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO)
            && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }
#endif
}

QString getRunGuardName() {
#ifdef QT_NO_DEBUG
    QString runGuardName = "xpiks_";
#else
    QString runGuardName = "xpiks-debug_";
#endif

    QString username = QString::fromLocal8Bit(qgetenv("USER"));
    if (username.isEmpty()) {
        username = QString::fromLocal8Bit(qgetenv("USERNAME"));
    }

    username.remove(QChar::Space);

    return (runGuardName + username);
}

int main(int argc, char *argv[]) {
    const QString runGuardName = getRunGuardName();
    Helpers::RunGuard guard(runGuardName);
    if (!guard.tryToRun()) {
        std::cerr << "Xpiks is already running";
        return -1;
    }

    // will call curl_global_init and cleanup
    Connectivity::CurlInitHelper curlInitHelper;
    Q_UNUSED(curlInitHelper);

    setHighDpiEnvironmentVariable();

    qRegisterMetaTypeStreamOperators<Models::ProxySettings>("ProxySettings");
    qRegisterMetaType<Common::SpellCheckFlags>("Common::SpellCheckFlags");
    initQSettings();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    qSetMessagePattern("%{time hh:mm:ss.zzz} %{type} T#%{threadid} %{function} - %{message}");
#endif

    qInstallMessageHandler(myMessageHandler);

    // ----------------------------------------------
    QApplication app(argc, argv);
    Common::SystemEnvironment environment(app.arguments());
    environment.ensureSystemDirectoriesExist();
    // ----------------------------------------------

#ifdef WITH_LOGS
    {
        QString time = QDateTime::currentDateTimeUtc().toString("ddMMyyyy-hhmmss-zzz");
        QString logFilename = QString("xpiks-qt-%1.log").arg(time);

        QString logFilePath = environment.fileInDir(logFilename, Constants::LOGS_DIR);

        Helpers::Logger &logger = Helpers::Logger::getInstance();
        logger.setLogFilePath(logFilePath);
    }
#endif

    QMLExtensions::ColorsModel colorsModel;
    Models::LogsModel logsModel(&colorsModel);
    logsModel.startLogging();

    LOG_INFO << "Log started. Today is" << QDateTime::currentDateTimeUtc().toString("dd.MM.yyyy");
    LOG_INFO << "Xpiks" << XPIKS_FULL_VERSION_STRING << "-" << STRINGIZE(BUILDNUMBER);
    LOG_INFO << QSysInfo::productType() << QSysInfo::productVersion() << QSysInfo::currentCpuArchitecture();
    LOG_INFO << "Working directory of Xpiks is:" << QDir::currentPath();

    Models::SettingsModel settingsModel(environment);
    settingsModel.initializeConfigs();
    settingsModel.retrieveAllValues();
    ensureUserIdExists(&settingsModel);

    QString userId = settingsModel.getUserAgentId();
    userId.remove(QRegExp("[{}-]."));

    Models::ArtworksRepository artworkRepository;
    Models::FilteredArtworksRepository filteredArtworksRepository(&artworkRepository);
    Models::ArtItemsModel artItemsModel;
    Models::CombinedArtworksModel combinedArtworksModel;
    Models::UploadInfoRepository uploadInfoRepository(environment);
    KeywordsPresets::PresetKeywordsModel presetsModel(environment);
    KeywordsPresets::FilteredPresetKeywordsModel filteredPresetsModel;
    filteredPresetsModel.setSourceModel(&presetsModel);
    Warnings::WarningsService warningsService(environment);
    Encryption::SecretsManager secretsManager;
    UndoRedo::UndoRedoManager undoRedoManager;
    Models::ZipArchiver zipArchiver;
    Suggestion::KeywordsSuggestor keywordsSuggestor(environment);
    Models::FilteredArtItemsProxyModel filteredArtItemsModel;
    filteredArtItemsModel.setSourceModel(&artItemsModel);
    Models::RecentDirectoriesModel recentDirectorieModel(environment);
    Models::RecentFilesModel recentFileModel(environment);
    libxpks::net::FtpCoordinator *ftpCoordinator = new libxpks::net::FtpCoordinator(settingsModel.getMaxParallelUploads());
    Models::ArtworkUploader artworkUploader(environment, ftpCoordinator);
    SpellCheck::SpellCheckerService spellCheckerService(environment, &settingsModel);
    SpellCheck::SpellCheckSuggestionModel spellCheckSuggestionModel;
    SpellCheck::UserDictEditModel userDictEditModel;
    MetadataIO::MetadataIOService metadataIOService;
    Warnings::WarningsModel warningsModel;
    warningsModel.setSourceModel(&artItemsModel);
    warningsModel.setWarningsSettingsModel(warningsService.getWarningsSettingsModel());
    Models::LanguagesModel languagesModel;
    AutoComplete::KeywordsAutoCompleteModel autoCompleteModel;
    AutoComplete::AutoCompleteService autoCompleteService(&autoCompleteModel, &presetsModel, &settingsModel);
    QMLExtensions::ImageCachingService imageCachingService(environment);
    Models::FindAndReplaceModel replaceModel(&colorsModel);
    Models::DeleteKeywordsViewModel deleteKeywordsModel;
    Models::ArtworkProxyModel artworkProxyModel;
    Translation::TranslationManager translationManager(environment);
    Translation::TranslationService translationService(translationManager);
    Models::UIManager uiManager(environment, &settingsModel);
    Models::SessionManager sessionManager(environment);
    sessionManager.initialize();
    QuickBuffer::QuickBuffer quickBuffer;
    Maintenance::MaintenanceService maintenanceService(environment);
    QMLExtensions::VideoCachingService videoCachingService(environment);
    QMLExtensions::ArtworksUpdateHub artworksUpdateHub;
    artworksUpdateHub.setStandardRoles(artItemsModel.getArtworkStandardRoles());
    Models::SwitcherModel switcherModel(environment);
    Connectivity::RequestsService requestsService;
    Helpers::DatabaseManager databaseManager(environment);
    SpellCheck::DuplicatesReviewModel duplicatesModel(&colorsModel);
    MetadataIO::CsvExportModel csvExportModel(environment);

    Connectivity::UpdateService updateService(environment, &settingsModel, &switcherModel, &maintenanceService);

    MetadataIO::MetadataIOCoordinator metadataIOCoordinator;

#if defined(QT_NO_DEBUG)
    const bool telemetryEnabled = settingsModel.getIsTelemetryEnabled();
#else
    const bool telemetryEnabled = false;
#endif
    Connectivity::TelemetryService telemetryService(userId, telemetryEnabled);

    Plugins::PluginManager pluginManager(environment);
    Plugins::PluginsWithActionsModel pluginsWithActions;
    pluginsWithActions.setSourceModel(&pluginManager);

    Helpers::HelpersQmlWrapper helpersQmlWrapper(environment, &colorsModel);

    LOG_INFO << "Models created";

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
    commandManager.InjectDependency(&replaceModel);
    commandManager.InjectDependency(&deleteKeywordsModel);
    commandManager.InjectDependency(&helpersQmlWrapper);
    commandManager.InjectDependency(&presetsModel);
    commandManager.InjectDependency(&translationManager);
    commandManager.InjectDependency(&translationService);
    commandManager.InjectDependency(&uiManager);
    commandManager.InjectDependency(&artworkProxyModel);
    commandManager.InjectDependency(&sessionManager);
    commandManager.InjectDependency(&warningsModel);
    commandManager.InjectDependency(&quickBuffer);
    commandManager.InjectDependency(&maintenanceService);
    commandManager.InjectDependency(&videoCachingService);
    commandManager.InjectDependency(&artworksUpdateHub);
    commandManager.InjectDependency(&switcherModel);
    commandManager.InjectDependency(&requestsService);
    commandManager.InjectDependency(&databaseManager);
    commandManager.InjectDependency(&duplicatesModel);
    commandManager.InjectDependency(&csvExportModel);

    userDictEditModel.setCommandManager(&commandManager);

    commandManager.ensureDependenciesInjected();

    // other initializations
    secretsManager.setMasterPasswordHash(settingsModel.getMasterPasswordHash());
    recentDirectorieModel.initialize();
    recentFileModel.initialize();

    commandManager.connectEntitiesSignalsSlots();

    languagesModel.initFirstLanguage();
    languagesModel.loadLanguages();

    colorsModel.initializeBuiltInThemes();
    logsModel.InjectDependency(&colorsModel);

    telemetryService.setInterfaceLanguage(languagesModel.getCurrentLanguage());
    colorsModel.applyTheme(settingsModel.getSelectedThemeIndex());

    qmlRegisterType<Helpers::ClipboardHelper>("xpiks", 1, 0, "ClipboardHelper");
    qmlRegisterType<QMLExtensions::TriangleElement>("xpiks", 1, 0, "TriangleElement");
    qmlRegisterType<QMLExtensions::FolderElement>("xpiks", 1, 0, "FolderElement");

    QQmlApplicationEngine engine;
    Helpers::GlobalImageProvider *globalProvider = new Helpers::GlobalImageProvider(QQmlImageProviderBase::Image);
    QMLExtensions::CachingImageProvider *cachingProvider = new QMLExtensions::CachingImageProvider(QQmlImageProviderBase::Image);
    cachingProvider->setImageCachingService(&imageCachingService);

    QQmlContext *rootContext = engine.rootContext();
    rootContext->setContextProperty("artItemsModel", &artItemsModel);
    rootContext->setContextProperty("artworkRepository", &filteredArtworksRepository);
    rootContext->setContextProperty("combinedArtworks", &combinedArtworksModel);
    rootContext->setContextProperty("secretsManager", &secretsManager);
    rootContext->setContextProperty("undoRedoManager", &undoRedoManager);
    rootContext->setContextProperty("keywordsSuggestor", &keywordsSuggestor);
    rootContext->setContextProperty("settingsModel", &settingsModel);
    rootContext->setContextProperty("filteredArtItemsModel", &filteredArtItemsModel);
    rootContext->setContextProperty("helpersWrapper", &helpersQmlWrapper);
    rootContext->setContextProperty("recentDirectories", &recentDirectorieModel);
    rootContext->setContextProperty("recentFiles", &recentFileModel);
    rootContext->setContextProperty("metadataIOCoordinator", &metadataIOCoordinator);
    rootContext->setContextProperty("pluginManager", &pluginManager);
    rootContext->setContextProperty("pluginsWithActions", &pluginsWithActions);
    rootContext->setContextProperty("warningsModel", &warningsModel);
    rootContext->setContextProperty("languagesModel", &languagesModel);
    rootContext->setContextProperty("i18", &languagesModel);
    rootContext->setContextProperty("uiColors", &colorsModel);
    rootContext->setContextProperty("acSource", &autoCompleteModel);
    rootContext->setContextProperty("replaceModel", &replaceModel);
    rootContext->setContextProperty("presetsModel", &presetsModel);
    rootContext->setContextProperty("filteredPresetsModel", &filteredPresetsModel);
    rootContext->setContextProperty("artworkProxy", &artworkProxyModel);
    rootContext->setContextProperty("translationManager", &translationManager);
    rootContext->setContextProperty("uiManager", &uiManager);
    rootContext->setContextProperty("quickBuffer", &quickBuffer);
    rootContext->setContextProperty("userDictEditModel", &userDictEditModel);
    rootContext->setContextProperty("switcher", &switcherModel);
    rootContext->setContextProperty("duplicatesModel", &duplicatesModel);
    rootContext->setContextProperty("csvExportModel", &csvExportModel);
    rootContext->setContextProperty("presetsGroups", presetsModel.getGroupsModel());

    rootContext->setContextProperty("tabsModel", uiManager.getTabsModel());
    rootContext->setContextProperty("activeTabs", uiManager.getActiveTabs());
    rootContext->setContextProperty("inactiveTabs", uiManager.getInactiveTabs());

#ifdef QT_DEBUG
    QVariant isDebug(true);
#else
    QVariant isDebug(false);
#endif
    rootContext->setContextProperty("debug", isDebug);
    rootContext->setContextProperty("debugTabs", isDebug);

    engine.addImageProvider("global", globalProvider);
    engine.addImageProvider("cached", cachingProvider);

    uiManager.addSystemTab(FILES_FOLDERS_TAB_ID, "qrc:/CollapserTabs/FilesFoldersIcon.qml", "qrc:/CollapserTabs/FilesFoldersTab.qml");
    uiManager.addSystemTab(QUICKBUFFER_TAB_ID, "qrc:/CollapserTabs/QuickBufferIcon.qml", "qrc:/CollapserTabs/QuickBufferTab.qml");
    uiManager.addSystemTab(TRANSLATOR_TAB_ID, "qrc:/CollapserTabs/TranslatorIcon.qml", "qrc:/CollapserTabs/TranslatorTab.qml");
    uiManager.initializeSystemTabs();
    uiManager.initializeState();

    LOG_DEBUG << "About to load main view...";
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    LOG_DEBUG << "Main view loaded";

    auto *uiProvider = pluginManager.getUIProvider();
    uiProvider->setQmlEngine(&engine);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().at(0));
    imageCachingService.setScale(window->effectiveDevicePixelRatio());

    QScreen *screen = window->screen();
    QObject::connect(window, &QQuickWindow::screenChanged, &imageCachingService, &QMLExtensions::ImageCachingService::screenChangedHandler);
    QObject::connect(screen, &QScreen::logicalDotsPerInchChanged, &imageCachingService, &QMLExtensions::ImageCachingService::dpiChanged);
    QObject::connect(screen, &QScreen::physicalDotsPerInchChanged, &imageCachingService, &QMLExtensions::ImageCachingService::dpiChanged);

    uiProvider->setRoot(window->contentItem());
    uiProvider->setUIManager(&uiManager);

    commandManager.afterConstructionCallback();

    return app.exec();
}
