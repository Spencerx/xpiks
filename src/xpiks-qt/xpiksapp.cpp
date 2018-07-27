/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xpiksapp.h"
#include <apisecretsstorage.h>
#include <QQmlContext>
#include <QQuickWindow>
#include <QScreen>
#include <Encryption/aes-qt.h>
#include <signal.h>
#include <Filesystem/filescollection.h>
#include <Filesystem/directoriescollection.h>
#include <Filesystem/filesdirectoriescollection.h>
#include <Commands/Services/savesessioncommand.h>
#include <Commands/Files/addfilescommand.h>
#include <Commands/Services/cleanuplegacybackupscommand.h>
#include <Commands/Editing/readmetadatatemplate.h>
#include <Commands/Base/templatedcommand.h>
#include <Commands/Files/addtorecenttemplate.h>
#include <Commands/Services/autoimportmetadatacommand.h>
#include <Commands/Base/compositecommandtemplate.h>
#include <Commands/Services/generatethumbnailstemplate.h>
#include <Commands/artworksupdatetemplate.h>
#include <Commands/UI/selectedartworkscommands.h>
#include <Commands/UI/singleeditablecommands.h>

XpiksApp::XpiksApp(Common::ISystemEnvironment &environment):
    m_SettingsModel(environment, m_SecretsManager),
    m_DatabaseManager(environment),
    m_SessionManager(environment),
    m_CommandManager(m_UndoRedoManager),
    m_SwitcherModel(environment),
    m_UICommandDispatcher(m_CommandManager),
    m_PresetsModel(environment),
    m_FilteredPresetsModel(m_PresetsModel),
    m_KeywordsCompletions(),
    m_KeywordsAutoCompleteModel(m_KeywordsCompletions),
    m_CurrentEditableModel(),
    m_RecentDirectorieModel(environment),
    m_RecentFileModel(environment),
    m_ArtworksRepository(m_RecentDirectorieModel),
    m_FilteredArtworksRepository(m_ArtworksRepository),
    m_ArtworksListModel(m_ArtworksRepository),
    m_ArtworksUpdateHub(m_ArtworksListModel),
    m_CombinedArtworksModel(m_CommandManager, m_PresetsModel, m_KeywordsCompletions),
    m_QuickBuffer(m_CurrentEditableModel, m_CommandManager),
    m_ReplaceModel(m_ColorsModel, m_CommandManager),
    m_DeleteKeywordsModel(m_CommandManager, m_PresetsModel),
    m_ArtworkProxyModel(m_CommandManager, m_PresetsModel, m_KeywordsCompletions, m_ArtworksUpdateHub),
    m_DuplicatesModel(m_ColorsModel),
    m_MaintenanceService(environment),
    m_AutoCompleteService(m_KeywordsAutoCompleteModel, m_PresetsModel, m_SettingsModel),
    m_RequestsService(m_SettingsModel.getProxySettings()),
    m_WarningsSettingsModel(environment),
    m_WarningsService(m_WarningsSettingsModel),
    m_SpellCheckerService(environment, m_WarningsService, m_SettingsModel),
    m_MetadataIOService(),
    m_ImageCachingService(environment, m_DatabaseManager),
    m_TranslationService(),
    m_VideoCachingService(environment, m_DatabaseManager, m_SwitcherModel),
    m_UpdateService(environment, m_SettingsModel, m_SwitcherModel, m_MaintenanceService),
    m_TelemetryService(m_SwitcherModel, m_SettingsModel),
    m_InspectionHub(m_SpellCheckerService, m_WarningsService, m_SettingsModel),
    m_SecretsManager(),
    m_UploadInfoRepository(environment, m_SecretsManager),
    m_ZipArchiver(),
    m_SecretsStorage(new libxpks::microstocks::APISecretsStorage()),
    m_ApiClients(m_SecretsStorage.get()),
    m_FtpCoordinator(new libxpks::net::FtpCoordinator(m_SecretsManager, m_SettingsModel)),
    m_ArtworkUploader(environment, m_UploadInfoRepository, m_SettingsModel),
    m_LanguagesModel(m_SettingsModel),
    m_UIManager(environment, m_SettingsModel),
    m_UserDictionary(environment),
    m_UserDictEditModel(m_UserDictionary),
    m_WarningsModel(m_ArtworksListModel, m_WarningsSettingsModel),
    m_TranslationManager(environment, m_TranslationService),
    m_CsvExportModel(environment),
    m_MetadataReadingHub(m_MetadataIOService, m_ArtworksUpdateHub, m_InspectionHub),
    m_MetadataIOCoordinator(m_MetadataReadingHub, m_SettingsModel, m_SwitcherModel, m_VideoCachingService),
    m_SpellSuggestionModel(m_SpellCheckerService, m_ArtworksUpdateHub),
    m_FilteredArtworksListModel(m_ArtworksListModel, m_CommandManager, m_PresetsModel,
                                m_KeywordsCompletions, m_SettingsModel),
    m_KeywordsSuggestor(m_ApiClients, m_RequestsService, m_SwitcherModel, m_SettingsModel, environment),
    m_PluginManager(environment, m_CommandManager, m_PresetsModel, m_DatabaseManager,
                    m_RequestsService, m_ApiClients, m_CurrentEditableModel, m_UIManager),
    m_PluginsWithActions(m_PluginManager),
    m_HelpersQmlWrapper(environment, m_ColorsModel)
{
    QObject::connect(&m_InitCoordinator, &Helpers::AsyncCoordinator::statusReported,
                     this, &XpiksApp::servicesInitialized);
}

