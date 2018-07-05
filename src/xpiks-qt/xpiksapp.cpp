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
#include <UndoRedo/removeartworksitem.h>
#include <UndoRedo/removedirectoryitem.h>
#include <Commands/Files/addfilescommand.h>
#include <Commands/Services/cleanuplegacybackupscommand.h>
#include <Commands/Editing/readmetadatatemplate.h>
#include <Commands/Base/templatedcommand.h>
#include <Commands/Files/addtorecenttemplate.h>
#include <Commands/Services/autoimportmetadatacommand.h>
#include <Commands/Base/compositecommandtemplate.h>
#include <Commands/Services/generatethumbnailstemplate.h>
#include <Commands/artworksupdatetemplate.h>
#include <Commands/Services/inspectartworkstemplate.h>
#include <Commands/Services/backupartworkstemplate.h>

XpiksApp::XpiksApp(Common::ISystemEnvironment &environment):
    m_LogsModel(&m_ColorsModel),
    m_SettingsModel(environment),
    m_RecentDirectorieModel(environment),
    m_ArtworksRepository(m_RecentDirectorieModel),
    m_FilteredArtworksRepository(m_ArtworksListModel),
    m_UploadInfoRepository(environment),
    m_RequestsService(m_SettingsModel.getProxySettings()),
    m_PresetsModel(environment),
    m_WarningsService(environment),
    m_DatabaseManager(environment),
    m_SecretsStorage(new libxpks::microstocks::APISecretsStorage()),
    m_ApiClients(m_SecretsStorage.get()),
    m_RecentFileModel(environment),
    m_ArtworkUploader(environment, m_UploadInfoRepository),
    m_SpellCheckerService(environment, &m_SettingsModel),
    m_MetadataIOService(&m_DatabaseManager),
    m_AutoCompleteService(&m_AutoCompleteModel, &m_PresetsModel, &m_SettingsModel),
    m_ImageCachingService(environment, &m_DatabaseManager),
    m_ReplaceModel(&m_ColorsModel),
    m_TranslationManager(environment),
    m_TranslationService(m_TranslationManager),
    m_UIManager(environment, &m_SettingsModel),
    m_SessionManager(environment),
    m_MaintenanceService(environment),
    m_VideoCachingService(environment, &m_DatabaseManager),
    m_SwitcherModel(environment),
    m_DuplicatesModel(&m_ColorsModel),
    m_CsvExportModel(environment),
    m_UpdateService(environment, &m_SettingsModel, &m_SwitcherModel, &m_MaintenanceService),
    m_KeywordsSuggestor(m_ApiClients, m_RequestsService, m_SwitcherModel, environment),
    m_TelemetryService(m_SwitcherModel, m_SettingsModel),
    m_PluginManager(environment, &m_DatabaseManager, m_RequestsService, m_ApiClients),
    m_HelpersQmlWrapper(environment, &m_ColorsModel),
    m_CommandManager(m_UndoRedoManager)
{
    m_FilteredPresetsModel.setSourceModel(&m_PresetsModel);

    m_WarningsModel.setSourceModel(&m_ArtworksListModel);
    m_WarningsModel.setWarningsSettingsModel(m_WarningsService.getWarningsSettingsModel());

    m_ArtworksUpdateHub.setStandardRoles(m_ArtworksListModel.getStandardUpdateRoles());

    m_PluginsWithActions.setSourceModel(&m_PluginManager);

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

Plugins::UIProvider *XpiksApp::getUIProvider() {
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
    injectActionsTemplates();

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
    context->setContextProperty("acSource", &m_AutoCompleteModel);
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
    m_SwitcherModel.updateConfigs();

    const int waitSeconds = 5;
    Helpers::AsyncCoordinatorStarter deferredStarter(&m_InitCoordinator, waitSeconds);
    Q_UNUSED(deferredStarter);

    m_AfterInitCalled = true;
    std::shared_ptr<Services::ServiceStartParams> emptyParams;
    auto coordinatorParams = std::make_shared(new Helpers::AsyncCoordinatorStartParams(&m_InitCoordinator));

    bool dbInitialized = m_DatabaseManager.initialize();
    Q_ASSERT(dbInitialized);
    if (!dbInitialized) {
        LOG_WARNING << "Failed to initialize the DB. Xpiks will crash soon";
    }

    m_MaintenanceService.startService();
    m_ImageCachingService.startService(coordinatorParams);
    m_VideoCachingService.startService();
    m_MetadataIOService.startService();

    m_SpellCheckerService.startService(coordinatorParams, m_WarningsService);
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
    m_UploadInfoRepository.initializeStocksList(&m_InitCoordinator);
    m_WarningsService.initWarningsSettings();
    m_MaintenanceService.initializeDictionaries(&m_TranslationManager, &m_InitCoordinator);
    m_UploadInfoRepository.initializeConfig();
    m_PresetsModel.initializePresets();
    m_CsvExportModel.initializeExportPlans(&m_InitCoordinator);
    m_KeywordsSuggestor.initSuggestionEngines();
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

    m_MainDelegator.clearCurrentItem();

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

    auto *uiProvider = m_PluginManager.getUIProvider();
    uiProvider->setRoot(window->contentItem());
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
    Common::AddFilesFlags flags = 0;
    Common::ApplyFlag(flags, m_SettingsModel.getAutoFindVectors(), Common::AddFilesFlags::FlagAutoFindVectors);

    std::shared_ptr<Filesystem::IFilesCollection> files(
                new Filesystem::FilesCollection(urls));

    doAddFiles(files, flags);
}

void XpiksApp::addDirectories(const QList<QUrl> &urls) {
    LOG_DEBUG << urls.size() << "urls";
    Common::AddFilesFlags flags = 0;
    Common::ApplyFlag(flags, m_SettingsModel.getAutoFindVectors(), Common::AddFilesFlags::FlagAutoFindVectors);
    Common::SetFlag(flags, Common::AddFilesFlags::FlagIsFullDirectory);

    std::shared_ptr<Filesystem::IFilesCollection> directories(
                new Filesystem::DirectoriesCollection(urls));

    doAddFiles(directories, flags);
}

void XpiksApp::dropItems(const QList<QUrl> &urls) {
    LOG_DEBUG << urls.size() << "urls";
    Common::AddFilesFlags flags = 0;
    Common::ApplyFlag(flags, m_SettingsModel.getAutoFindVectors(), Common::AddFilesFlags::FlagAutoFindVectors);

    std::shared_ptr<Filesystem::IFilesCollection> files(
                new Filesystem::FilesDirectoriesCollection(urls));

    doAddFiles(files, flags);
}

void XpiksApp::removeSelectedArtworks() {
    LOG_DEBUG << "#";
    auto removeResult = m_FilteredArtworksListModel.removeSelectedArtworks();
    Commands::SaveSessionCommand(m_MaintenanceService, m_ArtworksListModel, m_SessionManager).execute();
    m_UndoRedoManager.recordHistoryItem(
                std::make_unique<UndoRedo::IHistoryItem>(
                    new UndoRedo::RemoveArtworksHistoryItem()));
}

void XpiksApp::removeDirectory(int index) {
    int originalIndex = m_FilteredArtworksRepository.getOriginalIndex(index);
    auto removeResult = m_ArtworksListModel.removeArtworksDirectory(originalIndex);
    Commands::SaveSessionCommand(m_MaintenanceService, m_ArtworksListModel, m_SessionManager).execute();
    m_UndoRedoManager.recordHistoryItem(
                std::make_unique<UndoRedo::IHistoryItem>(
                    new UndoRedo::RemoveDirectoryHistoryItem()));
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

void XpiksApp::doAddFiles(const std::shared_ptr<Filesystem::IFilesCollection> &files, Common::AddFilesFlags flags) {
    using namespace Commands;

    auto afterAddActions = std::make_shared<CompositeCommandTemplate>(
    {
                    new ReadMetadataTemplate(m_MetadataIOService, m_MetadataIOCoordinator),
                    new GenerateThumbnailsTemplate(m_ImageCachingService, m_VideoCachingService),
                    new AutoImportMetadataCommand(m_MetadataIOCoordinator, m_SettingsModel, m_SwitcherModel),
                    new AddToRecentTemplate(m_RecentFileModel),
                    new SaveSessionCommand(m_MaintenanceService, m_ArtworksListModel, m_SessionManager),
                    new CleanupLegacyBackupsCommand(files, m_MaintenanceService)
                });

    auto addFilesCommand = std::make_shared<AddFilesCommand>(
                files, flags, m_ArtworksListModel,
                std::dynamic_pointer_cast<IArtworksCommandTemplate>(afterAddActions));

    m_CommandManager.processCommand(
                std::dynamic_pointer_cast<ICommand>(addFilesCommand));
}

void XpiksApp::afterServicesStarted() {
    LOG_DEBUG << "#";

#ifdef WITH_PLUGINS
    m_PluginManager.loadPlugins();
#endif

    if (m_SettingsModel.getSaveSession() || m_SessionManager.getIsEmergencyRestore()) {
        int newFilesAdded = m_SessionManager.restoreSession(m_ArtworksRepository);
        if (newFilesAdded > 0) {
            // immediately save restored session - to beat race between
            // saving session from Add Command and restoring FULL_DIR flag
            m_MainDelegator.saveSessionInBackground();
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
    m_MetadataIOCoordinator.autoDiscoverExiftool();

    m_MaintenanceService.moveSettings(&m_SettingsModel);
#ifdef QT_DEBUG
    m_MaintenanceService.upgradeImagesCache(&m_ImageCachingService);
#endif

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
                     m_ArtworksListModel, &Models::ArtworksListModel::onDuplicatesDisabled);
    QObject::connect(&m_SettingsModel, &Models::SettingsModel::spellCheckRestarted,
                     m_ArtworksListModel, &Models::ArtworksListModel::onSpellCheckRestarted);
    QObject::connect(&m_SettingsModel, &Models::SettingsModel::exiftoolSettingChanged,
                     &m_MaintenanceService, &Maintenance::MaintenanceService::onExiftoolPathChanged);

    QObject::connect(&m_MaintenanceService, &Maintenance::MaintenanceService::exiftoolDetected,
                     &m_SettingsModel, &Models::SettingsModel::onRecommendedExiftoolFound);
    QObject::connect(&m_MaintenanceService, &Maintenance::MaintenanceService::exiftoolDetected,
                     &m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::onRecommendedExiftoolFound);

    QObject::connect(&m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::writingWorkersFinished,
                     &m_ArtworksListModel, &Models::ArtworksListModel::onMetadataWritingFinished);

    QObject::connect(&m_FilteredArtworksListModel, &Models::FilteredArtworksListModel::clearCurrentEditable,
                     &m_CurrentEditable, &Models::CurrentEditableModel::onClearCurrentEditable);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::serviceAvailable,
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

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                     &m_ArtworksListModel, &Models::ArtworksListModel::userDictUpdateHandler);
    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                     &m_ArtworksListModel, &Models::ArtworksListModel::userDictClearedHandler);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                     &m_CombinedArtworksModel, &Models::CombinedArtworksModel::userDictUpdateHandler);
    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                     &m_CombinedArtworksModel, &Models::CombinedArtworksModel::userDictClearedHandler);

    QObject::connect(&m_ArtworksListModel, &Models::ArtworksListModel::fileWithIndexUnavailable,
                     &m_ArtworkProxyModel, &Models::ArtworkProxyModel::itemUnavailableHandler);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                     &m_ArtworkProxyModel, &Models::ArtworkProxyModel::userDictUpdateHandler);
    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                     &m_ArtworkProxyModel, &Models::ArtworkProxyModel::userDictClearedHandler);

    QObject::connect(&m_ArtworkProxyModel, &Models::ArtworkProxyModel::warningsCouldHaveChanged,
                     &m_WarningsModel, &Warnings::WarningsModel::onWarningsCouldHaveChanged);

    QObject::connect(&m_ArtworkProxyModel, &Models::ArtworkProxyModel::duplicatesCouldHaveChanged,
                     &m_DuplicatesModel, &SpellCheck::DuplicatesReviewModel::onDuplicatesCouldHaveChanged);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                     &m_QuickBuffer, &QuickBuffer::QuickBuffer::userDictUpdateHandler);
    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                     &m_QuickBuffer, &QuickBuffer::QuickBuffer::userDictClearedHandler);