XpiksApp::~XpiksApp() {
    m_LogsModel.stopLogging();
}

bool XpiksApp::getIsUpdateDownloaded() {
    return m_UpdateService.getIsUpdateDownloaded();
}

bool XpiksApp::getPluginsAvailable() const {
    bool result = false;
#ifdef WITH_PLUGINS
    result = true;
#endif
    return result;
}

Plugins::UIProvider &XpiksApp::getUIProvider() {
    return m_PluginManager.getUIProvider();
}

void XpiksApp::startLogging() {
    m_LogsModel.startLogging();
}

void XpiksApp::initialize() {
    m_SettingsModel.initializeConfigs();
    m_SettingsModel.retrieveAllValues();

    m_TelemetryService.initialize();

    m_FtpCoordinator.reset(new libxpks::net::FtpCoordinator(m_SecretsManager, m_SettingsModel));
    m_ArtworkUploader.setFtpCoordinator(m_FtpCoordinator);

    // other initializations
    m_SecretsManager.setMasterPasswordHash(m_SettingsModel.getMasterPasswordHash());
    m_RecentDirectorieModel.initialize();
    m_RecentFileModel.initialize();

    connectEntitiesSignalsSlots();
    registerUICommands();

    m_LanguagesModel.initFirstLanguage();
    m_LanguagesModel.loadLanguages();

    m_ColorsModel.initializeBuiltInThemes();
    m_LogsModel.InjectDependency(&m_ColorsModel);

    m_TelemetryService.setInterfaceLanguage(m_LanguagesModel.getCurrentLanguage());
    m_ColorsModel.applyTheme(m_SettingsModel.getSelectedThemeIndex());

    m_AvailabilityListeners.append(&m_CombinedArtworksModel);
    m_AvailabilityListeners.append(&m_ArtworkUploader);
    m_AvailabilityListeners.append(&m_ZipArchiver);
}

void XpiksApp::setupUI(QQmlContext *context) {
    context->setContextProperty("ArtworksListModel", &m_ArtworksListModel);
    context->setContextProperty("artworkRepository", &m_FilteredArtworksRepository);
    context->setContextProperty("combinedArtworks", &m_CombinedArtworksModel);
    context->setContextProperty("secretsManager", &m_SecretsManager);
    context->setContextProperty("undoRedoManager", &m_UndoRedoManager);
    context->setContextProperty("keywordsSuggestor", &m_KeywordsSuggestor);
    context->setContextProperty("settingsModel", &m_SettingsModel);
    context->setContextProperty("filteredArtworksListModel", &m_FilteredArtworksListModel);
    context->setContextProperty("helpersWrapper", &m_HelpersQmlWrapper);
    context->setContextProperty("recentDirectories", &m_RecentDirectorieModel);
    context->setContextProperty("recentFiles", &m_RecentFileModel);
    context->setContextProperty("metadataIOCoordinator", &m_MetadataIOCoordinator);
    context->setContextProperty("pluginManager", &m_PluginManager);
    context->setContextProperty("pluginsWithActions", &m_PluginsWithActions);
    context->setContextProperty("warningsModel", &m_WarningsModel);
    context->setContextProperty("languagesModel", &m_LanguagesModel);
    context->setContextProperty("i18", &m_LanguagesModel);
    context->setContextProperty("uiColors", &m_ColorsModel);
    context->setContextProperty("acSource", &m_KeywordsAutoCompleteModel);
    context->setContextProperty("replaceModel", &m_ReplaceModel);
    context->setContextProperty("presetsModel", &m_PresetsModel);
    context->setContextProperty("filteredPresetsModel", &m_FilteredPresetsModel);
    context->setContextProperty("artworkProxy", &m_ArtworkProxyModel);
    context->setContextProperty("translationManager", &m_TranslationManager);
    context->setContextProperty("uiManager", &m_UIManager);
    context->setContextProperty("quickBuffer", &m_QuickBuffer);
    context->setContextProperty("userDictEditModel", &m_UserDictEditModel);
    context->setContextProperty("switcher", &m_SwitcherModel);
    context->setContextProperty("duplicatesModel", &m_DuplicatesModel);
    context->setContextProperty("csvExportModel", &m_CsvExportModel);
    context->setContextProperty("presetsGroups", m_PresetsModel.getGroupsModel());

    context->setContextProperty("tabsModel", m_UIManager.getTabsModel());
    context->setContextProperty("activeTabs", m_UIManager.getActiveTabs());
    context->setContextProperty("inactiveTabs", m_UIManager.getInactiveTabs());

    context->setContextProperty("xpiksApp", this);

#ifdef QT_DEBUG
    QVariant isDebug(true);
#else
    QVariant isDebug(false);
#endif
    context->setContextProperty("debug", isDebug);
    context->setContextProperty("debugTabs", isDebug);

    m_UIManager.addSystemTab(FILES_FOLDERS_TAB_ID, "qrc:/CollapserTabs/FilesFoldersIcon.qml", "qrc:/CollapserTabs/FilesFoldersTab.qml");
    m_UIManager.addSystemTab(QUICKBUFFER_TAB_ID, "qrc:/CollapserTabs/QuickBufferIcon.qml", "qrc:/CollapserTabs/QuickBufferTab.qml");
    m_UIManager.addSystemTab(TRANSLATOR_TAB_ID, "qrc:/CollapserTabs/TranslatorIcon.qml", "qrc:/CollapserTabs/TranslatorTab.qml");
    m_UIManager.initializeSystemTabs();
    m_UIManager.initialize();
}