#ifdef WITH_PLUGINS
    QObject::connect(&m_CurrentEditable, &Models::CurrentEditableModel::currentEditableChanged,
                     &m_PluginManager, &Plugins::PluginManager::onCurrentEditableChanged);

    QObject::connect(&m_UndoRedoManager, &UndoRedo::UndoRedoManager::actionUndone,
                     &m_PluginManager, &Plugins::PluginManager::onLastActionUndone);

    QObject::connect(&m_PresetsModel, &KeywordsPresets::PresetKeywordsModel::presetsUpdated,
                     &m_PluginManager, &Plugins::PluginManager::onPresetsUpdated);
#endif
}

void XpiksApp::injectActionsTemplates() {
    LOG_DEBUG << "#";
    using ArtworkTemplate = Commands::ICommandTemplate<Artworks::ArtworksSnapshot>;
    auto inspectionTemplate = std::make_shared<ArtworkTemplate>(
                new InspectArtworksTemplate(m_SpellCheckerService, m_WarningsService, m_SettingsModel));
    auto backupTemplate = std::make_shared<ArtworkTemplate>(
                new BackupArtworksTemplate(m_MetadataIOService));

    m_ArtworksListModel.setEditCommandTemplate(
                std::shared_ptr<ArtworkTemplate>(
                    new Commands::CompositeCommandTemplate({inspectionTemplate, backupTemplate})));
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