void XpiksApp::start() {
    if (m_AfterInitCalled) {
        LOG_WARNING << "Attempt to call afterConstructionCallback() second time";
        return;
    }

    m_RequestsService.startService();

    m_SwitcherModel.initialize();
    m_SwitcherModel.updateConfigs(m_RequestsService);

    const int waitSeconds = 5;
    Helpers::AsyncCoordinatorStarter deferredStarter(&m_InitCoordinator, waitSeconds);
    Q_UNUSED(deferredStarter);

    m_AfterInitCalled = true;
    std::shared_ptr<Services::ServiceStartParams> emptyParams;
    auto coordinatorParams = std::make_shared<Helpers::AsyncCoordinatorStartParams>(&m_InitCoordinator);

    bool dbInitialized = m_DatabaseManager.initialize();
    Q_ASSERT(dbInitialized);
    if (!dbInitialized) {
        LOG_WARNING << "Failed to initialize the DB. Xpiks will crash soon";
    }

    m_MaintenanceService.startService();
    m_ImageCachingService.startService(coordinatorParams);
    m_MetadataIOService.startService(m_DatabaseManager, m_ArtworksUpdateHub);
    m_VideoCachingService.startService(m_ImageCachingService, m_ArtworksUpdateHub, m_MetadataIOService);

    m_SpellCheckerService.startService(coordinatorParams);
    m_WarningsService.startService(emptyParams);
    m_AutoCompleteService.startService(coordinatorParams);
    m_TranslationService.startService(coordinatorParams);

    QCoreApplication::processEvents();

    const QString reportingEndpoint =
        QLatin1String(
            "cc39a47f60e1ed812e2403b33678dd1c529f1cc43f66494998ec478a4d13496269a3dfa01f882941766dba246c76b12b2a0308e20afd84371c41cf513260f8eb8b71f8c472cafb1abf712c071938ec0791bbf769ab9625c3b64827f511fa3fbb");
    QString endpoint = Encryption::decodeText(reportingEndpoint, "reporting");
    m_TelemetryService.setEndpoint(endpoint);

    m_TelemetryService.startReporting();
    m_UploadInfoRepository.initializeStocksList(&m_InitCoordinator, m_RequestsService);
    m_WarningsSettingsModel.initializeConfigs(m_RequestsService);
    m_MaintenanceService.initializeDictionaries(&m_TranslationManager, &m_InitCoordinator);
    m_UploadInfoRepository.initializeConfig();
    m_PresetsModel.initializePresets();
    m_CsvExportModel.initializeExportPlans(&m_InitCoordinator, m_RequestsService);
    m_KeywordsSuggestor.initSuggestionEngines(m_MetadataIOService);
    m_UpdateService.initialize();
}

void XpiksApp::stop() {
    LOG_DEBUG << "Shutting down...";
    if (!m_AfterInitCalled) {
        return;
    }

#ifdef WITH_PLUGINS
    m_PluginManager.unloadPlugins();
#endif

    m_ArtworksRepository.stopListeningToUnavailableFiles();

    m_ArtworksListModel.disconnect();
    m_ArtworksListModel.deleteAllItems();
    m_FilteredArtworksListModel.disconnect();

    m_ImageCachingService.stopService();
    m_VideoCachingService.stopService();
    m_UpdateService.stopChecking();
    m_MetadataIOService.stopService();

    m_SpellCheckerService.stopService();
    m_WarningsService.stopService();
    m_AutoCompleteService.stopService();
    m_TranslationService.stopService();

    m_SettingsModel.syncronizeSettings();

    m_MaintenanceService.stopService();
    m_DatabaseManager.prepareToFinalize();

    // we have a second for important? stuff
    m_TelemetryService.reportAction(Connectivity::UserAction::Close);
    m_TelemetryService.stopReporting();
    m_RequestsService.stopService();
}

void XpiksApp::setupWindow(QQuickWindow *window) {
    if (window == nullptr) { return; }

    QScreen *screen = window->screen();

    QObject::connect(window, &QQuickWindow::screenChanged,
                     &m_ImageCachingService, &QMLExtensions::ImageCachingService::screenChangedHandler);
    QObject::connect(screen, &QScreen::logicalDotsPerInchChanged,
                     &m_ImageCachingService, &QMLExtensions::ImageCachingService::dpiChanged);
    QObject::connect(screen, &QScreen::physicalDotsPerInchChanged,
                     &m_ImageCachingService, &QMLExtensions::ImageCachingService::dpiChanged);

    QObject::connect(window, &QQuickWindow::screenChanged,
                     &m_UIManager, &Models::UIManager::onScreenChanged);
    QObject::connect(screen, &QScreen::logicalDotsPerInchChanged,
                     &m_UIManager, &Models::UIManager::onScreenDpiChanged);
    QObject::connect(screen, &QScreen::physicalDotsPerInchChanged,
                     &m_UIManager, &Models::UIManager::onScreenDpiChanged);

    auto &uiProvider = m_PluginManager.getUIProvider();
    uiProvider.setRoot(window->contentItem());
}

void XpiksApp::shutdown() {
    LOG_DEBUG << "emitting signal";
    emit globalBeforeDestruction();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    {
        stop();
    }
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    m_UpdateService.tryToUpgradeXpiks();
}

void XpiksApp::upgradeNow() {
    LOG_DEBUG << "#";
    m_UpdateService.setHaveUpgradeConsent();
    emit upgradeInitiated();
}

void XpiksApp::debugCrash() {
    raise(SIGTERM);
}

void XpiksApp::addFiles(const QList<QUrl> &urls) {
    LOG_DEBUG << urls.size() << "urls";
    Common::AddFilesFlags flags = Common::AddFilesFlags::None;
    Common::ApplyFlag(flags, m_SettingsModel.getAutoFindVectors(), Common::AddFilesFlags::FlagAutoFindVectors);
    auto files = std::make_shared<Filesystem::FilesCollection>(urls);
    doAddFiles(files, flags);
}

void XpiksApp::addDirectories(const QList<QUrl> &urls) {
    LOG_DEBUG << urls.size() << "urls";
    Common::AddFilesFlags flags = Common::AddFilesFlags::None;
    Common::ApplyFlag(flags, m_SettingsModel.getAutoFindVectors(), Common::AddFilesFlags::FlagAutoFindVectors);
    Common::SetFlag(flags, Common::AddFilesFlags::FlagIsFullDirectory);
    auto directories = std::make_shared<Filesystem::DirectoriesCollection>(urls);
    doAddFiles(directories, flags);
}

void XpiksApp::dropItems(const QList<QUrl> &urls) {
    LOG_DEBUG << urls.size() << "urls";
    Common::AddFilesFlags flags = Common::AddFilesFlags::None;
    Common::ApplyFlag(flags, m_SettingsModel.getAutoFindVectors(), Common::AddFilesFlags::FlagAutoFindVectors);
    auto files = std::make_shared<Filesystem::FilesDirectoriesCollection>(urls);
    doAddFiles(files, flags);
}

void XpiksApp::removeDirectory(int index) {
    int originalIndex = m_FilteredArtworksRepository.getOriginalIndex(index);
    auto removeResult = m_ArtworksListModel.removeFilesFromDirectory(originalIndex);
    Commands::SaveSessionCommand(m_MaintenanceService, m_ArtworksListModel, m_SessionManager).execute();
}

void XpiksApp::removeUnavailableFiles() {
    LOG_DEBUG << "#";
    m_CombinedArtworksModel.generateAboutToBeRemoved();
    m_ArtworksListModel.generateAboutToBeRemoved();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    for (auto &listener: m_AvailabilityListeners) {
        listener->removeUnavailableItems();
    }

    m_ArtworksListModel.deleteUnavailableItems();

    m_UndoRedoManager.discardLastAction();

    if (m_ArtworksRepository.canPurgeUnavailableFiles()) {
        m_ArtworksRepository.purgeUnavailableFiles();
    } else {
        LOG_INFO << "Unavailable files purging postponed";
    }
}

int XpiksApp::doAddFiles(const std::shared_ptr<Filesystem::IFilesCollection> &files, Common::AddFilesFlags flags) {
    using namespace Commands;
    using CompositeTemplate = CompositeCommandTemplate<Artworks::ArtworksSnapshot>;
    using ArtworksTemplate = std::shared_ptr<ICommandTemplate<Artworks::ArtworksSnapshot>>;

    std::vector<ArtworksTemplate> postAddActions = {
        std::make_shared<ReadMetadataTemplate>(m_MetadataIOService, m_MetadataIOCoordinator),
        std::make_shared<GenerateThumbnailsTemplate>(m_ImageCachingService, m_VideoCachingService),
        std::make_shared<AutoImportMetadataCommand>(m_MetadataIOCoordinator, m_SettingsModel, m_SwitcherModel),
        std::make_shared<AddToRecentTemplate>(m_RecentFileModel),
    };

    if (!Common::HasFlag(flags, Common::AddFilesFlags::FlagIsSessionRestore)) {
        postAddActions.emplace_back(
                    std::make_shared<SaveSessionCommand>(m_MaintenanceService, m_ArtworksListModel, m_SessionManager));
        postAddActions.emplace_back(
                    std::make_shared<CleanupLegacyBackupsCommand>(files, m_MaintenanceService));
    }

    ArtworksTemplate actions = std::make_shared<CompositeTemplate>(postAddActions);

    auto addFilesCommand = std::make_shared<AddFilesCommand>(
                               files, flags, m_ArtworksListModel, actions);

    m_CommandManager.processCommand(addFilesCommand);
    return addFilesCommand->getAddedCount();
}

void XpiksApp::afterServicesStarted() {
    LOG_DEBUG << "#";

#ifdef WITH_PLUGINS
    m_PluginManager.loadPlugins();
#endif

    if (m_SettingsModel.getSaveSession() || m_SessionManager.getIsEmergencyRestore()) {
        auto session = m_SessionManager.restoreSession();
        int addedCount = doAddFiles(std::get<0>(session), Common::AddFilesFlags::FlagIsSessionRestore);
        if (addedCount > 0) {
            m_ArtworksRepository.restoreFullDirectories(std::get<1>(session));
            auto snapshot = m_ArtworksListModel.snapshotAll();
            m_MaintenanceService.saveSession(snapshot, m_SessionManager);
        }
    }

#if !defined(INTEGRATION_TESTS)
    m_SwitcherModel.afterInitializedCallback();
#endif

    m_UpdateService.startChecking();

    executeMaintenanceJobs();
}

void XpiksApp::executeMaintenanceJobs() {
    // integration test just don't have old ones
    m_MaintenanceService.launchExiftool(m_SettingsModel.getExifToolPath());
    m_MaintenanceService.cleanupLogs();
    m_MaintenanceService.cleanupUpdatesArtifacts();
}

void XpiksApp::connectEntitiesSignalsSlots() {
    QObject::connect(&m_SecretsManager, &Encryption::SecretsManager::beforeMasterPasswordChange,
                     &m_UploadInfoRepository, &Models::UploadInfoRepository::onBeforeMasterPasswordChanged);
    QObject::connect(&m_SecretsManager, &Encryption::SecretsManager::afterMasterPasswordReset,
                     &m_UploadInfoRepository, &Models::UploadInfoRepository::onAfterMasterPasswordReset);

    QObject::connect(&m_ArtworksListModel, &Models::ArtworksListModel::selectedArtworksRemoved,
                     &m_FilteredArtworksListModel, &Models::FilteredArtworksListModel::onSelectedArtworksRemoved);
    QObject::connect(&m_ArtworksListModel, &Models::ArtworksListModel::artworkSelectedChanged,
                     &m_FilteredArtworksListModel, &Models::FilteredArtworksListModel::itemSelectedChanged);

    QObject::connect(&m_SettingsModel, &Models::SettingsModel::settingsUpdated,
                     &m_FilteredArtworksListModel, &Models::FilteredArtworksListModel::onSettingsUpdated);
    QObject::connect(&m_SettingsModel, &Models::SettingsModel::spellCheckDisabled,
                     &m_ArtworksListModel, &Models::ArtworksListModel::onSpellCheckDisabled);
    QObject::connect(&m_SettingsModel, &Models::SettingsModel::duplicatesCheckDisabled,
                     &m_ArtworksListModel, &Models::ArtworksListModel::onDuplicatesDisabled);
    QObject::connect(&m_SettingsModel, &Models::SettingsModel::exiftoolSettingChanged,
                     &m_MaintenanceService, &Maintenance::MaintenanceService::onExiftoolPathChanged);

    QObject::connect(&m_MaintenanceService, &Maintenance::MaintenanceService::exiftoolDetected,
                     &m_SettingsModel, &Models::SettingsModel::onRecommendedExiftoolFound);
    QObject::connect(&m_MaintenanceService, &Maintenance::MaintenanceService::exiftoolDetected,
                     &m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::onRecommendedExiftoolFound);

    QObject::connect(&m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::metadataWritingFinished,
                     &m_ArtworksListModel, &Models::ArtworksListModel::onMetadataWritingFinished);

    // current editable
    QObject::connect(&m_FilteredArtworksListModel, &Models::FilteredArtworksListModel::clearCurrentEditable,
                     &m_CurrentEditableModel, &Models::CurrentEditableModel::onClearCurrentEditable);
    QObject::connect(&m_CombinedArtworksModel, &Models::CombinedArtworksModel::clearCurrentEditable,
                     &m_CurrentEditableModel, &Models::CurrentEditableModel::onClearCurrentEditable);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckService::serviceAvailable,
                     &m_ArtworksListModel, &Models::ArtworksListModel::onSpellCheckerAvailable);

    QObject::connect(&m_ArtworksRepository, &Models::ArtworksRepository::selectionChanged,
                     &m_FilteredArtworksListModel, &Models::FilteredArtworksListModel::onDirectoriesSelectionChanged);

    QObject::connect(&m_UndoRedoManager, &UndoRedo::UndoRedoManager::undoStackEmpty,
                     &m_ArtworksListModel, &Models::ArtworksListModel::onUndoStackEmpty);

    QObject::connect(&m_UndoRedoManager, &UndoRedo::UndoRedoManager::undoStackEmpty,
                     &m_ArtworksRepository, &Models::ArtworksRepository::onUndoStackEmpty);

    QObject::connect(&m_SettingsModel, &Models::SettingsModel::userStatisticsChanged,
                     &m_TelemetryService, &Connectivity::TelemetryService::changeReporting);

    QObject::connect(&m_LanguagesModel, &Models::LanguagesModel::languageChanged,
                     &m_KeywordsSuggestor, &Suggestion::KeywordsSuggestor::onLanguageChanged);

    QObject::connect(&m_UpdateService, &Connectivity::UpdateService::updateAvailable,
                     &m_HelpersQmlWrapper, &Helpers::HelpersQmlWrapper::updateAvailable);
    QObject::connect(&m_UpdateService, &Connectivity::UpdateService::updateDownloaded,
                     this, &XpiksApp::isUpdateDownloadedChanged);
    QObject::connect(&m_UpdateService, &Connectivity::UpdateService::updateDownloaded,
                     &m_HelpersQmlWrapper, &Helpers::HelpersQmlWrapper::updateDownloaded);

    QObject::connect(&m_WarningsService, &Warnings::WarningsService::queueIsEmpty,
                     &m_WarningsModel, &Warnings::WarningsModel::onWarningsUpdateRequired);

    QObject::connect(&m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::recommendedExiftoolFound,
                     &m_SettingsModel, &Models::SettingsModel::onRecommendedExiftoolFound);

    QObject::connect(&m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::metadataReadingFinished,
                     &m_ArtworksListModel, &Models::ArtworksListModel::modifiedArtworksCountChanged);

    QObject::connect(&m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::metadataWritingFinished,
                     &m_ArtworksListModel, &Models::ArtworksListModel::modifiedArtworksCountChanged);

    QObject::connect(&m_ArtworksListModel, &Models::ArtworksListModel::fileWithIndexUnavailable,
                     &m_ArtworkProxyModel, &Models::ArtworkProxyModel::itemUnavailableHandler);

    QObject::connect(&m_ArtworkProxyModel, &Models::ArtworkProxyModel::warningsCouldHaveChanged,
                     &m_WarningsModel, &Warnings::WarningsModel::onWarningsCouldHaveChanged);

    QObject::connect(&m_ArtworkProxyModel, &Models::ArtworkProxyModel::duplicatesCouldHaveChanged,
                     &m_DuplicatesModel, &SpellCheck::DuplicatesReviewModel::onDuplicatesCouldHaveChanged);

    // user dictionary update
    QObject::connect(&m_UserDictionary, &SpellCheck::UserDictionary::userDictUpdate,
                     &m_QuickBuffer, &Models::QuickBuffer::userDictUpdateHandler);
    QObject::connect(&m_UserDictionary, &SpellCheck::UserDictionary::userDictCleared,
                     &m_QuickBuffer, &Models::QuickBuffer::userDictClearedHandler);
    QObject::connect(&m_UserDictionary, &SpellCheck::UserDictionary::userDictUpdate,
                     &m_ArtworksListModel, &Models::ArtworksListModel::userDictUpdateHandler);
    QObject::connect(&m_UserDictionary, &SpellCheck::UserDictionary::userDictCleared,
                     &m_ArtworksListModel, &Models::ArtworksListModel::userDictClearedHandler);
    QObject::connect(&m_UserDictionary, &SpellCheck::UserDictionary::userDictUpdate,
                     &m_CombinedArtworksModel, &Models::CombinedArtworksModel::userDictUpdateHandler);
    QObject::connect(&m_UserDictionary, &SpellCheck::UserDictionary::userDictCleared,
                     &m_CombinedArtworksModel, &Models::CombinedArtworksModel::userDictClearedHandler);
    QObject::connect(&m_UserDictionary, &SpellCheck::UserDictionary::userDictUpdate,
                     &m_ArtworkProxyModel, &Models::ArtworkProxyModel::userDictUpdateHandler);
    QObject::connect(&m_UserDictionary, &SpellCheck::UserDictionary::userDictCleared,
                     &m_ArtworkProxyModel, &Models::ArtworkProxyModel::userDictClearedHandler);

#ifdef WITH_PLUGINS
    QObject::connect(&m_CurrentEditableModel, &Models::CurrentEditableModel::currentEditableChanged,
                     &m_PluginManager, &Plugins::PluginManager::onCurrentEditableChanged);

    QObject::connect(&m_PresetsModel, &KeywordsPresets::PresetKeywordsModel::presetsUpdated,
                     &m_PluginManager, &Plugins::PluginManager::onPresetsUpdated);
#endif
}

void XpiksApp::registerUICommands() {
    m_UICommandDispatcher.registerCommands(
    {
                    std::make_shared<Commands::FixSpellingInSelectedCommand>(
                    m_FilteredArtworksListModel, m_SpellSuggestionModel),

                    std::make_shared<Commands::ShowDuplicatesInSelectedCommand>(
                    m_FilteredArtworksListModel, m_DuplicatesModel),

                    std::make_shared<Commands::SaveSelectedCommand>(
                    m_FilteredArtworksListModel, m_MetadataIOCoordinator, m_MetadataIOService),

                    std::make_shared<Commands::WipeMetadataInSelectedCommand>(
                    m_FilteredArtworksListModel, m_MetadataIOCoordinator),

                    std::make_shared<Commands::RemoveSelectedCommand>(
                    m_FilteredArtworksListModel, m_ArtworksListModel,
                    std::make_shared<Commands::SaveSessionCommand>(m_MaintenanceService, m_ArtworksListModel, m_SessionManager)),

                    std::make_shared<Commands::ReimportMetadataForSelected>(
                    m_FilteredArtworksListModel, m_MetadataIOCoordinator),

                    std::make_shared<Commands::FixSpellingInCombinedEditCommand>(
                    m_CombinedArtworksModel, m_SpellSuggestionModel),

                    std::make_shared<Commands::FixSpellingInArtworkProxyCommand>(
                    m_ArtworkProxyModel, m_SpellSuggestionModel),

                    std::make_shared<Commands::FixSpellingInArtworkCommand>(
                    m_FilteredArtworksListModel, m_ArtworksListModel, m_SpellSuggestionModel)
                });
}

void XpiksApp::servicesInitialized(int status) {
    LOG_DEBUG << "#";
    Q_ASSERT(m_ServicesInitialized == false);

    Helpers::AsyncCoordinator::CoordinationStatus coordStatus = (Helpers::AsyncCoordinator::CoordinationStatus)status;

    if (m_ServicesInitialized == false) {
        m_ServicesInitialized = true;

        if (coordStatus == Helpers::AsyncCoordinator::AllDone ||
                coordStatus == Helpers::AsyncCoordinator::Timeout) {
            afterServicesStarted();
        }
    }
}
